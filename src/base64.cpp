
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

#include <lb/encoding/base64.h>

#include <memory>


namespace lb
{


namespace encoding
{


namespace base64
{


/** \brief A lookup array for the 64 base64 characters. */
const char base64Lookup[ 65 ]
{
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
};

/**
    \brief Converts 3 bytes of data to 4 bytes of base64 encoded data.
    \param src The source bytes as an unsigned type to ensure bit shifting
               yields zeros. Assumes 3 contiguous bytes can be accessed.
    \param dst The destination buffer. Assumes that 4 contiguous bytes can be
               written to.

    Endian-agnostic.
 */
void encodeTriplet( const unsigned char* src, char* dst )
{
  dst[0] = base64Lookup[ (unsigned char)(   *src >> 2 ) ];
  dst[1] = base64Lookup[ (unsigned char)( ( ( *src << 4 ) & 0x30 ) | ( *(src+1) >> 4 ) ) ];
  dst[2] = base64Lookup[ (unsigned char)( ( ( *(src+1) << 2 ) & 0x3F ) | ( *(src+2) >> 6 ) ) ];
  dst[3] = base64Lookup[ (unsigned char)( ( *(src+2) & 0x3F ) ) ];
}

/**
    \brief Converts 1 byte of data to 4 bytes of base64 encoded data. The final
           2 destination bytes are padding bytes i.e. "==".
    \param src The source bytes as an unsigned type to ensure bit shifting
               yields zeros. Assumes 3 contiguous bytes can be accessed.
    \param dst The destination buffer. Assumes that 4 contiguous bytes can be
               written to.

    Endian-agnostic.
 */
void encodeDoublet( const unsigned char* src, char* dst )
{
  dst[0] = base64Lookup[ (unsigned char)(   *src >> 2 ) ];
  dst[1] = base64Lookup[ (unsigned char)( ( ( *src << 4 ) & 0x30 ) | ( *(src+1) >> 4 ) ) ];
  dst[2] = base64Lookup[ (unsigned char)( ( *(src+1) << 2 ) & 0x3F ) ];
  dst[3] = '=';
}

/**
    \brief Converts 2 bytes of data to 4 bytes of base64 encoded data. The final
           destination byte is a padding byte i.e. '='.
    \param src The source bytes as an unsigned type to ensure bit shifting
               yields zeros. Assumes 3 contiguous bytes can be accessed.
    \param dst The destination buffer. Assumes that 4 contiguous bytes can be
               written to.

    Endian-agnostic.
 */
void encodeSinglet( const unsigned char* src, char* dst )
{
  dst[0] = base64Lookup[ (unsigned char)(   *src >> 2 ) ];
  dst[1] = base64Lookup[ (unsigned char)( ( *src << 4 ) & 0x30 ) ];
  dst[2] = '=';
  dst[3] = '=';
}

void encode( const char* src, size_t numSrcChars, char* dst )
{
  const size_t extra{ numSrcChars % 3 };
  const size_t numTriplets{ ( numSrcChars - extra ) / 3 };

  unsigned char* usrc{ (unsigned char*)src };

  for ( size_t i = 0; i < numTriplets; ++i, usrc += 3, dst += 4 )
  {
    encodeTriplet( usrc, dst );
  }

  switch( extra )
  {
  case 0:
    break;
  case 1:
    encodeSinglet( usrc, dst );
    break;
  case 2:
    encodeDoublet( usrc, dst );
    break;
  default: // Impossible
    break;
  }
}

std::string encode( const std::string& src )
{
  if ( src.empty() )
  {
    return {};
  }

  const size_t requiredStorage{ 4 * ( (src.size()-1)/3 + 1) };

  std::unique_ptr<char[]> dst{ std::make_unique<char[]>( requiredStorage ) };

  encode( src.c_str(), src.size(), dst.get() );

  // std::string always takes a copy. Understanable, but unfortunate here. If
  // only there was some sort of move semantics for passing C-style string
  // ownership to std::string.
  return std::string( dst.get(), requiredStorage );
}


} // End of namespace base64


} // End of namespace encoding


} // End of namespace lb
