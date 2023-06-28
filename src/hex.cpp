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

#include <lb/encoding/hex.h>


namespace lb
{


namespace encoding
{


namespace hex
{


void encode( char src, char* dst )
{
  const auto firstDigit{ ( src & 0xF0 ) >> 4 };
  if ( firstDigit < 10 )
  {
    dst[0] = '0' + firstDigit;
  }
  else
  {
    dst[0] = 'A' + firstDigit - 10;
  }
  const auto secondDigit{ src & 0x0F };
  if ( secondDigit < 10 )
  {
    dst[1] = '0' + secondDigit;
  }
  else
  {
    dst[1] = 'A' + secondDigit - 10;
  }
}

void encode( const char* src, size_t numSrcBytes, char* dst )
{
  for ( size_t i = 0; i < numSrcBytes; ++i, ++src, dst += 2 )
  {
    encode( *src, dst );
  }
}


} // End of namespace hex


} // End of namespace encoding


} // End of namespace lb
