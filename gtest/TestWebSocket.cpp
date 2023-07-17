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

TEST(Decoding, WebSocketFrame)
{
  // Note that these tests focus on the overall frame decoding. Header decoding
  // is tested separately in the Decoding_WebSocketHeader test so there is no
  // need to go over that again. One consequence of this is that we can limit
  // ourselves to payload sizes less than 126 bytes.

  char frameBytes[ 2 + 4 + 125 ]; // first two header bytes + header mask + paylaod

  // Decode
  // - no bytes
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    const auto decodeResult = decoder.decode( frameBytes, 0 );
    EXPECT_FALSE( decodeResult.parseError );
    EXPECT_TRUE( decodeResult.frames.empty() );
    EXPECT_EQ( decodeResult.numExtra, 0 );
  }

  // Decode
  // - single byte, insufficient even for a header
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    memcpy( frameBytes, "\x00", 1 );
    const auto decodeResult = decoder.decode( frameBytes, 1 );
    EXPECT_FALSE( decodeResult.parseError );
    EXPECT_TRUE( decodeResult.frames.empty() );
    EXPECT_EQ( decodeResult.numExtra, 1 );
  }

  // Decode
  // - two bytes, a header indicating zero-sized payload i.e. the smallest
  //   possible complete frame
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    memcpy( frameBytes, "\x00\x00", 2 );
    const auto decodeResult = decoder.decode( frameBytes, 2 );
    EXPECT_FALSE( decodeResult.parseError );
    ASSERT_EQ( decodeResult.frames.size(), 1 );
    testHeader( decodeResult.frames[0].header, false, ws::Header::OpCode::eContinuation, 0 );
    EXPECT_TRUE( decodeResult.frames[0].payload.empty() );
    EXPECT_EQ( decodeResult.numExtra, 0 );
  }

  // Decode
  // - three bytes, a header indicating a single byte payload and the payload
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    memcpy( frameBytes, "\x00\x01X", 3 );
    const auto decodeResult = decoder.decode( frameBytes, 3 );
    EXPECT_FALSE( decodeResult.parseError );
    ASSERT_EQ( decodeResult.frames.size(), 1 );
    testHeader( decodeResult.frames[0].header, false, ws::Header::OpCode::eContinuation, 1 );
    ASSERT_EQ( decodeResult.frames[0].payload.size(), 1 );
    EXPECT_EQ( decodeResult.frames[0].payload[0], 'X' );
    EXPECT_EQ( decodeResult.numExtra, 0 );
  }

  // Decode
  // - three bytes, a header indicating a three byte payload and the first payload byte
  // - two bytes, the remaining two payload bytes
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    memcpy( frameBytes, "\x00\x03XYZ", 5 );
    const auto decodeResult1 = decoder.decode( frameBytes, 3 );
    EXPECT_FALSE( decodeResult1.parseError );
    EXPECT_TRUE( decodeResult1.frames.empty() );
    EXPECT_EQ( decodeResult1.numExtra, 1 );
    const auto decodeResult2 = decoder.decode( frameBytes + 3, 2 );
    EXPECT_FALSE( decodeResult2.parseError );
    ASSERT_EQ( decodeResult2.frames.size(), 1 );
    testHeader( decodeResult2.frames[0].header, false, ws::Header::OpCode::eContinuation, 3 );
    ASSERT_EQ( decodeResult2.frames[0].payload.size(), 3 );
    EXPECT_EQ( decodeResult2.frames[0].payload, "XYZ" );
    EXPECT_EQ( decodeResult2.numExtra, 0 );
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
    EXPECT_FALSE( decodeResult1.parseError );
    EXPECT_TRUE( decodeResult1.frames.empty() );
    EXPECT_EQ( decodeResult1.numExtra, 1 );
    const auto decodeResult2 = decoder.decode( frameBytes + 1, 3 );
    EXPECT_FALSE( decodeResult2.parseError );
    EXPECT_TRUE( decodeResult2.frames.empty() );
    EXPECT_EQ( decodeResult2.numExtra, 2 );
    const auto decodeResult3 = decoder.decode( frameBytes + 4, 7 );
    EXPECT_FALSE( decodeResult3.parseError );
    ASSERT_EQ( decodeResult3.frames.size(), 1 );
    testHeader( decodeResult3.frames[0].header, false, ws::Header::OpCode::eContinuation, 9 );
    ASSERT_EQ( decodeResult3.frames[0].payload.size(), 9 );
    EXPECT_EQ( decodeResult3.frames[0].payload, "abcDEF[]!" );
    EXPECT_EQ( decodeResult3.numExtra, 0 );
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
    EXPECT_FALSE( decodeResult1.parseError );
    EXPECT_TRUE( decodeResult1.frames.empty() );
    EXPECT_EQ( decodeResult1.numExtra, 1 );
    const auto decodeResult2 = decoder.decode( frameBytes + 1, 3 );
    EXPECT_FALSE( decodeResult2.parseError );
    EXPECT_TRUE( decodeResult2.frames.empty() );
    EXPECT_EQ( decodeResult2.numExtra, 2 );
    const auto decodeResult3 = decoder.decode( frameBytes + 4, 7 );
    EXPECT_FALSE( decodeResult3.parseError );
    ASSERT_EQ( decodeResult3.frames.size(), 1 );
    testHeader( decodeResult3.frames[0].header, false, ws::Header::OpCode::eContinuation, 9 );
    ASSERT_EQ( decodeResult3.frames[0].payload.size(), 9 );
    EXPECT_EQ( decodeResult3.frames[0].payload, "abcDEF[]!" );
    EXPECT_EQ( decodeResult3.numExtra, 0 );
  }

  // Decode
  // - 17 bytes, a six byte header with a no-op masked 11 byte payload
  {
    ws::Decoder decoder;
    memset( frameBytes, 0x00, sizeof(frameBytes) );
    memcpy( frameBytes, "\x00\x8B\x00\x00\x00\x00[123456789]", 17 );
    const auto decodeResult = decoder.decode( frameBytes, 17 );
    EXPECT_FALSE( decodeResult.parseError );
    ASSERT_EQ( decodeResult.frames.size(), 1 );
    ASSERT_EQ( decodeResult.frames[0].payload.size(), 11 );
    EXPECT_EQ( decodeResult.frames[0].payload, "[123456789]" );
    EXPECT_EQ( decodeResult.numExtra, 0 );
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
    EXPECT_FALSE( decodeResult.parseError );
    ASSERT_EQ( decodeResult.frames.size(), 1 );
    ASSERT_EQ( decodeResult.frames[0].payload.size(), 5 );
    EXPECT_EQ( decodeResult.frames[0].payload, "Hello" );
    EXPECT_EQ( decodeResult.numExtra, 0 );
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
  size_t numDecodedBytes;

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
  ws::encodeMaskedPayload( "", 0, mask, encoded );
  EXPECT_EQ( std::string( encoded ), "" );

  encoded[0] = '\0';
  ws::encodeMaskedPayload( "\x7F\x9F\x4D\x51\x58", 5, mask, encoded );
  EXPECT_EQ( std::string( encoded ), "Hello" );

  encoded[0] = '\0'; // Decoding is identical to encoding so we should be able to go back
  ws::encodeMaskedPayload( "Hello", 5, mask, encoded );
  EXPECT_EQ( std::string( encoded ), "\x7f\x9f\x4d\x51\x58" );

  // std::string in-place tests (just repeat C-string tests)
  std::string inplace;
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
