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

#include <sstream>

#include <lb/encoding/bits.h>


TEST(Encoding, Bits)
{
  std::ostringstream oss;

  oss << lb::encoding::bits::Printer( '\0' );
  EXPECT_EQ( oss.str(), "00000000" ); // 0 in decimal, 0x00
  oss.str( {} );

  oss << lb::encoding::bits::Printer( 'a' );
  EXPECT_EQ( oss.str(), "01100001" ); // 97 in decimal, 0x61
  oss.str( {} );

  oss << lb::encoding::bits::Printer( '5' );
  EXPECT_EQ( oss.str(), "00110101" ); // 53 in decimal, 0x35
  oss.str( {} );

  oss << lb::encoding::bits::Printer( '~' );
  EXPECT_EQ( oss.str(), "01111110" ); // 126 in decimal, 0x7E
  oss.str( {} );

  oss << lb::encoding::bits::Printer( 0x51 );
  EXPECT_EQ( oss.str(), "01010001" ); // 81 in decimal, 'Q'
  oss.str( {} );

  oss << lb::encoding::bits::Printer( 0x80 );
  EXPECT_EQ( oss.str(), "10000000" ); // 128 in decimal
  oss.str( {} );

  oss << lb::encoding::bits::Printer( 0xAB );
  EXPECT_EQ( oss.str(), "10101011" ); // 171 in decimal
  oss.str( {} );

  oss << lb::encoding::bits::Printer( 0xFF );
  EXPECT_EQ( oss.str(), "11111111" ); // 255 in decimal
  oss.str( {} );
}
