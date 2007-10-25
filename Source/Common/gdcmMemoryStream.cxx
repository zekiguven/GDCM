/*=========================================================================

  Program: GDCM (Grass Root DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006-2007 Mathieu Malaterre
  Copyright (c) 1993-2005 CREATIS
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "gdcmMemoryStream.h"

namespace gdcm
{
IStream& MemoryStream::Read(char* s, std::streamsize n)
{
  strncpy(s, Pointer+GPos, n);
  return *this;
}

//OStream& MemoryStream::WriteBuffer(const char* s, std::streamsize n)
//{
//  Pointer = s;
//  Length = n;
//  return *this;
//}

std::streamsize MemoryStream::Gcount ( ) const
{
  return Length;
}

IStream& MemoryStream::Seekg (std::streamoff off, std::ios::seekdir dir)
{
  switch(dir)
    {
  case std::ios::beg:
    GPos = off;
    break;
  case std::ios::cur:
    GPos = GPos + off;
    break;
  case std::ios::end:
    GPos = Length - off;
    break;
  default:
    abort();
    }
  return *this;
}

std::streampos MemoryStream::Tellg ( )
{
  return GPos;
}

} // end namespace gdcm
