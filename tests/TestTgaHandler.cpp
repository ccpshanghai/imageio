// Copyright © 2014 CCP ehf.

#include "StdAfx.h"
#include "TestHelpers.h"
#include "MemoryStream.h"

#include <vector>

using namespace ImageIO;

namespace
{

const TestImage s_rgb = { 
	TEST_FILE( rgb, tga ),
	BitmapDimensions( TEX_TYPE_2D, PIXEL_FORMAT_B8G8R8X8_UNORM, 5, 8, 1, 1 ), 
	&CheckTopLeftPixel<uint32_t, 0xffff2aff>, &CheckBottomRightPixel<uint32_t, 0xffff00b9>, nullptr };
const TestImage s_rgbRle = { 
	TEST_FILE( rgbRle, tga ),
	BitmapDimensions( TEX_TYPE_2D, PIXEL_FORMAT_B8G8R8X8_UNORM, 5, 8, 1, 1 ), 
	&CheckTopLeftPixel<uint32_t, 0xffff2aff>, &CheckBottomRightPixel<uint32_t, 0xffff00b9>, nullptr };
const TestImage s_rgba = { 
	TEST_FILE( rgba, tga ),
	BitmapDimensions( TEX_TYPE_2D, PIXEL_FORMAT_B8G8R8A8_UNORM, 5, 8, 1, 1 ), 
	&CheckTopLeftPixel<uint32_t, 0xffff2aff>, &CheckBottomRightPixel<uint32_t, 0x61ff00b9>, nullptr };
const TestImage s_rgbaRle = { 
	TEST_FILE( rgbaRle, tga ),
	BitmapDimensions( TEX_TYPE_2D, PIXEL_FORMAT_B8G8R8A8_UNORM, 5, 8, 1, 1 ), 
	&CheckTopLeftPixel<uint32_t, 0xffff2aff>, &CheckBottomRightPixel<uint32_t, 0x61ff00b9>, nullptr };
const TestImage s_r = { 
	TEST_FILE( r, tga ),
	BitmapDimensions( TEX_TYPE_2D, PIXEL_FORMAT_R8_UNORM, 5, 8, 1, 1 ), 
	&CheckTopLeftPixel<uint8_t, 0xff>, &CheckBottomRightPixel<uint8_t, 0x61>, nullptr };
const TestImage s_rgbIndexed = { 
	TEST_FILE( rgbIndexed, tga ),
	BitmapDimensions( TEX_TYPE_2D, PIXEL_FORMAT_B8G8R8X8_UNORM, 5, 8, 1, 1 ), 
	&CheckTopLeftPixel<uint32_t, 0xffff2aff>, &CheckBottomRightPixel<uint32_t, 0xffff00b9>, nullptr };
const TestImage s_iDontExist = { L"resources/iDontExist.tga", nullptr, 0 };

}

TEST( TgaHandler, CanLoadRgbTga )
{
	SCOPED_TRACE( __FUNCTION__ );
	TestReadImage( s_rgb );
}

TEST( TgaHandler, CanLoadRgbTgaWithRleCompression )
{
	SCOPED_TRACE( __FUNCTION__ );
	TestReadImage( s_rgbRle );
}

TEST( TgaHandler, CanLoadRgbaTga )
{
	SCOPED_TRACE( __FUNCTION__ );
	TestReadImage( s_rgba );
}

TEST( TgaHandler, CanLoadRgbaTgaWithRleCompression )
{
	SCOPED_TRACE( __FUNCTION__ );
	TestReadImage( s_rgbaRle );
}

TEST( TgaHandler, CanLoadGrayscaleTga )
{
	SCOPED_TRACE( __FUNCTION__ );
	TestReadImage( s_r );
}

TEST( TgaHandler, CanLoadRgbTgaWithWithPalette )
{
	SCOPED_TRACE( __FUNCTION__ );
	TestReadImage( s_rgbIndexed );
}

TEST( TgaHandler, LoadingRgbTgaWithTrimmedHeaderFails )
{
	SCOPED_TRACE( __FUNCTION__ );
	AssertReadTruncatedImageFails( s_rgb, 12 );
}

TEST( TgaHandler, LoadingRgbTgaWithTrimmedDataFails )
{
	SCOPED_TRACE( __FUNCTION__ );
	AssertReadTruncatedImageFails( s_rgb, 100 );
}

TEST( TgaHandler, LoadingRgbaRleTgaWithTrimmedHeaderFails )
{
	SCOPED_TRACE( __FUNCTION__ );
	AssertReadTruncatedImageFails( s_rgbaRle, 12 );
}

TEST( TgaHandler, LoadingRgbaRleTgaWithTrimmedDataFails )
{
	SCOPED_TRACE( __FUNCTION__ );
	AssertReadTruncatedImageFails( s_rgbaRle, 100 );
}

TEST( TgaHandler, LoadingNonExistentTgaFails )
{
	SCOPED_TRACE( __FUNCTION__ );
	AssertReadImageFails( s_iDontExist );
}

TEST( TgaHandler, CanSaveRgbTga )
{
	SCOPED_TRACE( __FUNCTION__ );
	AssertCanSaveImage( s_rgb );
}

TEST( TgaHandler, CanSaveRgbaTga )
{
	SCOPED_TRACE( __FUNCTION__ );
	AssertCanSaveImage( s_rgba );
}

TEST( TgaHandler, CanSaveGrayscaleTga )
{
	SCOPED_TRACE( __FUNCTION__ );
	AssertCanSaveImage( s_r );
}

// Hand-built TGA files for the RLE decoder's edges and the right-to-left flip. The decoder checks
// every packet against the end of the data and of the bitmap before touching either; these files
// are the cases the old decoder read or wrote past (run under AddressSanitizer to see it) or, for
// the flip, never returned from.

namespace
{

enum : uint8_t
{
	RAW_RGB = 2,
	RLE_INDEXED = 9,
	RLE_RGB = 10,
	RLE_GREYSCALE = 11,
	RIGHT_TO_LEFT = 0x10,
};

std::vector<uint8_t> MakeTga( uint8_t imageType, uint8_t bpp, uint16_t width, uint16_t height,
	std::initializer_list<uint8_t> data, uint8_t descriptor = 0 )
{
	const bool indexed = imageType == RLE_INDEXED;
	std::vector<uint8_t> file = {
		0,                                      // idLength
		uint8_t( indexed ? 1 : 0 ),             // colorMapType
		imageType,
		0, 0,                                   // colorMapStart
		0, uint8_t( indexed ? 1 : 0 ),          // colorMapLength: 256 (little-endian)
		uint8_t( indexed ? 24 : 0 ),            // colorMapBpp
		0, 0, 0, 0,                             // left, top
		uint8_t( width ), uint8_t( width >> 8 ), uint8_t( height ), uint8_t( height >> 8 ), // right, bottom
		bpp,
		descriptor,
	};
	if( indexed )
	{
		// Entry i is (i, 255 - i, 7).
		for( unsigned i = 0; i != 256; ++i )
		{
			file.push_back( uint8_t( i ) );
			file.push_back( uint8_t( 255 - i ) );
			file.push_back( 7 );
		}
	}
	file.insert( file.end(), data );
	return file;
}

Result LoadTga( const std::vector<uint8_t>& file, HostBitmap& bitmap )
{
	// Exactly the file's bytes on the heap, so AddressSanitizer sees a read past the end.
	std::unique_ptr<uint8_t[]> copy( new uint8_t[file.size()] );
	memcpy( copy.get(), file.data(), file.size() );
	ReadMemoryStream stream( copy.get(), file.size() );
	return ImageIO::Tga::ReadImage( stream, LoadParameters( L"hand-built.tga" ), bitmap, nullptr );
}

}

TEST( TgaHandler, RleGreyscaleFillsEveryPixelAndNoMore )
{
	// 4x2, one byte a pixel: a run of 5, then 3 raw. The old decoder wrote byte [3] of every
	// 1-byte pixel, 3 bytes past the bitmap at the last one.
	HostBitmap bitmap;
	const Result result = LoadTga( MakeTga( RLE_GREYSCALE, 8, 4, 2, { 0x84, 9, 0x02, 1, 2, 3 } ), bitmap );
	ASSERT_TRUE( result ) << result.GetErrorMessage();
	ASSERT_EQ( PIXEL_FORMAT_R8_UNORM, bitmap.GetFormat() );
	// Stored bottom-up: the first row in the file is the bottom row.
	const uint8_t expected[] = { 9, 1, 2, 3, 9, 9, 9, 9 };
	EXPECT_EQ( 0, memcmp( expected, bitmap.GetRawData(), sizeof( expected ) ) );
}

TEST( TgaHandler, RleRgbExpandsToFourBytesAPixel )
{
	// 2x1, 24 bits: one raw packet of 2. The old decoder copied 4 bytes from each 3-byte pixel.
	HostBitmap bitmap;
	const Result result = LoadTga( MakeTga( RLE_RGB, 24, 2, 1, { 0x01, 10, 20, 30, 40, 50, 60 } ), bitmap );
	ASSERT_TRUE( result ) << result.GetErrorMessage();
	const uint8_t expected[] = { 10, 20, 30, 255, 40, 50, 60, 255 };
	EXPECT_EQ( 0, memcmp( expected, bitmap.GetRawData(), sizeof( expected ) ) );
}

TEST( TgaHandler, RleIndexedStepsOneByteAnIndex )
{
	// 3x1, 8-bit indexes 1, 2, 2 (raw 1, run 2). The old decoder stepped 3 bytes an index.
	HostBitmap bitmap;
	const Result result = LoadTga( MakeTga( RLE_INDEXED, 8, 3, 1, { 0x00, 1, 0x81, 2 } ), bitmap );
	ASSERT_TRUE( result ) << result.GetErrorMessage();
	const uint8_t expected[] = { 1, 254, 7, 255, 2, 253, 7, 255, 2, 253, 7, 255 };
	EXPECT_EQ( 0, memcmp( expected, bitmap.GetRawData(), sizeof( expected ) ) );
}

TEST( TgaHandler, RleRunPastTheBitmapFails )
{
	// 2x2 greyscale, a run of 128: the old decoder wrote 124 bytes past the bitmap.
	HostBitmap bitmap;
	EXPECT_EQ( Result::INVALID_DATA, LoadTga( MakeTga( RLE_GREYSCALE, 8, 2, 2, { 0xff, 1 } ), bitmap ).code );
	EXPECT_FALSE( bitmap.IsValid() );
}

TEST( TgaHandler, RleRawPacketPastTheBitmapFails )
{
	HostBitmap bitmap;
	EXPECT_EQ( Result::INVALID_DATA, LoadTga( MakeTga( RLE_RGB, 24, 1, 1, { 0x01, 1, 2, 3, 4, 5, 6 } ), bitmap ).code );
}

TEST( TgaHandler, RleTrimmedPacketFails )
{
	// Each file ends inside a packet: no header left, a run without its pixel, a raw packet
	// short of a pixel, a pixel short of a byte.
	for( const auto& data : std::vector<std::vector<uint8_t>>{ { 0x80, 1, 2, 3 }, { 0x81 }, { 0x01, 1, 2, 3, 4, 5 }, { 0x80, 1, 2 } } )
	{
		std::vector<uint8_t> file = MakeTga( RLE_RGB, 24, 2, 1, {} );
		file.insert( file.end(), data.begin(), data.end() );
		HostBitmap bitmap;
		EXPECT_EQ( Result::INVALID_DATA, LoadTga( file, bitmap ).code ) << data.size() << " bytes of data";
	}
}

TEST( TgaHandler, RleDepthItCannotExpandIsRefused )
{
	// 16-bit pixels used to be decoded as 4 bytes each; an indexed image with 16-bit indexes
	// stepped by the palette's 3 bytes.
	HostBitmap bitmap;
	EXPECT_EQ( Result::HEADER_NOT_SUPPORTED, LoadTga( MakeTga( RLE_RGB, 16, 1, 1, { 0x80, 1, 2 } ), bitmap ).code );
	EXPECT_EQ( Result::HEADER_NOT_SUPPORTED, LoadTga( MakeTga( RLE_INDEXED, 16, 1, 1, { 0x80, 1, 0 } ), bitmap ).code );
}

TEST( TgaHandler, RightToLeftImageIsMirrored )
{
	// 3x1 raw, right-to-left. The flip loop never advanced: this test used to hang.
	HostBitmap bitmap;
	const Result result = LoadTga( MakeTga( RAW_RGB, 24, 3, 1, { 1, 1, 1, 2, 2, 2, 3, 3, 3 }, RIGHT_TO_LEFT ), bitmap );
	ASSERT_TRUE( result ) << result.GetErrorMessage();
	const uint8_t expected[] = { 3, 3, 3, 255, 2, 2, 2, 255, 1, 1, 1, 255 };
	EXPECT_EQ( 0, memcmp( expected, bitmap.GetRawData(), sizeof( expected ) ) );
}
