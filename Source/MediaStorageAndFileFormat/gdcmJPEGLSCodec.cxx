/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006-2009 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "gdcmJPEGLSCodec.h"
#include "gdcmTransferSyntax.h"
#include "gdcmSequenceOfFragments.h"
#include "gdcmDataElement.h"

// CharLS includes
#include "gdcmcharls/stdafx.h" // sigh...
#include "gdcmcharls/interface.h"
#include "gdcmcharls/util.h"
#include "gdcmcharls/defaulttraits.h"
#include "gdcmcharls/losslesstraits.h"


namespace gdcm
{

JPEGLSCodec::JPEGLSCodec():BufferLength(0)
{
}

JPEGLSCodec::~JPEGLSCodec()
{
}

bool JPEGLSCodec::GetHeaderInfo(std::istream &is, TransferSyntax &ts)
{
#ifndef GDCM_USE_JPEGLS
  return false;
#else
  is.seekg( 0, std::ios::end);
  std::streampos buf_size = is.tellg();
  char *dummy_buffer = new char[buf_size];
  is.seekg(0, std::ios::beg);
  is.read( dummy_buffer, buf_size);

  JlsParamaters metadata;
  if (JpegLsReadHeader(dummy_buffer, buf_size, &metadata) != OK)
    {
    return false;
    }
  delete[] dummy_buffer;

  // $1 = {width = 512, height = 512, bitspersample = 8, components = 1, allowedlossyerror = 0, ilv = ILV_NONE, colorTransform = 0, custom = {MAXVAL = 0, T1 = 0, T2 = 0, T3 = 0, RESET = 0}}

  this->Dimensions[0] = metadata.width;
  this->Dimensions[1] = metadata.height;
  switch( metadata.bitspersample )
    {
  case 8:
    this->PF = PixelFormat( PixelFormat::UINT8 );
    break;
  case 12:
    this->PF = PixelFormat( PixelFormat::UINT16 );
    this->PF.SetBitsStored( 12 );
    break;
  case 16:
    this->PF = PixelFormat( PixelFormat::UINT16 );
    break;
  default:
    abort();
    }
  if( metadata.components == 1 )
    {
    PI = PhotometricInterpretation::MONOCHROME2;
    this->PF.SetSamplesPerPixel( 1 );
    }
  else if( metadata.components == 3 )
    {
    PI = PhotometricInterpretation::RGB;
    this->PF.SetSamplesPerPixel( 3 );
    }
  else abort();


  if( metadata.allowedlossyerror == 0 )
    {
    ts = TransferSyntax::JPEGLSLossless;
    }
  else
    {
    ts = TransferSyntax::JPEGLSNearLossless;
    }


  return true;
#endif
}

bool JPEGLSCodec::CanDecode(TransferSyntax const &ts) const
{
#ifndef GDCM_USE_JPEGLS
  return false;
#else
  return ts == TransferSyntax::JPEGLSLossless 
      || ts == TransferSyntax::JPEGLSNearLossless;
#endif
}

bool JPEGLSCodec::CanCode(TransferSyntax const &ts) const
{
#ifndef GDCM_USE_JPEGLS
  return false;
#else
  return ts == TransferSyntax::JPEGLSLossless 
      || ts == TransferSyntax::JPEGLSNearLossless;
#endif
}

bool JPEGLSCodec::Decode(DataElement const &in, DataElement &out)
{
#ifndef GDCM_USE_JPEGLS
  return false;
#else
  assert( NumberOfDimensions == 2 );

  const SequenceOfFragments *sf = in.GetSequenceOfFragments();
  assert( sf );
  std::stringstream is;
  unsigned long totalLen = sf->ComputeByteLength();
  char *buffer = new char[totalLen];
  sf->GetBuffer(buffer, totalLen);
  //is.write(buffer, totalLen);

  JlsParamaters metadata;
  if (JpegLsReadHeader(buffer, totalLen, &metadata) != OK)
    {
    return false;
    }

  // allowedlossyerror == 0 => Lossless
  LossyFlag = metadata.allowedlossyerror;

  const BYTE* pbyteCompressed = (const BYTE*)buffer;
  int cbyteCompressed = totalLen;

  JlsParamaters params = {0};
  JpegLsReadHeader(pbyteCompressed, cbyteCompressed, &params);

  std::vector<BYTE> rgbyteCompressed;
  rgbyteCompressed.resize(params.height *params.width* 4);

  std::vector<BYTE> rgbyteOut;
  rgbyteOut.resize(params.height *params.width * ((params.bitspersample + 7) / 8) * params.components);

  JLS_ERROR result = JpegLsDecode(&rgbyteOut[0], rgbyteOut.size(), pbyteCompressed, cbyteCompressed);
  ASSERT(result == OK);

  delete[] buffer;

  out = in;

  out.SetByteValue( (char*)&rgbyteOut[0], rgbyteOut.size() );
  return true;

#endif
}

// Compress into JPEG
bool JPEGLSCodec::Code(DataElement const &in, DataElement &out)
{
#ifndef GDCM_USE_JPEGLS
  return false;
#else
  out = in;
  //
  // Create a Sequence Of Fragments:
  SmartPointer<SequenceOfFragments> sq = new SequenceOfFragments;
  const Tag itemStart(0xfffe, 0xe000);
  //sq->GetTable().SetTag( itemStart );

  const unsigned int *dims = this->GetDimensions();
    int image_width = dims[0];
    int image_height = dims[1];
    const PixelFormat &pf = this->GetPixelFormat();
    int sample_pixel = pf.GetSamplesPerPixel();
    int bitsallocated = pf.GetBitsAllocated();
    int bitsstored = pf.GetBitsStored();

  const ByteValue *bv = in.GetByteValue();
  const char *input = bv->GetPointer();
  unsigned long len = bv->GetLength();
  unsigned long image_len = len / dims[2];
  size_t inputlength = image_len;

	JlsParamaters params = {0};
	params.components = sample_pixel;
	params.bitspersample = bitsstored;
	params.height = image_height;
	params.width = image_width;

	if (sample_pixel == 3)
	{
		params.ilv = ILV_LINE;
		params.colorTransform = 1;
	}

	std::vector<BYTE> rgbyteCompressed;
	rgbyteCompressed.resize(image_width * image_height * 4);

	size_t cbyteCompressed;
	JpegLsEncode(&rgbyteCompressed[0], rgbyteCompressed.size(), &cbyteCompressed, input, inputlength, &params);

    Fragment frag;
    frag.SetByteValue( (char*)&rgbyteCompressed[0], cbyteCompressed );
    sq->AddFragment( frag );

  assert( sq->GetNumberOfFragments() == dims[2] );
  out.SetValue( *sq );

  return true;

#endif
}

} // end namespace gdcm
