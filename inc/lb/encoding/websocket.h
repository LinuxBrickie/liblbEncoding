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
#include <optional>
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

  /** \brief The size in bytes this \a Header requires when encoded. */
  uint8_t encodedSizeInBytes() const;

  /**
      \brief The size in bytes a \a Header with \a payloadSize and \a isMasked
             would require when encoded.
   */
  static uint8_t encodedSizeInBytes( size_t payloadSize, bool isMasked );

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

    \sa encodeMaskedPayload( std::string&, const uint8_t[4] )
    \sa encodeMaskedPayload( const std::string&, const uint8_t[4] )
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

    \sa encodeMaskedPayload( const char*, size_t, const uint8_t[4], char* )
    \sa encodeMaskedPayload( const std::string&, const uint8_t[4] )
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

    \sa encodeMaskedPayload( const char*, size_t, const uint8_t[4], char* )
    \sa encodeMaskedPayload( std::string&, const uint8_t[4] )
 */
std::string encodeMaskedPayload( const std::string& src
                               , const uint8_t mask[4] );
std::string decodeMaskedPayload( const std::string& src
                               , const uint8_t mask[4] );

namespace closestatus
{

//! The two byte integer value directly from the payload.
using PayloadCode = unsigned int;

/**
   From RFC6455 Section 7.4.2:

   Reserved Status Code Ranges

   0-999
      Status codes in the range 0-999 are not used.

   1000-2999
      Status codes in the range 1000-2999 are reserved for definition by
      this protocol, its future revisions, and extensions specified in a
      permanent and readily available public specification.

   3000-3999
      Status codes in the range 3000-3999 are reserved for use by
      libraries, frameworks, and applications.  These status codes are
      registered directly with IANA.  The interpretation of these codes
      is undefined by this protocol.

   4000-4999
      Status codes in the range 4000-4999 are reserved for private use
      and thus can't be registered.  Such codes can be used by prior
      agreements between WebSocket applications.  The interpretation of
      these codes is undefined by this protocol.
 */
enum class CodeRange
{
  eUnused,   //!<    0 -  999
  eProtocol, //!< 1000 - 2999
  eIANA,     //!< 3000 - 3999
  ePrivate,  //!< 4000 - 4999
  eOutside   //!< 5000 onwards
};

// For logging and debugging
std::string toString( CodeRange );

/** \brief Provides the CloseStatusCodeRange for the numeric code. */
CodeRange toCodeRange( PayloadCode );


/**
    \brief The valid payload status codes for CloseStatusCodeRange::eProtocol
 */
enum class ProtocolCode
{
  eNormal              = 1000,
  eGoingAway           = 1001,
  eProtocolError       = 1002,
  eUnacceptableData    = 1003,
  eNoCodeProvided      = 1005, //!< Only ever set in the *absence* of a code
  eMismatchedData      = 1007,
  ePolicyViolation     = 1008,
  eTooMuchData         = 1009,
  eLackingExtension    = 1010, //!< Client only
  eUnexpectedCondition = 1011, //!< Server only
};

// For logging and debugging
std::string toString( ProtocolCode );

/**
    \brief Convert the numeric code to a specific ProtocolCode.
    \return The ProtocolCode or an empty optional if there is no match.
 */
std::optional<ProtocolCode> toProtocol( PayloadCode );

/** \brief Convert the ProtocolCode to a numeric code. */
PayloadCode toPayload( ProtocolCode );

/**
    \brief The valid payload status codes for CloseStatusCodeRange::eIANA as of
           August 2023.
 */
enum class IANACode
{
  eUnauthorised = 3000,
  eForbidden    = 3003
};

// For logging and debugging
std::string toString( IANACode );

/**
    \brief Convert the numeric code to a specific IANACloseStatusCode.
    \throw std::runtime_error if conversion fails.
 */
std::optional<IANACode> toIANA( PayloadCode );

/** \brief Convert the IANACode to a numeric code. */
PayloadCode toPayload( IANACode );


/**
    \brief Encodes \a payloadCode into \a dst as a two-byte integer stored
           in network order.
    \param closeStatusCode The status code to encode.
    \param dst The destination for the encoding. Assumes that two contiguous
               bytes are available for access.
    \sa encodePayloadCode( CloseStatusCode, std::string& )

    Note that an attempt to set CloseStatusCode::eNoCodeProvided will be
    silently ignored.
 */
void encodePayloadCode( PayloadCode payloadCode, char* dst );

/**
    \brief Encodes \a closeStatusCode into \a dst as a two-byte integer stored
           in network order.
    \param closeStatusCode The status code to encode.
    \param dst The destination for the encoding. Assumes that two contiguous
               bytes are available for access.
    \sa encodePayloadCode( CloseStatusCode, char* )

    Note that an attempt to set CloseStatusCode::eNoCodeProvided will be
    silently ignored.
 */
void encodePayloadCode( PayloadCode payloadCode, std::string& dst );

/**
    \brief Decodes two bytes of \a src into a PayloadCode assuming that they
          represent an integer stored in network order.
    \param src The source for the decoding. Less than two characters may be passed.
    \return The decoded PayloadCode or ProtocolCode::eNoCodeProvided if absent.
    \sa decodePayloadCode( PayloadCode, std::string& )

    If \a src contains less than two characters then the code is treated as
    absent and the value associated with ProtocolCode::eNoCodeProvided is
    returned.
 */
PayloadCode decodePayloadCode( const char* src, size_t numSrcChars );

/**
    \brief Decodes two bytes of \a src into a PayloadCode assuming that they
          represent an integer stored in network order.
    \param src The source for the decoding. Less than two characters may be passed.
    \return The decoded PayloadCode or 0 if absent.
    \sa decodePayloadCode( PayloadCode, const char*, size_t )

    If \a src contains less than two characters then the code is treated as
    absent and the value associated with ProtocolCode::eNoCodeProvided is
    returned.
 */
PayloadCode decodePayloadCode( const std::string& src );


} // End of namespace closestatus


} // End of namespace websocket


} // End of namespace encoding


} // End of namespace lb


#endif // LB_ENCODING_WEBSOCKET_H
