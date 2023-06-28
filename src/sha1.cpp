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

#include <lb/encoding/sha1.h>
#include <lb/encoding/hex.h>

#include <cstdint>
#include <cstring>
#include <memory>


namespace lb
{


namespace encoding
{


namespace sha1
{


/** \brief This returns the distance to the next number up from \a n which is
           equivalent to \a target under modulo \a modulo.

    For example, suppose n is 99 and the target is 5 with modulo 20. The next
    number up from 99 that is of the form 20k + 5 is when k is 5 i.e. 105. The
    distance from n to 105 is 6.

 */
template< class T >
T findDistanceToTargetModulo( T n, T target, T modulo )
{
  const auto nm{ n % modulo };
  if ( target >= nm )
  {
    return target - nm;
  }
  else
  {
    return target - nm + modulo;
  }
}

void encode( const char* src, size_t numSrcChars, char* dst )
{
  // This code has been written to be endian agnostic. It has only been tested
  // on a little endian system though...

  // Message length in bits.
  const uint64_t ml = numSrcChars * 8;

  // We will append the bit '1' to the message and then append 0 ≤ k < 512 bits
  // '0', such that the resulting message length in bits is congruent to 448
  // mod 512. We then add on the (original?) ml as a 64-bit integer giving a
  // total length that is a multiple of 512 bits.
  const uint64_t numZeroBits{ findDistanceToTargetModulo<uint64_t>( ml + 1, 448U, 512U ) };

  // Append ml, the original message length in bits, as a 64-bit big-endian
  // integer.

  const size_t N = ( ml + 1 + numZeroBits  + 64 )/ 8;

  // Heap allocate for now. We could do variable sized array to get stack
  // allocation for short strings I suppose. There are all sorts of things we
  // could do to optimise this allocation but let's not be premature...
  //
  // Note that the memory will be initialised to zero so we do not have to
  // explcitily set numZeroBits to '0'.
  std::unique_ptr<uint8_t[]> buffer{ std::make_unique<uint8_t[]>( N ) };

  std::memcpy( buffer.get(), src, numSrcChars );
  buffer.get()[ numSrcChars ] = 0x80; // set a '1' immediately after the message

  buffer.get()[ N - 4 ] = (uint8_t)(ml >> 24);
  buffer.get()[ N - 3 ] = (uint8_t)(ml >> 16);
  buffer.get()[ N - 2 ] = (uint8_t)(ml >>  8);
  buffer.get()[ N - 1 ] = (uint8_t)(ml >>  0);

  // All constants are big endian. Within each word, the most significant byte
  // is stored in the leftmost byte position.

  // Big Endian representations
  uint32_t h0 = 0x67452301; // little endian 0x01234567
  uint32_t h1 = 0xEFCDAB89; // little endian 0x89ABCDEF
  uint32_t h2 = 0x98BADCFE; // little endian 0xEFDCBA98
  uint32_t h3 = 0x10325476; // little endian 0x76543210
  uint32_t h4 = 0xC3D2E1F0; // little endian 0xF0E1D2C3

  uint32_t bigEndianWords[80];

  // Process the message in successive 512-bit chunks (64 bytes)
  // Break chunk into sixteen 32-bit big-endian words w[i], 0 ≤ i ≤ 15 which
  // we extend to 80 later.
  //
  // Note that the following loops are wasteful on big-endian architecture as
  // we could just do std::memcpy( w, p, 64 ) but this way the code is endian
  // agnostic.
  uint32_t* p{ (uint32_t*)buffer.get() };
  for ( int chunk = 0; chunk < N; chunk += 64 )
  {
    uint8_t* w{ (uint8_t*)bigEndianWords };

    for ( int iw = 0; iw < 16; ++iw, p += 1 )
    {
      for ( int i = 0; i < 4; ++i, ++w )
      {
        *w = (uint8_t)(*p >> 8*(3-i));
      }
    }

    // Message schedule: extend the sixteen 32-bit words into eighty 32-bit words:
    uint32_t *bw = bigEndianWords;
    for ( int i = 16; i < 80; ++i )
    {
      bw[i] = bw[i-3] ^ bw[i-8] ^ bw[i-14] ^ bw[i-16];
      // Now left rotate the bits by one
      bw[i] = ( bw[i] << 1 ) | ( bw[i] >> 31 );
    }

    // Initialize hash value for this chunk:
    uint32_t a = h0;
    uint32_t b = h1;
    uint32_t c = h2;
    uint32_t d = h3;
    uint32_t e = h4;
    uint32_t k = 0x5A827999;
    // Main loop. Manually unrolled.

    for ( int i = 0; i < 20; ++i )
    {
      uint32_t f = ( b & c ) | ( ( ~b ) & d );

      const uint32_t temp = ( ( a << 5 ) | ( a >> 27 ) ) + f + e + k + bw[i];
      e = d;
      d = c;
      c = ( b << 30 ) | ( b >> 2 );
      b = a;
      a = temp;
    }

    k = 0x6ED9EBA1;
    for ( int i = 20; i < 40; ++i )
    {
      uint32_t f = b ^ c ^ d;

      const uint32_t temp = ( ( a << 5 ) | ( a >> 27 ) ) + f + e + k + bw[i];
      e = d;
      d = c;
      c = ( b << 30 ) | ( b >> 2 );
      b = a;
      a = temp;
    }

    k = 0x8F1BBCDC;
    for ( int i = 40; i < 60; ++i )
    {
      uint32_t f = ( b & c ) | ( b & d ) | ( c & d );

      const uint32_t temp = ( ( a << 5 ) | ( a >> 27 ) ) + f + e + k + bw[i];
      e = d;
      d = c;
      c = ( b << 30 ) | ( b >> 2 );
      b = a;
      a = temp;
    }

    k = 0xCA62C1D6;
    for ( int i = 60; i < 80; ++i )
    {
      uint32_t f = b ^ c ^ d;

      const uint32_t temp = ( ( a << 5 ) | ( a >> 27 ) ) + f + e + k + bw[i];
      e = d;
      d = c;
      c = ( b << 30 ) | ( b >> 2 );
      b = a;
      a = temp;
    }

    // Add this chunk's hash to result so far.
    h0 = h0 + a;
    h1 = h1 + b;
    h2 = h2 + c;
    h3 = h3 + d;
    h4 = h4 + e;
  }

  // The final hash value (big-endian) is a 160-bit number of the form:
  //
  //  (h0 << 128) | (h1 << 96) | (h2 << 64) | (h3 << 32) | h4
  //
  // Convert each byte to a 2-digit hex value (without using std::stringstream
  // to avoid overhead).
  char tmp[2];
  for ( int i = 0; i < 4; ++i )
  {
    const auto twoi{ 2 * i };
    const auto eighti{ 8 * i };
    char tmp[2];
    hex::encode( uint8_t(h0 >> eighti), tmp );
    dst[ 6 - twoi      ] = tmp[0];
    dst[ 6 - twoi +  1 ] = tmp[1];
    hex::encode( uint8_t(h1 >> eighti), tmp );
    dst[ 6 - twoi +  8 ] = tmp[0];
    dst[ 6 - twoi +  9 ] = tmp[1];
    hex::encode( uint8_t(h2 >> eighti), tmp );
    dst[ 6 - twoi + 16 ] = tmp[0];
    dst[ 6 - twoi + 17 ] = tmp[1];
    hex::encode( uint8_t(h3 >> eighti), tmp );
    dst[ 6 - twoi + 24 ] = tmp[0];
    dst[ 6 - twoi + 25 ] = tmp[1];
    hex::encode( uint8_t(h4 >> eighti), tmp );
    dst[ 6 - twoi + 32 ] = tmp[0];
    dst[ 6 - twoi + 33 ] = tmp[1];
  }
}


std::string encode( const std::string& src )
{
  const size_t requiredStorage{ 40 };

  std::unique_ptr<char[]> dst{ std::make_unique<char[]>( requiredStorage ) };

  encode( src.c_str(), src.size(), dst.get() );

  // std::string always takes a copy. Understanable, but unfortunate here. If
  // only there was some sort of move semantics for passing C-style string
  // ownership to std::string.
  return std::string( dst.get(), requiredStorage );
}


} // End of namespace sha1


} // End of namespace encoding


} // End of namespace lb
