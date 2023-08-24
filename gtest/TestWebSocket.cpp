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

#include <gtest/gtest.h>

#include <lb/encoding/websocket.h>


namespace ws = lb::encoding::websocket;


// Printers for correct gtest output.
namespace lb { namespace encoding { namespace websocket {
  void PrintTo( const Header::OpCode& ds, std::ostream* os )
  {
    *os << "OpCode " << Header::toString( ds );
  }
  void PrintTo( const Header::DecodeResult& ds, std::ostream* os )
  {
    *os << "DecodeStatus " << Header::toString( ds );
  }
  void PrintTo( const closestatus::CodeRange& cr, std::ostream* os )
  {
    *os << "Close Status Code Range " << closestatus::toString( cr );
  }
  void PrintTo( const closestatus::ProtocolCode& pc, std::ostream* os )
  {
    *os << "Close Status Protocol Code " << closestatus::toString( pc );
  }
  void PrintTo( const closestatus::IANACode& ic, std::ostream* os )
  {
    *os << "Close Status IANA Code " << closestatus::toString( ic );
  }
} } }


void testHeader( const ws::Header& header
               , bool fin
               , ws::Header::OpCode opCode
               , uint64_t payloadSize
               , bool isMasked = false
               , uint8_t mask0 = 0
               , uint8_t mask1 = 0
               , uint8_t mask2 = 0
               , uint8_t mask3 = 0 )
{
  EXPECT_EQ( header.fin, fin );
  EXPECT_EQ( header.rsv1, false ); // reserved i.e. unused, don't care
  EXPECT_EQ( header.rsv2, false ); // reserved i.e. unused, don't care
  EXPECT_EQ( header.rsv3, false ); // reserved i.e. unused, don't care
  EXPECT_EQ( header.opCode, opCode );
  EXPECT_EQ( header.payloadSize, payloadSize );
  EXPECT_EQ( header.isMasked, isMasked );
  EXPECT_EQ( header.mask[0], mask0 );
  EXPECT_EQ( header.mask[1], mask1 );
  EXPECT_EQ( header.mask[2], mask2 );
  EXPECT_EQ( header.mask[3], mask3 );
}

TEST(Decoding, WebSocketHeader)
{
  std::string headerBytes;

  {
    ws::Header header;
    headerBytes = "";
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eIncomplete );
  }

  {
    ws::Header header;
    headerBytes = "a";
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eIncomplete );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x00", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eContinuation, 0 );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x01", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eContinuation, 1 );
  }

  // FIN bit
  {
    ws::Header header;
    headerBytes.assign( "\x80\x7D", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, true, ws::Header::OpCode::eContinuation, 125 );
  }

  // Test OpCodes
  {
    ws::Header header;
    headerBytes.assign( "\x01\x3D", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eText, 61 );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x02\x1D", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eBinary, 29 );
  }

  {
    ws::Header header;  headerBytes.assign( "\x03\x6F", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eInvalidOpCode );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x04\x6F", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eInvalidOpCode );
  }

  {
    ws::Header header;  headerBytes.assign( "\x05\x6F", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eInvalidOpCode );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x06\x6F", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eInvalidOpCode );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x07\x6F", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eInvalidOpCode );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x08\x0D", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eConnectionClose, 13 );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x09\x07", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::ePing, 7 );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x0A\x03", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::ePong, 3 );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x0B\x0A", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eInvalidOpCode );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x0C\x0A", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eInvalidOpCode );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x0D\x0A", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eInvalidOpCode );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x0E\x0A", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eInvalidOpCode );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x0F\x0A", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eInvalidOpCode );
  }

  // Test 2-byte extended payload size
  {
    ws::Header header;
    headerBytes.assign( "\x00\x7E", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eIncomplete );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7E\x01", 3 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eIncomplete );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7E\x00\x01", 4 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::ePayloadSizeInflatedEncoding );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7E\x00\x7D", 4 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::ePayloadSizeInflatedEncoding );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7E\x00\x7E", 4 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eContinuation, 126 );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7E\xFF\xFF", 4 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eContinuation, 65535 );
  }

  // Test 8-byte extended payload size
  {
    ws::Header header;
    headerBytes.assign( "\x00\x7F", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eIncomplete );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7F\x01", 3 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eIncomplete );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7F\x01\x02", 4 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eIncomplete );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7F\x01\x02\x03", 5 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eIncomplete );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7F\x01\x02\x03\x04", 6 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eIncomplete );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7F\x01\x02\x03\x04\x05", 7 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eIncomplete );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7F\x01\x02\x03\x04\x05\x06", 8 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eIncomplete );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7F\x01\x02\x03\x04\x05\x06\x07", 9 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eIncomplete );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7F\x00\x00\x00\x00\x00\x00\x00\x01", 10 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::ePayloadSizeInflatedEncoding );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7F\x00\x00\x00\x00\x00\x00\xFF\xFF", 10 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::ePayloadSizeInflatedEncoding );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7F\x80\x00\x00\x00\x00\x00\x00\x00", 10 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::ePayloadSizeEighthByteMSBNotZero );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7F\x00\x00\x00\x00\x00\x01\x00\x00", 10 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eContinuation, 65536 );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7F\x40\x00\x00\x00\x00\x00\x00\x00", 10 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eContinuation, 4611686018427387904 );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7F\x7F\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 10 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eContinuation, 9223372036854775807 );
  }

  // Test mask
  {
    ws::Header header;
    headerBytes.assign( "\x00\x81", 2 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eIncomplete );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x81\x01", 3 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eIncomplete );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x81\x01\x02", 4 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eIncomplete );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x81\x01\x02\x03", 5 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eIncomplete );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x81\x0A\x0B\x0C\x0D", 6 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eContinuation, 1, true, 0x0A, 0x0B, 0x0C, 0x0D );
  }

  // Mask + 2-byte extended payload size
  {
    ws::Header header;
    headerBytes.assign( "\x00\xFE\x01\x02\x0A\x0B\x0C\x0D", 8 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eContinuation, 258, true, 0x0A, 0x0B, 0x0C, 0x0D );
  }

  // Mask + 8-byte extended payload size
  {
    ws::Header header;
    headerBytes.assign( "\x00\xFF\x01\x23\x45\x67\x89\xAB\xCD\xEF\x0A\x0B\x0C\x0D", 14 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eContinuation, 81985529216486895, true, 0x0A, 0x0B, 0x0C, 0x0D );
  }

  // Trailing bytes should not affect success, repeat some of the previous tests
  // with some extra random data appended.
  {
    ws::Header header;
    headerBytes.assign( "\x00\x01\xAB\xDE", 4 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eContinuation, 1 );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x80\x7D\xF1\x23", 4 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, true, ws::Header::OpCode::eContinuation, 125 );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x01\x3D\x11\x22\x33", 5 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eText, 61 );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7E\x00\x7E\x55\x66\x77", 7 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eContinuation, 126 );
  }

  {
    ws::Header header;
    headerBytes.assign( "\x00\x7F\x00\x00\x00\x00\x00\x01\x00\x00\xAA\xBB\xCC", 13 );
    const auto decodeResult = header.decode( headerBytes.c_str(), headerBytes.size() );
    EXPECT_EQ( decodeResult, ws::Header::DecodeResult::eSuccess );
    testHeader( header, false, ws::Header::OpCode::eContinuation, 65536 );
  }
}

// Only testing the payloads as headers tested elsewhere
void testPayloads( ws::Decoder::Result actualResult
                             , const std::vector<std::string>& expectedPayloads
                             , size_t expectedNumExtra )
{
  ASSERT_FALSE( actualResult.parseError );
  ASSERT_EQ( actualResult.frames.size(), expectedPayloads.size() );
  for ( size_t i = 0; i < actualResult.frames.size(); ++i )
  {
    EXPECT_EQ( actualResult.frames[i].payload, expectedPayloads[i] );
  }
  EXPECT_EQ( actualResult.numExtra, expectedNumExtra );
}

TEST(Decoding, WebSocketFrame)
{
  // Note that these tests focus on the overall frame decoding. Header decoding
  // is tested separately in the Decoding_WebSocketHeader test so there is no
  // need to go over that again. One consequence of this is that we can limit
  // ourselves to payload sizes less than 126 bytes.

  char frameBytes[ 2 + 4 + 125 ]; // first two header bytes + header mask + payload

  // Decode
  // - no bytes
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    const auto decodeResult = decoder.decode( frameBytes, 0 );
    testPayloads( decodeResult, {}, 0 );
  }

  // Decode
  // - single byte, insufficient even for a header
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    memcpy( frameBytes, "\x00", 1 );
    const auto decodeResult = decoder.decode( frameBytes, 1 );
    testPayloads( decodeResult, {}, 1 );
  }

  // Decode
  // - two bytes, a header indicating zero-sized payload i.e. the smallest
  //   possible complete frame
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    memcpy( frameBytes, "\x00\x00", 2 );
    const auto decodeResult = decoder.decode( frameBytes, 2 );
    testPayloads( decodeResult, { {} }, 0 );
    testHeader( decodeResult.frames[0].header, false, ws::Header::OpCode::eContinuation, 0 );
  }

  // Decode
  // - three bytes, a header indicating a single byte payload and the payload
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    memcpy( frameBytes, "\x00\x01X", 3 );
    const auto decodeResult = decoder.decode( frameBytes, 3 );
    testPayloads( decodeResult, { { "X" } }, 0 );
    testHeader( decodeResult.frames[0].header, false, ws::Header::OpCode::eContinuation, 1 );
  }

  // Decode
  // - one byte, the first byte of a header indicating a three byte payload
  // - four bytes, the remaining header bvte and the three payload bytes
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    memcpy( frameBytes, "\x00\x03XYZ", 5 );
    const auto decodeResult1 = decoder.decode( frameBytes, 1 );
    testPayloads( decodeResult1, {}, 1 );
    const auto decodeResult2 = decoder.decode( frameBytes + 1, 4 );
    testPayloads( decodeResult2, { { "XYZ" } }, 0 );
    testHeader( decodeResult2.frames[0].header, false, ws::Header::OpCode::eContinuation, 3 );
  }

  // Decode
  // - three bytes, a header indicating a three byte payload and the first payload byte
  // - two bytes, the remaining two payload bytes
  //
  // This is the same as the example above but the byte buffers are arranged differently.
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    memcpy( frameBytes, "\x00\x03XYZ", 5 );
    const auto decodeResult1 = decoder.decode( frameBytes, 3 );
    testPayloads( decodeResult1, {}, 1 );
    const auto decodeResult2 = decoder.decode( frameBytes + 3, 2 );
    testPayloads( decodeResult2, { { "XYZ" } }, 0 );
    testHeader( decodeResult2.frames[0].header, false, ws::Header::OpCode::eContinuation, 3 );
  }

  // Decode
  // - one byte, the first byte of a two byte header indicating a 9 byte payload
  // - three bytes, the second byte of the header indicating a three byte payload and the first payload byte
  // - two bytes, the remaining two payload bytes
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    memcpy( frameBytes, "\x00\x09""abcDEF[]!", 11 );
    const auto decodeResult1 = decoder.decode( frameBytes, 1 );
    testPayloads( decodeResult1, {}, 1 );
    const auto decodeResult2 = decoder.decode( frameBytes + 1, 3 );
    testPayloads( decodeResult2, {}, 2 );
    const auto decodeResult3 = decoder.decode( frameBytes + 4, 7 );
    EXPECT_FALSE( decodeResult3.parseError );
    testPayloads( decodeResult3, { { "abcDEF[]!" } }, 0 );
    testHeader( decodeResult3.frames[0].header, false, ws::Header::OpCode::eContinuation, 9 );
  }

  // Decode
  // - 17 bytes, a six byte header with a no-op masked 11 byte payload
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    memcpy( frameBytes, "\x00\x8B\x00\x00\x00\x00[123456789]", 17 );
    const auto decodeResult = decoder.decode( frameBytes, 17 );
    testPayloads( decodeResult, { { "[123456789]" } }, 0 );
  }

  // Decode
  // - 11 bytes, a six byte header with a masked 5 byte payload
  //
  // This is the masked frame example from RFC 6455. The unmasked payload is the
  // string "Hello".
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    memcpy( frameBytes, "\x81\x85\x37\xFA\x21\x3D\x7F\x9F\x4D\x51\x58", 11 );
    const auto decodeResult = decoder.decode( frameBytes, 11 );
    testPayloads( decodeResult, { { "Hello" } }, 0 );
  }

  // Repeat three of the previous tests but re-using the Decoder object to test
  // that it is properly retains state across frames. This is a simple test where
  // each frame's bytes end at the end of a byte buffer.
  {
    ws::Decoder decoder;
    // Decode (1 of 3)
    // - 17 bytes, a six byte header with a no-op masked 11 byte payload
    {
      memset( frameBytes, 0x00, sizeof(frameBytes) );
      memcpy( frameBytes, "\x00\x8B\x00\x00\x00\x00[123456789]", 17 );
      const auto decodeResult = decoder.decode( frameBytes, 17 );
      testPayloads( decodeResult, { { "[123456789]" } }, 0 );
    }

    // Decode (2 of 3)
    // - one byte, the first byte of a two byte header indicating a 9 byte payload
    // - three bytes, the second byte of the header indicating a three byte payload and the first payload byte
    // - two bytes, the remaining two payload bytes
    {
      memset( frameBytes, 0x00, sizeof(frameBytes) );
      memcpy( frameBytes, "\x00\x09""abcDEF[]!", 11 );
      const auto decodeResult1 = decoder.decode( frameBytes, 1 );
      testPayloads( decodeResult1, {}, 1 );
      const auto decodeResult2 = decoder.decode( frameBytes + 1, 3 );
      testPayloads( decodeResult2, {}, 2 );
      const auto decodeResult3 = decoder.decode( frameBytes + 4, 7 );
      testPayloads( decodeResult3, { { "abcDEF[]!" } }, 0 );
      testHeader( decodeResult3.frames[0].header, false, ws::Header::OpCode::eContinuation, 9 );
    }

    // Decode (3 of 3)
    // - 11 bytes, a six byte header with a masked 5 byte payload
    //
    // This is the masked frame example from RFC 6455. The unmasked payload is the
    // string "Hello".
    {
      memset( frameBytes, 0x00, sizeof(frameBytes) );
      memcpy( frameBytes, "\x81\x85\x37\xFA\x21\x3D\x7F\x9F\x4D\x51\x58", 11 );
      const auto decodeResult = decoder.decode( frameBytes, 11 );
      testPayloads( decodeResult, { { "Hello" } }, 0 );
    }
  }

  // Now use the same three frames but split the byte buffers up differently so
  // that frame boundaries lie within byte buffers and not at byte buffer
  // boundaries.
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    memcpy( frameBytes
          , "\x00\x8B\x00\x00\x00\x00[123456789]"          // 6 byte header, 11 byte payload
            "\x00\x09""abcDEF[]!"                          // 2 byte header,  9 byte payload
            "\x81\x85\x37\xFA\x21\x3D\x7F\x9F\x4D\x51\x58" // 6 byte header,  5 byte payload
          , 17 + 11 + 11 );

    // Decode (1 of 8)
    // - 17 bytes, a six byte header with a no-op masked 11 byte payload
    const auto decodeResult1 = decoder.decode( frameBytes, 5 );
    testPayloads( decodeResult1, {}, 5 );

    const auto decodeResult2 = decoder.decode( frameBytes + 5, 5 );
    testPayloads( decodeResult2, {}, 4 );

    const auto decodeResult3 = decoder.decode( frameBytes + 10, 5 );
    testPayloads( decodeResult3, {}, 5 );

    const auto decodeResult4 = decoder.decode( frameBytes + 15, 5 );
    testPayloads( decodeResult4, { { "[123456789]" } }, 1 ); // + header of next frame

    const auto decodeResult5 = decoder.decode( frameBytes + 20, 5 );
    testPayloads( decodeResult5, {}, 5 );

    const auto decodeResult6 = decoder.decode( frameBytes + 25, 5 );
    testPayloads( decodeResult6, { "abcDEF[]!" }, 2 );

    const auto decodeResult7 = decoder.decode( frameBytes + 30, 5 );
    testPayloads( decodeResult7, {}, 1 );

    const auto decodeResult8 = decoder.decode( frameBytes + 35, 4 );
    testPayloads( decodeResult8, { "Hello" }, 0 );
  }

  // Decode
  // - bad header where payload size has an inflated encoding
  //
  // Note the payload is irrelevant here as decoding should stop at header
  // parsing. We are really just testing that the parseError flag gets set.
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    memcpy( frameBytes, "\x00\x7E\x00\x01", 3 );
    const auto decodeResult = decoder.decode( frameBytes, 4 );
    EXPECT_TRUE( decodeResult.parseError );
    EXPECT_TRUE( decodeResult.frames.empty() );
    EXPECT_EQ( decodeResult.numExtra, 4 );
  }
}

template <class T>
void decodingWebSocketPayloadT( T& payload )
{
  ws::closestatus::PayloadCode closeStatusCode;

  payload[0] = 0x01; // invalid
  payload[1] = 0xE8;
  closeStatusCode = ws::closestatus::decodePayloadCode( payload );
  EXPECT_EQ( closeStatusCode, 488U );
  EXPECT_EQ( ws::closestatus::toCodeRange( closeStatusCode )
           , ws::closestatus::CodeRange::eUnused );

  payload[0] = 0x03;
  payload[1] = 0x6D; // invalid
  closeStatusCode = ws::closestatus::decodePayloadCode( payload );
  EXPECT_EQ( closeStatusCode, 877U );
  EXPECT_EQ( ws::closestatus::toCodeRange( closeStatusCode )
           , ws::closestatus::CodeRange::eUnused );

  // All (current) valid Protocol codes have this as the first byte
  payload[0] = 0x03;

  payload[1] = 0xE8;
  closeStatusCode = ws::closestatus::decodePayloadCode( payload );
  EXPECT_EQ( ws::closestatus::toCodeRange( closeStatusCode )
           , ws::closestatus::CodeRange::eProtocol );
  EXPECT_EQ( closeStatusCode
           , ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eNormal ) );

  payload[1] = 0xE9;
  closeStatusCode = ws::closestatus::decodePayloadCode( payload );
  EXPECT_EQ( ws::closestatus::toCodeRange( closeStatusCode )
           , ws::closestatus::CodeRange::eProtocol );
  EXPECT_EQ( closeStatusCode
           , ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eGoingAway ) );

  payload[1] = 0xEA;
  closeStatusCode = ws::closestatus::decodePayloadCode( payload );
  EXPECT_EQ( ws::closestatus::toCodeRange( closeStatusCode )
           , ws::closestatus::CodeRange::eProtocol );
  EXPECT_EQ( closeStatusCode
           , ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eProtocolError ) );

  payload[1] = 0xEB;
  closeStatusCode = ws::closestatus::decodePayloadCode( payload );
  EXPECT_EQ( ws::closestatus::toCodeRange( closeStatusCode )
           , ws::closestatus::CodeRange::eProtocol );
  EXPECT_EQ( closeStatusCode
           , ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eUnacceptableData ) );

  payload[1] = 0xEF;
  closeStatusCode = ws::closestatus::decodePayloadCode( payload );
  EXPECT_EQ( ws::closestatus::toCodeRange( closeStatusCode )
           , ws::closestatus::CodeRange::eProtocol );
  EXPECT_EQ( closeStatusCode
           , ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eMismatchedData ) );

  payload[1] = 0xF0;
  closeStatusCode = ws::closestatus::decodePayloadCode( payload );
  EXPECT_EQ( ws::closestatus::toCodeRange( closeStatusCode )
           , ws::closestatus::CodeRange::eProtocol );
  EXPECT_EQ( closeStatusCode
           , ws::closestatus::toPayload( ws::closestatus::ProtocolCode::ePolicyViolation ) );

  payload[1] = 0xF1;
  closeStatusCode = ws::closestatus::decodePayloadCode( payload );
  EXPECT_EQ( ws::closestatus::toCodeRange( closeStatusCode )
           , ws::closestatus::CodeRange::eProtocol );
  EXPECT_EQ( closeStatusCode
           , ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eTooMuchData ) );

  payload[1] = 0xF2;
  closeStatusCode = ws::closestatus::decodePayloadCode( payload );
  EXPECT_EQ( ws::closestatus::toCodeRange( closeStatusCode )
           , ws::closestatus::CodeRange::eProtocol );
  EXPECT_EQ( closeStatusCode
           , ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eLackingExtension ) );

  payload[1] = 0xF3;
  closeStatusCode = ws::closestatus::decodePayloadCode( payload );
  EXPECT_EQ( ws::closestatus::toCodeRange( closeStatusCode )
           , ws::closestatus::CodeRange::eProtocol );
  EXPECT_EQ( closeStatusCode
           , ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eUnexpectedCondition ) );

  // All (current) valid IANA codes have this as the first byte
  payload[0] = 0x0B;

  payload[1] = 0xB8;
  closeStatusCode = ws::closestatus::decodePayloadCode( payload );
  EXPECT_EQ( ws::closestatus::toCodeRange( closeStatusCode )
           , ws::closestatus::CodeRange::eIANA );
  EXPECT_EQ( closeStatusCode
           , ws::closestatus::toPayload( ws::closestatus::IANACode::eUnauthorised ) );

  payload[1] = 0xBB;
  closeStatusCode = ws::closestatus::decodePayloadCode( payload );
  EXPECT_EQ( ws::closestatus::toCodeRange( closeStatusCode )
           , ws::closestatus::CodeRange::eIANA );
  EXPECT_EQ( closeStatusCode
           , ws::closestatus::toPayload( ws::closestatus::IANACode::eForbidden ) );
}

TEST(Decoding, WebSocketPayload)
{
  // C-string API tests
  char payloadBytes[2];
  memset( payloadBytes, 0x00, 2 );
  decodingWebSocketPayloadT( payloadBytes );


  // std::string API tests (just repeat C-string tests)
  std::string payloadString( "\x00\x00", 2 );
  decodingWebSocketPayloadT( payloadString );
}

void testDecodedBytes( const char* context, char* actual, size_t numExpected, const char* expected )
{
  for ( size_t i = 0; i < numExpected; ++i, ++actual, ++expected )
  {
    EXPECT_EQ( *actual, *expected ) << " byte " << i << " mismatch in test " << context;
  }
  for ( size_t i = numExpected; i < ws::Header::maxSizeInBytes; ++i, ++actual )
  {
    EXPECT_EQ( *actual, '\0' ) << " byte " << i << " overwrote in test " << context;;
  }
}

TEST(Encoding, WebSocketHeader)
{
  char headerBytes[ ws::Header::maxSizeInBytes ];

  {
    ws::Header header;
    memset( headerBytes, 0x00, sizeof(headerBytes) );
    header.encode( headerBytes );
    testDecodedBytes( "default constructed", headerBytes, 2, "\x00\x00" );
  }

  {
    ws::Header header;
    header.payloadSize = 1;
    memset( headerBytes, 0x00, sizeof(headerBytes) );
    header.encode( headerBytes );
    testDecodedBytes( "single byte payload", headerBytes, 2, "\x00\x01" );
  }

  // FIN bit
  {
    ws::Header header;
    header.fin = true;
    header.payloadSize = 1;
    memset( headerBytes, 0x00, sizeof(headerBytes) );
    header.encode( headerBytes );
    testDecodedBytes( "FIN bit", headerBytes, 2, "\x80\x01" );
  }

  // Test OpCodes
  {
    ws::Header header;
    header.opCode = ws::Header::OpCode::eText;
    header.payloadSize = 10;
    memset( headerBytes, 0x00, sizeof(headerBytes) );
    header.encode( headerBytes );
    testDecodedBytes( "OpCode Text", headerBytes, 2, "\x01\x0A" );
  }

  {
    ws::Header header;
    header.opCode = ws::Header::OpCode::eBinary;
    header.payloadSize = 16;
    memset( headerBytes, 0x00, sizeof(headerBytes) );
    header.encode( headerBytes );
    testDecodedBytes( "OpCode Binary", headerBytes, 2, "\x02\x10" );
  }

  {
    ws::Header header;
    header.opCode = ws::Header::OpCode::eConnectionClose;
    header.payloadSize = 40;
    memset( headerBytes, 0x00, sizeof(headerBytes) );
    header.encode( headerBytes );
    testDecodedBytes( "OpCode ConnectionClose", headerBytes, 2, "\x08\x28" );
  }

  {
    ws::Header header;
    header.opCode = ws::Header::OpCode::ePing;
    header.payloadSize = 64;
    memset( headerBytes, 0x00, sizeof(headerBytes) );
    header.encode( headerBytes );
    testDecodedBytes( "OpCode Ping", headerBytes, 2, "\x09\x40" );
  }

  {
    ws::Header header;
    header.opCode = ws::Header::OpCode::ePong;
    header.payloadSize = 125;
    memset( headerBytes, 0x00, sizeof(headerBytes) );
    header.encode( headerBytes );
    testDecodedBytes( "OpCode Pong", headerBytes, 2, "\x0A\x7D" );
  }

  // Test 2-byte extended payload size
  {
    ws::Header header;
    header.payloadSize = 126;
    memset( headerBytes, 0x00, sizeof(headerBytes) );
    header.encode( headerBytes );
    testDecodedBytes( "2-byte size min", headerBytes, 4, "\x00\x7E\x00\x7E" );
  }

  {
    ws::Header header;
    header.payloadSize = 65535;
    memset( headerBytes, 0x00, sizeof(headerBytes) );
    header.encode( headerBytes );
    testDecodedBytes( "2-byte size max", headerBytes, 4, "\x00\x7E\xFF\xFF" );
  }

  // Test 8-byte extended payload size
  {
    ws::Header header;
    header.payloadSize = 65536;
    memset( headerBytes, 0x00, sizeof(headerBytes) );
    header.encode( headerBytes );
    testDecodedBytes( "8-byte size min", headerBytes, 10, "\x00\x7F\x00\x00\x00\x00\x00\x01\x00\x00" );
  }

  {
    ws::Header header;
    header.payloadSize = 4611686018427387904;
    memset( headerBytes, 0x00, sizeof(headerBytes) );
    header.encode( headerBytes );
    testDecodedBytes( "8-byte size 4611686018427387904", headerBytes, 10, "\x00\x7F\x40\x00\x00\x00\x00\x00\x00\x00" );
  }

  {
    ws::Header header;
    header.payloadSize = 9223372036854775807;
    memset( headerBytes, 0x00, sizeof(headerBytes) );
    header.encode( headerBytes );
    testDecodedBytes( "8-byte size max", headerBytes, 10, "\x00\x7F\x7F\xFF\xFF\xFF\xFF\xFF\xFF\xFF" );
  }

  // Test mask
  {
    ws::Header header;
    header.payloadSize = 1;
    header.isMasked = true;
    header.mask[0] = 0x0A;
    header.mask[1] = 0x0B;
    header.mask[2] = 0x0C;
    header.mask[3] = 0x0D;
    memset( headerBytes, 0x00, sizeof(headerBytes) );
    header.encode( headerBytes );
    testDecodedBytes( "mask", headerBytes, 6, "\x00\x81\x0A\x0B\x0C\x0D" );
  }

  // Mask + 2-byte extended payload size
  {
    ws::Header header;
    header.payloadSize = 258;
    header.isMasked = true;
    header.mask[0] = 0x0A;
    header.mask[1] = 0x0B;
    header.mask[2] = 0x0C;
    header.mask[3] = 0x0D;
    memset( headerBytes, 0x00, sizeof(headerBytes) );
    header.encode( headerBytes );
    testDecodedBytes( "2-byte size and mask", headerBytes, 8, "\x00\xFE\x01\x02\x0A\x0B\x0C\x0D" );
  }

  // Mask + 8-byte extended payload size
  {
    ws::Header header;
    header.payloadSize = 81985529216486895;
    header.isMasked = true;
    header.mask[0] = 0x0A;
    header.mask[1] = 0x0B;
    header.mask[2] = 0x0C;
    header.mask[3] = 0x0D;
    memset( headerBytes, 0x00, sizeof(headerBytes) );
    header.encode( headerBytes );
    testDecodedBytes( "8-byte size and mask", headerBytes, 14, "\x00\xFF\x01\x23\x45\x67\x89\xAB\xCD\xEF\x0A\x0B\x0C\x0D" );
  }
}

TEST(Encoding, WebSocketPayload)
{
  char encoded[11];
  memset( encoded, 0x00, sizeof(encoded) );

  // This is the masked frame example from RFC 6455. The unmasked payload is the
  // string "Hello".
  const uint8_t mask[4] = { 0x37, 0xFA, 0x21, 0x3D };

  // C-string API tests

  encoded[0] = '\0';
  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eNormal )
                                    , encoded );
  EXPECT_EQ( std::string( encoded ), "\x03\xE8" );

  encoded[0] = '\0';
  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eGoingAway )
                                    , encoded );
  EXPECT_EQ( std::string( encoded ), "\x03\xE9" );

  encoded[0] = '\0';
  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eProtocolError )
                                    , encoded );
  EXPECT_EQ( std::string( encoded ), "\x03\xEA" );

  encoded[0] = '\0';
  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eUnacceptableData )
                                    , encoded );
  EXPECT_EQ( std::string( encoded ), "\x03\xEB" );

  encoded[0] = '\0';
  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eMismatchedData )
                                    , encoded );
  EXPECT_EQ( std::string( encoded ), "\x03\xEF" );

  encoded[0] = '\0';
  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::ePolicyViolation )
                                    , encoded );
  EXPECT_EQ( std::string( encoded ), "\x03\xF0" );

  encoded[0] = '\0';
  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eTooMuchData )
                                    , encoded );
  EXPECT_EQ( std::string( encoded ), "\x03\xF1" );

  encoded[0] = '\0';
  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eLackingExtension )
                                    , encoded );
  EXPECT_EQ( std::string( encoded ), "\x03\xF2" );

  encoded[0] = '\0';
  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eUnexpectedCondition )
                                    , encoded );
  EXPECT_EQ( std::string( encoded ), "\x03\xF3" );

  encoded[0] = '\0';
  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::IANACode::eUnauthorised )
                                    , encoded );
  EXPECT_EQ( std::string( encoded ), "\x0B\xB8" );

  encoded[0] = '\0';
  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::IANACode::eForbidden )
                                    , encoded );
  EXPECT_EQ( std::string( encoded ), "\x0B\xBB" );

  encoded[0] = '\0';
  ws::encodeMaskedPayload( "", 0, mask, encoded );
  EXPECT_EQ( std::string( encoded ), "" );

  encoded[0] = '\0';
  ws::encodeMaskedPayload( "\x7F\x9F\x4D\x51\x58", 5, mask, encoded );
  EXPECT_EQ( std::string( encoded ), "Hello" );

  encoded[0] = '\0'; // Decoding is identical to encoding so we should be able to go back
  ws::encodeMaskedPayload( "Hello", 5, mask, encoded );
  EXPECT_EQ( std::string( encoded ), "\x7F\x9F\x4D\x51\x58" );

  // std::string in-place tests (just repeat C-string tests)
  std::string inplace;

  inplace.assign( "\x00\x00", 2 );
  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eNormal )
                                    , inplace );
  EXPECT_EQ( inplace, "\x03\xE8" );

  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eGoingAway )
                                    , inplace );
  EXPECT_EQ( inplace, "\x03\xE9" );

  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eProtocolError )
                                    , inplace );
  EXPECT_EQ( inplace, "\x03\xEA" );

  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eUnacceptableData )
                                    , inplace );
  EXPECT_EQ( inplace, "\x03\xEB" );

  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eMismatchedData )
                                    , inplace );
  EXPECT_EQ( inplace, "\x03\xEF" );

  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::ePolicyViolation )
                                    , inplace );
  EXPECT_EQ( inplace, "\x03\xF0" );

  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eTooMuchData )
                                    , inplace );
  EXPECT_EQ( inplace, "\x03\xF1" );

  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eLackingExtension )
                                    , inplace );
  EXPECT_EQ( inplace, "\x03\xF2" );

  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::ProtocolCode::eUnexpectedCondition )
                                    , inplace );
  EXPECT_EQ( inplace, "\x03\xF3" );

  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::IANACode::eUnauthorised )
                                    , inplace );
  EXPECT_EQ( inplace, "\x0B\xB8" );

  ws::closestatus::encodePayloadCode( ws::closestatus::toPayload( ws::closestatus::IANACode::eForbidden )
                                    , inplace );
  EXPECT_EQ( inplace, "\x0B\xBB" );

  inplace.assign( "" );
  ws::encodeMaskedPayload( inplace, mask );
  EXPECT_EQ( inplace, "" );

  inplace.assign( "\x7F\x9F\x4D\x51\x58" );
  ws::encodeMaskedPayload( inplace, mask );
  EXPECT_EQ( inplace, "Hello" );

  inplace.assign( "Hello" );
  ws::encodeMaskedPayload( inplace, mask );
  EXPECT_EQ( inplace, "\x7F\x9F\x4D\x51\x58" );

  // std::string copy tests (just repeat C-string tests)
  EXPECT_EQ( ws::encodeMaskedPayload( "", mask ), "" );
  EXPECT_EQ( ws::encodeMaskedPayload( "\x7F\x9F\x4D\x51\x58", mask ), "Hello" );
  EXPECT_EQ( ws::encodeMaskedPayload( "Hello", mask ), "\x7F\x9F\x4D\x51\x58" );
}
