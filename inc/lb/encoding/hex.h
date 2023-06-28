#ifndef LB_ENCODING_HEX_H
#define LB_ENCODING_HEX_H

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


namespace hex
{


/**
    \brief Encode a byte value into a pair of hexadecimal characters.
    \param src The byte to encode (may contain nulls).
    \param buffer The destination for the conversion. Assumes that 2 contiguous
                  bytes are available for access.

    Endian agnostic.

    A bare bones alternative to using a C++ stream with the std::hex manipulator.
 */
void encode( char src, char* dst );


/**
    \brief Encodes a string into pairs of hexadecimal characters.
    \param src The byte to encode (may contain nulls).
    \param buffer The destination for the conversion. Assumes that 2 contiguous
                  bytes are available for access.

    Endian agnostic.

    A bare bones alternative to using a C++ stream with the std::hex manipulator.
 */
void encode( const char* src, size_t numSrcBytes, char* dst );


/**
    \brief Encodes a string into pairs of hexadecimal characters.
    \param src The byte to encode in the form of a std::string (may contain nulls).
    \return The encoded hexadecimal characters.

    Endian agnostic.

    A bare bones alternative to using a C++ stream with the std::hex manipulator.

    This is a std::string wrapper for the C_string version. Beware this does
    have a C-string copy overhead when creating the return value.
 */
std::string encode( const std::string& src );


} // End of namespace hex


} // End of namespace encoding


} // End of namespace lb


#endif // LB_ENCODING_HEX_H
