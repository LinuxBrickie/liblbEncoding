#ifndef LB_ENCODING_WEBSOCKET_H
#define LB_ENCODING_WEBSOCKET_H

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


#include <cstdint>
#include <memory>
#include <string>
#include <vector>


namespace lb
{


namespace encoding
{


namespace websocket
{


/** \brief The (at-most) fourteen-bytes header of a frame.

    Following diagram taken from Wikipedia

    0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    FIN  RSV1 RSV2 RSV3 Opcode              Mask Payload length
    Extended payload length (optional)
    Masking key (optional)
    Payload data

    - FIN bit indicates the final fragment in a message.
    - RSVx bits MUST be 0 unless defined by an extension.
    - Opcode
    -- 0  Continuation frame
    -- 1  Text frame
    -- 2  Binary frame
    -- 8  Connection close
    -- 9  Ping
    -- A  Pong
    - Mask bit is set to 1 if the payload data is masked.
    - Payload length
    -- 0-125  This is the payload length.
    -- 126    The following 2 bytes are the payload length.
    -- 127    The following 8 bytes are the payload length.
    - Masking key, 4 bytes. All frames sent from the client should be masked
      by this key. This field is absent if the mask bit is set to 0.

    The header may be varying sizes depending on how the payload length is
    encoded and whether a masking key is present. The smallest possible
    header size is two bytes and the longest is fourteen. Valid sizes in
    between are four, six, eight and ten bytes.
*/
struct Header
{  
  static const size_t minSizeInBytes{  2 };
  static const size_t maxSizeInBytes{ 14 };


  // Fields
  bool fin { false };
  bool rsv1{ false };
  bool rsv2{ false };
  bool rsv3{ false };
  enum class OpCode
  {
    eContinuation    = 0x00,
    eText            = 0x01,
    eBinary          = 0x02,
    eConnectionClose = 0x08,
    ePing            = 0x09,
    ePong            = 0x0A
  }
  opCode{ OpCode::eContinuation };
  static std::string toString( OpCode );

  bool isMasked{ false };
  uint64_t payloadSize{ 0 };
  uint8_t mask[4] { 0, 0, 0, 0 };


  // Methods
  uint8_t serialisedSizeInBytes() const;

  enum class DecodeResult
  {
    eSuccess,
    eIncomplete,
    eInvalidOpCode,
    ePayloadSizeInflatedEncoding,
    ePayloadSizeEighthByteMSBNotZero
  };
  static std::string toString( DecodeResult );

  /** \brief Decodes the bytes in \a src into this \a Header.
      \param src May contain more bytes than required, the extra bytes are ignored.
      \param numSrcBytes The number of available bytes in \a src.
      \return An enum value describing the outcome of the decoding. This \a Header
              will only be valid if the return is \a DecodeStatus::eSuccess.

      Once decoding has finished any extraneous bytes in \a src are simply
      ignored.
   */
  DecodeResult decode( const char* src, size_t numSrcBytes );

  /** \brief Encodes this \a Header to \a dst.
      \param dst The destination for the encoding. Assumes that sufficient contiguous
                 bytes are available for access, see description for details.

      The required minimum size of \a dst is serialisedSizeInBytes().

      Unlike decoding, encoding cannot fail (provided \a dst is big enough) and
      so there is no return value.
  */
  void encode( char* dst ) const;
};


/** \brief A WebSocket frame. Messages are composed of one or more frames.

    A frame consists of a header and a payload.

    See RFC 6455 for detail.
 */
struct Frame
{
  Header header;
  std::string payload;
};


/** \brief Decodes one or more byte buffers into zero or more frames.

    See the documentation for the \a decode method for information.
 */
class Decoder
{
public:
  /**
      \brief Construct a Decoder. Keeps track of decoding across multiple byte buffers.
      \param cacheReserveSize Number of bytes to reserve for cached data, either
             header or payload.
   */
  Decoder( size_t cacheReserveSize = 1024 );
  ~Decoder();

  // Default move construction and move assignment. Copy forbidden.
  Decoder( Decoder&& ) = default;
  Decoder& operator=( Decoder& ) = default;
  Decoder( const Decoder& ) = delete;
  Decoder& operator=( const Decoder& ) = delete;

  struct Result
  {
    bool parseError{ false };

    std::vector<Frame> frames;

    /** \brief Number of extra bytes left over after parsing the last complete
               frame (if any) or complete header. A copy of these extra bytes
               are retained in the parser for future calls to append to. This
               is just intended as an indicator to the caller that a partial
               frame is pending.
     */
    size_t numExtra{ 0 };
  };

  /** \brief Decodes the bytes in \a src into zero or more frames. Can be called
             respeatedly to build up a contiguous set of bytes.
      \param src Bytes containing all or part of one or more frames.
      \param numSrcBytes The number of available bytes in \a src. All bytes will
             be inspected even if a complete frame is found before the end.
      \return A \a Result struct containing the decoded frames, if any.

      This is designed to work with a stream of bytes being received over a
      network. When a collection of bytes is received, such as from a call to
      recv, they can be passed into this method. It will attempt to decode them
      as a WebSocket \a Header followed by the payload. Any full frames found
      will be returned in \a Result. Any remaining bytes will be still be
      inspected and if a \Header is found it will be cached along with the
      remaining incomplete payload bytes. If no \a Header is found then the
      remaining incomplete \a Header bytes are cached. The cached information
      is prepended to the bytes on the next invocation to allow decoding to
      proceed.

      As an example, take the extreme case where you only feed in a single byte
      at a time to this method. The first call cannot return any frames as the
      smallest possible \a Header is two bytes. The first byte is cached and
      when a second call is made with the second byte the bytes are concatenated
      and inspected as a whole. If a valid \a Header is found then this is cached
      and on the third and subsequent calls the payload is built up. Eventually
      the frame will be complete and appear in \a Result.

      On the other hand you might feed in a large number of bytes to a single
      call that encompass multiple frames. They will all be decoded and made
      available in \a Result. Again any extraneous bytes will be cached either
      directly
   */
  Result decode( const char* src, size_t numSrcBytes );

private:
  struct Private;
  std::unique_ptr<Private> d;
};

// There is no encode function for Frame. Simply do a Header::encode and then
// append the payload bytes if unmasked. If masked then pass the payload bytes
// through one of the encodeMaskedPayload overloads first.

/**
    \brief Encodes \a numSrcChars bytes of data from \a src by applying the
           WebSocket payload mask (found in the \a Header).
    \param src The bytes to encode (may contain nulls).
    \param numSrcChars The number of bytes to encode.
    \param mask The four byte mask from the \a Header.
    \param dst The destination for the encoding. Assumes that the same number
               of contiguous bytes are available for access. If desired, \a dst
               may be be the same as \a src i.e. in-place conversion supported.

    Note that decoding is the same operation, you can either call this function
    for decoding or the wrapper decodeMaskedPayload. If you use \a Decoder then
    this is all done for you anyway.
 */
void encodeMaskedPayload( const char* src
                        , size_t numSrcChars
                        , const uint8_t mask[4]
                        , char* dst );
void decodeMaskedPayload( const char* src
                        , size_t numSrcChars
                        , const uint8_t mask[4]
                        , char* dst );

/**
    \brief Encodes the data in \a src in-place by applying the WebSocket payload
           mask (found in the \a Header).
    \param src The bytes to encode in the form of a std::string (may contain nulls).
    \param mask The four byte mask from the \a Header.

    This is a std::string variant of the C_string version. In order to do the
    masking in-place it uses different code so is not a wrapper as such.

    Note that decoding is the same operation, you can either call this function
    for decoding or the wrapper decodeMaskedPayload. If you use \a Decoder then
    this is all done for you anyway.
 */
void encodeMaskedPayload( std::string& src
                        , const uint8_t mask[4] );
void decodeMaskedPayload( std::string& src
                        , const uint8_t mask[4] );

/**
    \brief Encodes the data in \a src by applying the WebSocket payload mask
           (found in the \a Header).
    \param src The bytes to encode in the form of a std::string (may contain nulls).
    \param mask The four byte mask from the \a Header.
    \return The masked string. Size will be identical to \a src.

    This is a std::string wrapper for the C_string version. Beware this does
    have a C-string copy overhead when creating the return value. Use the
    in-place std::string overload if you do not want a copy.

    Note that decoding is the same operation, you can either call this function
    for decoding or the wrapper decodeMaskedPayload. If you use \a Decoder then
    this is all done for you anyway.
 */
std::string encodeMaskedPayload( const std::string& src
                               , const uint8_t mask[4] );
std::string decodeMaskedPayload( const std::string& src
                               , const uint8_t mask[4] );


} // End of namespace websocket


} // End of namespace encoding


} // End of namespace lb

#endif // LB_ENCODING_WEBSOCKET_H
