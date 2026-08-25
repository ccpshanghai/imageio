// Copyright © 2026 CCP ehf.

#include "StdAfx.h"

using namespace ImageIO;

// Block geometry, which stopped being a constant when ASTC arrived.
//
// Every BC format is 4x4 texels, so for fourteen formats "blocks per row" was `width / 4` and
// fourteen call sites across trinity wrote that 4 in by hand. ASTC 4x4 keeps it true; 6x6 and 8x8
// do not. These assertions are the ones that would have caught it: not "does the enum have ASTC
// rows" -- it did -- but "does anything know how wide an ASTC block is".

TEST( PixelFormat, EveryBcBlockIsFourByFour )
{
	// Both byte sizes are represented: BC1/BC4 are 8-byte blocks, the rest 16, and the footprint
	// is 4x4 regardless. A test that only checked BC1 would pass on a table keyed by byte size.
	for( PixelFormat format : { PIXEL_FORMAT_BC1_UNORM, PIXEL_FORMAT_BC2_UNORM,
			 PIXEL_FORMAT_BC3_UNORM, PIXEL_FORMAT_BC4_UNORM, PIXEL_FORMAT_BC5_SNORM,
			 PIXEL_FORMAT_BC6H_UF16, PIXEL_FORMAT_BC7_UNORM_SRGB } )
	{
		EXPECT_EQ( 4u, GetBlockWidth( format ) ) << "format " << int( format );
		EXPECT_EQ( 4u, GetBlockHeight( format ) ) << "format " << int( format );
	}
}

TEST( PixelFormat, AstcBlockFootprintIsTheOneInItsName )
{
	EXPECT_EQ( 4u, GetBlockWidth( PIXEL_FORMAT_ASTC_4x4_UNORM ) );
	EXPECT_EQ( 4u, GetBlockHeight( PIXEL_FORMAT_ASTC_4x4_UNORM_SRGB ) );
	EXPECT_EQ( 6u, GetBlockWidth( PIXEL_FORMAT_ASTC_6x6_UNORM ) );
	EXPECT_EQ( 6u, GetBlockHeight( PIXEL_FORMAT_ASTC_6x6_UNORM_SRGB ) );
	EXPECT_EQ( 8u, GetBlockWidth( PIXEL_FORMAT_ASTC_8x8_UNORM ) );
	EXPECT_EQ( 8u, GetBlockHeight( PIXEL_FORMAT_ASTC_8x8_UNORM_SRGB ) );
}

TEST( PixelFormat, AnAstcBlockIsSixteenBytesAtEveryFootprint )
{
	// The reason the two functions are separate: the byte size does NOT vary with the footprint,
	// so a caller cannot infer one from the other. 6x6 stores 36 texels in the same 128 bits that
	// 4x4 uses for 16 -- that is the whole point of choosing a larger block.
	for( PixelFormat format : { PIXEL_FORMAT_ASTC_4x4_UNORM, PIXEL_FORMAT_ASTC_6x6_UNORM,
			 PIXEL_FORMAT_ASTC_8x8_UNORM } )
	{
		EXPECT_EQ( 16u, GetBlockByteSize( format ) ) << "format " << int( format );
	}
}

TEST( PixelFormat, AnUncompressedFormatHasABlockExtentOfOne )
{
	// 1 rather than GetBlockByteSize's 0, so that
	//     GetBlockCount( width, GetBlockWidth( f ) ) * bytesPerUnit
	// is the pitch on any format with no branch, and so that nothing divides by zero.
	EXPECT_EQ( 1u, GetBlockWidth( PIXEL_FORMAT_B8G8R8A8_UNORM ) );
	EXPECT_EQ( 1u, GetBlockHeight( PIXEL_FORMAT_R32G32B32A32_FLOAT ) );
	EXPECT_EQ( 1u, GetBlockWidth( PIXEL_FORMAT_UNKNOWN ) );
	EXPECT_EQ( 48u, GetBlockCount( 48, GetBlockWidth( PIXEL_FORMAT_B8G8R8A8_UNORM ) ) );
}

TEST( PixelFormat, BlockCountIsTheArithmeticThatWasWrong )
{
	// 48 texels of 6x6 is 8 blocks. The code this replaces said `48 / 4` and got 12 -- a row
	// pitch half again too large, which reads as garbled texels rather than as a failure.
	EXPECT_EQ( 8u, GetBlockCount( 48, GetBlockWidth( PIXEL_FORMAT_ASTC_6x6_UNORM ) ) );
	EXPECT_EQ( 12u, GetBlockCount( 48, GetBlockWidth( PIXEL_FORMAT_BC1_UNORM ) ) );
	EXPECT_EQ( 6u, GetBlockCount( 48, GetBlockWidth( PIXEL_FORMAT_ASTC_8x8_UNORM ) ) );
}

TEST( PixelFormat, APartialBlockIsStillAWholeBlock )
{
	// Rounding up is not a nicety: an image is only a whole number of blocks when its size is a
	// multiple of the footprint, and a 6x6 mip chain stops being one almost immediately.
	EXPECT_EQ( 9u, GetBlockCount( 50, 6 ) );
	EXPECT_EQ( 1u, GetBlockCount( 1, 8 ) );
	EXPECT_EQ( 1u, GetBlockCount( 6, 6 ) );
	EXPECT_EQ( 2u, GetBlockCount( 7, 6 ) );

	// An empty axis has no blocks, and a zero extent must not divide.
	EXPECT_EQ( 0u, GetBlockCount( 0, 6 ) );
	EXPECT_EQ( 0u, GetBlockCount( 48, 0 ) );
}

TEST( PixelFormat, EveryCompressedFormatHasABlockExtentAboveOne )
{
	// The invariant that keeps the two tables from drifting: if IsCompressedFormat says yes, the
	// footprint must be a real block, and the byte size must be non-zero. A format added to the
	// enum and to IsCompressedFormat but not to GetBlockWidth fails here rather than in a pitch
	// calculation three repositories away.
	for( int i = 0; i < PIXEL_FORMAT_SENTINEL; ++i )
	{
		const PixelFormat format = PixelFormat( i );
		if( !IsCompressedFormat( format ) )
		{
			continue;
		}
		EXPECT_GT( GetBlockWidth( format ), 1u ) << "format " << i;
		EXPECT_GT( GetBlockHeight( format ), 1u ) << "format " << i;
		EXPECT_NE( 0u, GetBlockByteSize( format ) ) << "format " << i;
	}
}
