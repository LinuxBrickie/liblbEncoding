#ifndef LB_ENCODING_SHA1_H
#define LB_ENCODING_SHA1_H

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

#include <cstddef>
#include <string>


namespace lb
{


namespace encoding
{


namespace sha1
{


/**
    \brief Encodes \a numSrcChars bytes of data from \a src to s SHA1 digest in
           hexadecimal string form.
    \param src The bytes to encode (may contain nulls).
    \param numSrcChars The number of bytes to encode.
    \param dst The destination for the encoding. Assumes that sufficient contiguous
               bytes are available for access, see description for details.

    Endian agnostic in the sense that it operates on bytes. Obviously this does
    *not* mean that you get the same result for both little and big endian if
    you have, say, uint32_t data whose address you cast to char*.

    The data does not have to be ASCII, general binary data can be encoded too.

    The required size of \a dst is 40 since the SHA1 digest is a 20 byte number
    and each byte is represented by two hexadeciaml characters.

    Obviously this is a one-way encoding.

    \sa std::string encode( const std::string& )
 */
void encode( const char* src, size_t numSrcChars, char* dst );


/**
    \brief Encodes the data in \a src to s SHA1 digest in hexadecimal string
           form.
    \param src The bytes to encode in the form of a std::string (may contain nulls).
    \return dst The hexadecimal SHA1 string encooding of \a src.

    Endian agnostic in the sense that it operates on bytes. Obviously this does
    *not* mean that you get the same result for both little and big endian if
    you have, say, uint32_t data which you cast to char*.

    The data does not have to be ASCII, general binary data can be encoded too.

    The size of the returned string is 40.

    Obviously this is a one-way encoding.

    This is a std::string wrapper for the C_string version.

    \sa void encode( const char*, size_t, char* )
 */
std::string encode( const std::string& );


} // End of namespace sha1


} // End of namespace encoding


} // End of namespace lb


#endif // LB_ENCODING_SHA1_H
