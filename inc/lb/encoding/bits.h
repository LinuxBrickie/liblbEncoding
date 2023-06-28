#ifndef LB_ENCODING_BITS_H
#define LB_ENCODING_BITS_H

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

#include <iosfwd>


namespace lb
{


namespace encoding
{


namespace bits
{


/**
    \brief Encode a byte value into an 8 character string of ones and zeros.
    \param src The byte to encode.
    \param buffer The destination for the encoding. Assumes that 2 contiguous
                  bytes are available for access.

    Endian agnostic.

    A bare bones alternative to using a C++ stream with the std::hex manipulator.
 */
void encode( char src, char* dst );


/**
    \brief Manipulator for printing a byte's bits to an output stream.

    A useful debugging aid.
 */
struct Printer
{
  char bits[9];

  Printer( char c );// : c{ c } {}

  void operator()( std::ostream& os ) const;
};


/** \brief An output stream operator to print the bits of a char.

    Example usage:

      std::cout << lb::enconding::bits::Printer( 'a' ) << std::endl;

    Note that argument dependent lookup will find the operator in this
    lb::encoding::bits namespace.
 */
std::ostream& operator<<( std::ostream& os, const Printer& printer );


} // End of namespace bits


} // End of namespace encoding


} // End of namespace lb


#endif // LB_ENCODING_BITS_H
