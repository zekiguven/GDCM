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
#include "gdcmItem.h"

#include <assert.h>

namespace gdcm
{

VL Item::GetLength() const
{
  if( ValueLengthField.IsUndefined() )
    {
    assert( !NestedDataSet.GetLength().IsUndefined() );
    // Item Start             4
    // Item Length            4
    // DataSet                ?
    // Item End Delimitation  4
    // Item End Length        4
    return TagField.GetLength() /* 4 */ + ValueLengthField.GetLength() /* 4 */
      + NestedDataSet.GetLength() + 4 + 4;
    }
  else
    {
    // Item Start             4
    // Item Length            4
    // DataSet                ?
    return TagField.GetLength() /* 4 */ + ValueLengthField.GetLength() /* 4 */
      + ValueLengthField;
    }
}

} // end namespace gdcm

