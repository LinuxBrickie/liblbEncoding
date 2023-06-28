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

#include <lb/encoding/base64.h>


TEST(Encoding, Base64)
{
  char encoded[128];
  encoded[4] = '\0';

  lb::encoding::base64::encode( "Man", 3, encoded );
  EXPECT_EQ( std::string( encoded ), "TWFu" );

  lb::encoding::base64::encode( "Ma" , 2, encoded );
  EXPECT_EQ( std::string( encoded ), "TWE=" );

  lb::encoding::base64::encode( "M"  , 1, encoded );
  EXPECT_EQ( std::string( encoded ), "TQ==" );

  encoded[36] = '\0';
  lb::encoding::base64::encode( "Many hands make light work.", 27, encoded );
  EXPECT_EQ( std::string( encoded ), "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu" );

  encoded[28] = '\0';
  lb::encoding::base64::encode( "\x2f\xd4\xe1\xc6\x7a\x2d\x28\xfc\xed\x84\x9e\xe1\xbb\x76\xe7\x39\x1b\x93\xeb\x12", 20, encoded );
  EXPECT_EQ( std::string( encoded ), "L9ThxnotKPzthJ7hu3bnORuT6xI=" );
}
