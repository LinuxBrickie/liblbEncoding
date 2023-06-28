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

#include <lb/encoding/bits.h>

#include <iostream>


namespace lb
{


namespace encoding
{


namespace bits
{


void encode( char src, char* dst )
{
  *dst++ = ( src & 0x80 ) ? '1' : '0';
  *dst++ = ( src & 0x40 ) ? '1' : '0';
  *dst++ = ( src & 0x20 ) ? '1' : '0';
  *dst++ = ( src & 0x10 ) ? '1' : '0';
  *dst++ = ( src & 0x08 ) ? '1' : '0';
  *dst++ = ( src & 0x04 ) ? '1' : '0';
  *dst++ = ( src & 0x02 ) ? '1' : '0';
  *dst   = ( src & 0x01 ) ? '1' : '0';
}

Printer::Printer( char c )
{
  encode( c, bits );
  bits[8] = '\0';
}

void Printer::operator()( std::ostream& os ) const
{
  os << bits;
}

std::ostream& operator<<( std::ostream& os, const Printer& printer )
{
  printer( os );
  return os;
}


} // End of namespace bits


} // End of namespace encoding


} // End of namespace lb
