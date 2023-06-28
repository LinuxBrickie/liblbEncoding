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

#include <lb/encoding/sha1.h>


TEST(Encoding, Sha1)
{
  // C-string API tests
  char encoded[128];

  encoded[40] = '\0';
  lb::encoding::sha1::encode( "", 0, encoded );
  EXPECT_EQ( std::string( encoded ), "DA39A3EE5E6B4B0D3255BFEF95601890AFD80709" );

  encoded[40] = '\0';
  lb::encoding::sha1::encode( "The quick brown fox jumps over the lazy dog", 43, encoded );
  EXPECT_EQ( std::string( encoded ), "2FD4E1C67A2D28FCED849EE1BB76E7391B93EB12" );

  encoded[40] = '\0';
  lb::encoding::sha1::encode( "The quick brown fox jumps over the lazy cog", 43, encoded );
  EXPECT_EQ( std::string( encoded ), "DE9F2C7FD25E1B3AFAD3E85A0BD17D9B100DB4B3" );

  // std::string API tests (just repeat C-string tests)
  EXPECT_EQ( lb::encoding::sha1::encode( "" ), "DA39A3EE5E6B4B0D3255BFEF95601890AFD80709" );
  EXPECT_EQ( lb::encoding::sha1::encode( "The quick brown fox jumps over the lazy dog" ), "2FD4E1C67A2D28FCED849EE1BB76E7391B93EB12" );
  EXPECT_EQ( lb::encoding::sha1::encode( "The quick brown fox jumps over the lazy cog" ), "DE9F2C7FD25E1B3AFAD3E85A0BD17D9B100DB4B3" );
}
