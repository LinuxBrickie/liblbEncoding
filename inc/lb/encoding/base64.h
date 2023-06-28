#ifndef LB_ENCODING_BASE64_H
#define LB_ENCODING_BASE64_H

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

#include <string>
#include <stdint.h>

#include <cstring>


namespace lb
{


namespace encoding
{


namespace base64
{


/**
    \brief Encodes \a numSrcChars bytes of data to base64.
    \param src The bytes to encode.
    \param numSrcChars The number of bytes to encode.
    \param dst The destination for the encoding. Assumes that sufficient contiguous
               bytes are available for access, see description doe details.

    Endian agnostic.

    The data does not have to be ASCII, general binary data can be encoded too.

    The required minimum size of \a dst is 4 * (numSrcChars/3 + 1) where the
    division by 3 is assumed to round down.

    Depending on the value of \a numSrcChars the final one or two destination
    bytes may be padding bytes ('='). Only when \a numSrcChars is a multiple of
    3 will there be no padding bytes. The encoded data is always a multiple of
    4 bytes.
 */
void encode( const char* src, size_t numSrcChars, char* dst );


} // End of namespace base64


} // End of namespace encoding


} // End of namespace lb


#endif // LB_ENCODING_BASE64_H
