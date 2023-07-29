/*
    Copyright (C) 2023  Paul Fotheringham (LinuxBrickie)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <lb/encoding/websocket.h>

#include <arpa/inet.h>
#include <stdexcept>


namespace lb
{


namespace encoding
{


namespace websocket
{


struct Decoder::Private
{
  Private( size_t cacheReserveSize )
  {
    partialData.reserve( cacheReserveSize );
  }

  Decoder::Result decode( const char* p, size_t numBytes );

  bool decodeHeader( const char*& buffer, size_t& numBufferBytes );
  bool decodePayload( const char*& buffer, size_t& numBufferBytes );

  enum class Status
  {
    eNothing,
    ePartialHeader,
    ePartialPayload
  }
  status{ Status::eNothing };

  /** \brief Stored data if a frame spans one or more calls to operator().

      This will either be enitrely header data or entirely payload data. If
      the latter then the header wiil be stored in \a header.

      We want to be able to mask the payload data in place so we cannot use a
      a std::string, instead use a std::vector as that will give us non-const
      access to the contiguous memory.
   */
  std::vector<char> partialData;

  // Only valid once we get to ePartialPayload
  Header header;
  std::string payload;
};


// static
std::string Header::toString( OpCode opCode )
{
  switch ( opCode )
  {
  case OpCode::eContinuation:
    return "Continuation";
  case OpCode::eText:
    return "Text";
  case OpCode::eBinary:
    return "Binary";
  case OpCode::eConnectionClose:
    return "ConnectinoClose";
  case OpCode::ePing:
    return "Ping";
  case OpCode::ePong:
    return "Pong";
  }
  return "Unknown";
}

uint8_t Header::encodedSizeInBytes() const
{
  return encodedSizeInBytes( payloadSize, isMasked );
}

// static
uint8_t Header::encodedSizeInBytes( size_t payloadSize, bool isMasked )
{
  uint8_t size{ 2 };
  if ( payloadSize >= ( 1 << 16 ) )
  {
    size += 8;
  }
  else if ( payloadSize > 125  )
  {
    size += 2;
  }
  if ( isMasked )
  {
    size += 4;
  }
  return size;
}

// static
std::string Header::toString( DecodeResult ds )
{
  switch( ds )
  {
  case DecodeResult::eSuccess:
    return "Success";
  case DecodeResult::eIncomplete:
    return "Incomplete";
  case DecodeResult::eInvalidOpCode:
    return "InvalidOpCode";
  case DecodeResult::ePayloadSizeInflatedEncoding:
    return "PayloadSizeInflatedEncoding";
  case DecodeResult::ePayloadSizeEighthByteMSBNotZero:
    return "PaylaodSizeEightthByteMSBNotZero";
  }
  return "Unknown";
}

Header::DecodeResult Header::decode( const char*buffer, size_t numBufferBytes )
{
  if ( numBufferBytes < minSizeInBytes )
  {
    // Defienitely not enough information
    return DecodeResult::eIncomplete;
  }

  const char* p{ buffer };

  // First byte
  fin  = *p & ( 1 << 7 );
  rsv1 = *p & ( 1 << 6 );
  rsv2 = *p & ( 1 << 5 );
  rsv3 = *p & ( 1 << 4 );
  // From RFC6455 Section 5.2:
  //
  // "If an unknown opcode is received, the receiving endpoint MUST _Fail
  //  the WebSocket Connection_."
  const uint8_t op = *p & 0x0F;
  switch ( op )
  {
  case 0:
    opCode = OpCode::eContinuation;
    break;
  case 1:
    opCode = OpCode::eText;
    break;
  case 2:
    opCode = OpCode::eBinary;
    break;
  case 8:
    opCode = OpCode::eConnectionClose;
    break;
  case 9:
    opCode = OpCode::ePing;
    break;
  case 10:
    opCode = OpCode::ePong;
    break;
  default:
    return DecodeResult::eInvalidOpCode;
  }

  // Second byte (and potentially the next two or eight bytes)
  ++p; --numBufferBytes;
  isMasked = *p & ( 1 << 7 );
  const uint8_t payloadSizeField{ uint8_t( *p & 0x7F ) };

  ++p; --numBufferBytes;
  if ( payloadSizeField == 126 )
  {
    if ( numBufferBytes < 2 )
    {
      return DecodeResult::eIncomplete;
    }
    payloadSize = ntohs( *((uint16_t*)p) );
    if ( payloadSize < 126 )
    {
      return DecodeResult::ePayloadSizeInflatedEncoding;
    }
    p += 2; numBufferBytes -= 2;
  }
  else if ( payloadSizeField == 127 )
  {
    if ( numBufferBytes < 8 )
    {
      return DecodeResult::eIncomplete;
    }
    uint16_t endianTest{ 1 };
    if ( (*(uint8_t*)(&endianTest)) == 1 ) // Little Endian
    {
      payloadSize = ( uint64_t( ntohl( *((uint32_t*)p) ) ) << 32 );
      p += 4;
      payloadSize |= uint64_t( ntohl( *((uint32_t*)p) ) );
      p += 4;
    }
    else // Big Endian
    {
      payloadSize = *((uint64_t*)p);
      p += 8;
    }
    numBufferBytes -= 8;
    if ( payloadSize < 65536 )
    {
      return DecodeResult::ePayloadSizeInflatedEncoding;
    }
    // From RFC6455 Section 5.2:
    //
    // "If 127, the following 8 bytes interpreted as a 64-bit unsigned integer
    //  (the most significant bit MUST be 0) are the payload length."
    if ( ( payloadSize >> 63 ) != 0 )
    {
      return DecodeResult::ePayloadSizeEighthByteMSBNotZero;
    }
  }
  else
  {
    payloadSize = payloadSizeField;
  }

  // Final four bytes (if required)
  if ( isMasked )
  {
    if ( numBufferBytes < 4 )
    {
      return DecodeResult::eIncomplete;
    }
    mask[0] = *p++;
    mask[1] = *p++;
    mask[2] = *p++;
    mask[3] = *p++;
  }

  return DecodeResult::eSuccess;
}

void Header::encode( char* p ) const
{
  // First byte
  *p = 0;
  if ( fin )
  {
    *p |= ( 1 << 7 );
  }
  if ( rsv1 )
  {
    *p |= ( 1 << 6 );
  }
  if ( rsv2 )
  {
    *p |= ( 1 << 5 );
  }
  if ( rsv3 )
  {
    *p |= ( 1 << 4 );
  }
  *p |= (uint8_t)opCode;

  // Second byte (and potentially the next two or eight bytes)
  ++p;
  *p = 0;
  if ( isMasked )
  {
    *p |= ( 1 << 7 );
  }
  if ( payloadSize < 126 )
  {
    *p |= payloadSize;
    ++p;
  }
  else if ( payloadSize < ( 1 << 16 ) )
  {
    *p |= 126;
    ++p;
    uint16_t* p16 = (uint16_t*)p;
    *p16 = htons( payloadSize );
    p += 2;
  }
  else
  {
    *p |= 127;
    ++p;
    uint16_t endianTest{ 1 };
    if ( (*(uint8_t*)(&endianTest)) == 1 ) // Little Endian
    {
      *p++ = ( payloadSize & 0xFF00000000000000 ) >> 56;
      *p++ = ( payloadSize & 0x00FF000000000000 ) >> 48;
      *p++ = ( payloadSize & 0x0000FF0000000000 ) >> 40;
      *p++ = ( payloadSize & 0x000000FF00000000 ) >> 32;
      *p++ = ( payloadSize & 0x00000000FF000000 ) >> 24;
      *p++ = ( payloadSize & 0x0000000000FF0000 ) >> 16;
      *p++ = ( payloadSize & 0x000000000000FF00 ) >> 8;
      *p++ =   payloadSize & 0x00000000000000FF;
    }
    else // Big Endian
    {
      uint64_t* p64 = (uint64_t*)p;
      *p64 = payloadSize;
      p += 8;
    }
  }

  // Final four bytes (if required)
  if ( isMasked )
  {
    *p++ = mask[0];
    *p++ = mask[1];
    *p++ = mask[2];
    *p++ = mask[3];
  }
}


Decoder::Decoder( size_t cacheReserveSize )
  : d{ std::make_unique<Private>( cacheReserveSize ) }
{
}

Decoder::~Decoder() = default;

Decoder::Result Decoder::decode( const char* p, size_t numBytes )
{
  return d->decode( p, numBytes );
}

Decoder::Result Decoder::Private::decode( const char* p, size_t numBytes )
{
  Result result;

  try
  {
    while ( numBytes > 0 )
    {
      switch( status )
      {
      case Status::eNothing:
        if ( !decodeHeader( p, numBytes ) )
        {
          status = Status::ePartialHeader;
          partialData.insert( partialData.end(), p, p + numBytes );
          result.numExtra = numBytes;
          numBytes = 0;
          continue;
        }
        if ( !decodePayload( p, numBytes ) )
        {
          status = Status::ePartialPayload;
          partialData.insert( partialData.end(), p, p + numBytes );
          result.numExtra = numBytes;
          numBytes = 0;
          continue;
        }
        break;

      case Status::ePartialHeader:
        partialData.insert( partialData.end(), p, p + numBytes );
        p = &partialData[0];
        result.numExtra = numBytes;
        numBytes = partialData.size();
        if ( !decodeHeader( p, numBytes ) )
        {
          numBytes = 0;
          continue;
        }
        result.numExtra = numBytes;
        if ( !decodePayload( p, numBytes ) )
        {
          // Remove the header bytes from partialData so that it contains just
          // the partial payload.
          std::vector<char> tmpPartial;
          tmpPartial.insert( tmpPartial.end(), p, p + numBytes );
          partialData.clear();
          partialData.swap( tmpPartial );

          status = Status::ePartialPayload;
          result.numExtra = numBytes;
          numBytes = 0;
          continue;
        }
        break;

      case Status::ePartialPayload:
        partialData.insert( partialData.end(), p, p + numBytes );
        p = &partialData[0];
        result.numExtra = numBytes;
        numBytes = partialData.size();
        if ( !decodePayload( p, numBytes ) )
        {
          numBytes = 0;
          continue;
        }
        break;
      }

      // Any code path that did not produce a full frame did a continue so if
      // we got here, i.e. from a switch break, then we have a full frame.
      partialData.clear();
      status = Status::eNothing;
      result.numExtra = 0;

      result.frames.emplace_back();
      std::swap( result.frames.back().header,  header );
      std::swap( result.frames.back().payload, payload );
    }
  }
  catch( std::runtime_error e )
  {
    result.parseError = true;
    result.numExtra = numBytes;
  }

  return result;
}

// Buffer and numBufferBytes only incremented on a true return
bool Decoder::Private::decodeHeader( const char*& buffer, size_t& numBufferBytes )
{
  switch ( header.decode( buffer, numBufferBytes ) )
  {
  case Header::DecodeResult::eIncomplete:
    break;
  case Header::DecodeResult::ePayloadSizeInflatedEncoding:
    throw std::runtime_error{ "Failed to deserialise frame header. Payload size using inflated encoding." };
  case Header::DecodeResult::ePayloadSizeEighthByteMSBNotZero:
    throw std::runtime_error{ "Failed to deserialise frame header. Eight byte payload size most signigicant bit is non-zero." };
  case Header::DecodeResult::eSuccess:
  {
    const auto numHeaderBytes{ header.encodedSizeInBytes() };
    buffer += numHeaderBytes;
    numBufferBytes -= numHeaderBytes;
    return true;
  }
  }

  return false;
}

// Buffer and numBufferBytes only incremented on a true return
bool Decoder::Private::decodePayload( const char*& buffer, size_t& numBufferBytes )
{
  if ( numBufferBytes < header.payloadSize )
  {
    return false;
  }

  // Take a (single) copy of the bytes.
  payload.assign( buffer, header.payloadSize );

  if ( header.isMasked )
  {
    encodeMaskedPayload( payload, header.mask );
  }

  buffer         += header.payloadSize;
  numBufferBytes -= header.payloadSize;

  return true;
}

void encodeMaskedPayload( const char* src
                        , size_t numSrcChars
                        , const uint8_t mask[4]
                        , char* dst )
{
  const size_t numTrailingBytes{ numSrcChars % 4 };
  const size_t numQuartets{ ( numSrcChars - numTrailingBytes ) / 4 };
  size_t i = 0;
  for ( size_t q = 0; q < numQuartets; ++q )
  {
    *dst = *src ^ mask[0]; ++src; ++dst;
    *dst = *src ^ mask[1]; ++src; ++dst;
    *dst = *src ^ mask[2]; ++src; ++dst;
    *dst = *src ^ mask[3]; ++src; ++dst;
  }

  for ( size_t t = 0; t < numTrailingBytes; ++t, ++src, ++dst )
  {
    *dst = *src ^ mask[t];
  }
}

void decodeMaskedPayload( const char* src
                        , size_t numSrcChars
                        , const uint8_t mask[4]
                        , char* dst )
{
  encodeMaskedPayload( src, numSrcChars, mask, dst );
}

void encodeMaskedPayload( std::string& src
                        , const uint8_t mask[4] )
{
  const size_t numTrailingBytes{ src.size() % 4 };
  const size_t numQuartets{ ( src.size() - numTrailingBytes ) / 4 };
  size_t i = 0;
  for ( size_t q = 0; q < numQuartets; ++q )
  {
    src[i] = src[i] ^ mask[0]; ++i;
    src[i] = src[i] ^ mask[1]; ++i;
    src[i] = src[i] ^ mask[2]; ++i;
    src[i] = src[i] ^ mask[3]; ++i;
  }

  for ( size_t t = 0; t < numTrailingBytes; ++t, ++i )
  {
    src[i] = src[i] ^ mask[t];
  }
}

void decodeMaskedPayload( std::string& src
                        , const uint8_t mask[4] )
{
  encodeMaskedPayload( src, mask );
}

std::string encodeMaskedPayload( const std::string& src
                               , const uint8_t mask[4] )
{
  if ( src.empty() )
  {
    return {};
  }

  std::unique_ptr<char[]> dst{ std::make_unique<char[]>( src.size() ) };

  encodeMaskedPayload( src.c_str(), src.size(), mask, dst.get() );

  // std::string always takes a copy. Understandable, but unfortunate here. If
  // only there was some sort of move semantics for passing C-style string
  // ownership to std::string. Of course we provide the in-place overload so
  // only use this if you really want a copy.
  return { dst.get(), src.size() };
}

std::string decodeMaskedPayload( const std::string& src
                               , const uint8_t mask[4] )
{
  return encodeMaskedPayload( src, mask );
}

template < class T >
void encodePayloadCloseStatusCodeT( CloseStatusCode statusCode, T& dst )
{
  switch ( statusCode )
  {
  case CloseStatusCode::eNormal: // 1000
    dst[0] = 0x03;
    dst[1] = 0xE8;
    break;
  case CloseStatusCode::eGoingAway: // 1001
    dst[0] = 0x03;
    dst[1] = 0xE9;
    break;
  case CloseStatusCode::eProtocolError: // 1002
    dst[0] = 0x03;
    dst[1] = 0xEA;
    break;
  case CloseStatusCode::eUnacceptableData: // 1003
    dst[0] = 0x03;
    dst[1] = 0xEB;
    break;
  case CloseStatusCode::eMismatchedData: // 1007
    dst[0] = 0x03;
    dst[1] = 0xEF;
    break;
  case CloseStatusCode::ePolicyViolation: // 1008
    dst[0] = 0x03;
    dst[1] = 0xF0;
    break;
  case CloseStatusCode::eTooMuchData: // 1009
    dst[0] = 0x03;
    dst[1] = 0xF1;
    break;
  case CloseStatusCode::eLackingExtension: // 1010
    dst[0] = 0x03;
    dst[1] = 0xF2;
    break;
  case CloseStatusCode::eUnexpectedCondition: // 1011
    dst[0] = 0x03;
    dst[1] = 0xF3;
    break;
  }
}

void encodePayloadCloseStatusCode( CloseStatusCode statusCode, char* dst )
{
  encodePayloadCloseStatusCodeT( statusCode, dst );
}

void encodePayloadCloseStatusCode( CloseStatusCode statusCode, std::string& dst )
{
  encodePayloadCloseStatusCodeT( statusCode, dst );
}

template < class T >
std::optional<CloseStatusCode> decodePayloadCloseStatusCodeT( const T& src )
{
  if ( src[0] == (char)0x03 )
  {
    if ( src[1] == (char)0xE8 )
    {
      return CloseStatusCode::eNormal; // 1000
    }
    else if ( src[1] == (char)0xE9 )
    {
      return CloseStatusCode::eGoingAway; // 1001
    }
    else if ( src[1] == (char)0xEA )
    {
      return CloseStatusCode::eProtocolError; // 1002
    }
    else if ( src[1] == (char)0xEB )
    {
      return CloseStatusCode::eUnacceptableData; // 1003
    }
    else if ( src[1] == (char)0xEF )
    {
      return CloseStatusCode::eMismatchedData; // 1007
    }
    else if ( src[1] == (char)0xF0 )
    {
      return CloseStatusCode::ePolicyViolation; // 1008
    }
    else if ( src[1] == (char)0xF1 )
    {
      return CloseStatusCode::eTooMuchData; // 1009
    }
    else if ( src[1] == (char)0xF2 )
    {
      return CloseStatusCode::eLackingExtension; // 1010
    }
    else if ( src[1] == (char)0xF3 )
    {
      return CloseStatusCode::eUnexpectedCondition; // 1011
    }
  }

  return {};
}

std::optional<CloseStatusCode> decodePayloadCloseStatusCode( const char* src )
{
  return decodePayloadCloseStatusCodeT( src );
}

std::optional<CloseStatusCode> decodePayloadCloseStatusCode( const std::string& src )
{
  return decodePayloadCloseStatusCodeT( src );
}


} // End of namespace websocket


} // End of namespace encoding


} // End of namespace lb
