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
#include "gdcm.h"

int TestReadPatientName(int argc, char *argv[])
{
  (void)argc;
  gdcm::Parser p;
  std::string filename = argv[1];
  p.SetFileName( filename );
  gdcm::Tag t(0x0010, 0x0010);
  const char *v = p.Request( t );
  gdcm::Attributes<gdcm::VR::PN,gdcm::VM::VM1> pn( v );

  std::cout << pn << std::endl;

  return 0;
}
