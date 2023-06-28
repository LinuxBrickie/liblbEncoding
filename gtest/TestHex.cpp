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

#include <lb/encoding/hex.h>


TEST(Encoding, Hex)
{
  // Single byte tests
  char twoDigits[2];
  lb::encoding::hex::encode( 0x00, twoDigits );
  EXPECT_EQ( twoDigits[0], '0' );
  EXPECT_EQ( twoDigits[1], '0' );
  lb::encoding::hex::encode( 0x3d, twoDigits );
  EXPECT_EQ( twoDigits[0], '3' );
  EXPECT_EQ( twoDigits[1], 'D' );
  lb::encoding::hex::encode( 0xFF, twoDigits );
  EXPECT_EQ( twoDigits[0], 'F' );
  EXPECT_EQ( twoDigits[1], 'F' );

  // Mutli byte tests
  std::string src( "The quick brown fox jumps over the lazy dog." );
  char multiDigits[ src.size() * 2 + 1 ];
  multiDigits[ src.size() * 2 ] = '\0';
  lb::encoding::hex::encode( src.c_str(), src.size(), multiDigits );
  std::string dst( multiDigits );
  EXPECT_EQ( dst, "54686520717569636B2062726F776E20666F78206A756D7073206F76657220746865206C617A7920646F672E" );

}
