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

// BitmapDimensions is where the block geometry actually gets used, and where the 4x4 assumption
// was written three more times -- as `/ 4u` in GetMipPitch, as `/ 16` in GetMipSize, and as `/ 4`
// in GetMipNumRows. Two bugs shared those expressions: the footprint, and the truncation.

TEST( BitmapDimensionsBlockGeometry, ACompressedMipSmallerThanItsBlockIsStillOneBlock )
{
	// This is the truncation half, and it was wrong for BC too -- not just for ASTC. Every mip
	// chain ends at 1x1, where `width / 4` is 0: the old GetMipPitch returned a pitch of zero,
	// and GetMipSize computed 2*2*1/16*8 = 0 for the 2x2 level as well.
	const BitmapDimensions bc1( TEX_TYPE_2D, PIXEL_FORMAT_BC1_UNORM, 16, 16, 1, 5 );

	EXPECT_EQ( 8u, bc1.GetMipPitch( 4 ) ) << "1x1 mip is one 8-byte BC1 block";
	EXPECT_EQ( 1u, bc1.GetMipNumRows( 4 ) );
	EXPECT_EQ( 8u, bc1.GetMipSize( 4 ) );
	EXPECT_EQ( 8u, bc1.GetMipSize( 3 ) ) << "2x2 mip is one block, not zero bytes";
}

TEST( BitmapDimensionsBlockGeometry, AFourByFourFormatIsUnchanged )
{
	// The regression guard: whatever ASTC needed must not have moved BC. 16x16 BC1 is 4x4 blocks
	// of 8 bytes; 12x8 BC7 is 3x2 blocks of 16.
	const BitmapDimensions bc1( TEX_TYPE_2D, PIXEL_FORMAT_BC1_UNORM, 16, 16, 1, 5 );
	EXPECT_EQ( 4u * 8u, bc1.GetMipPitch( 0 ) );
	EXPECT_EQ( 4u, bc1.GetMipNumRows( 0 ) );
	EXPECT_EQ( 4u * 4u * 8u, bc1.GetMipSize( 0 ) );

	const BitmapDimensions bc7( TEX_TYPE_2D, PIXEL_FORMAT_BC7_UNORM, 12, 8, 1, 1 );
	EXPECT_EQ( 3u * 16u, bc7.GetMipPitch( 0 ) );
	EXPECT_EQ( 2u, bc7.GetMipNumRows( 0 ) );
	EXPECT_EQ( 3u * 2u * 16u, bc7.GetMipSize( 0 ) );
}

TEST( BitmapDimensionsBlockGeometry, AstcSixBySixIsTheCaseTheOldFormCouldNotExpress )
{
	// 48 texels of 6x6 is 8 blocks. `48 / 4` said 12, so the pitch was half again too large and
	// every row after the first read from the wrong offset.
	const BitmapDimensions astc( TEX_TYPE_2D, PIXEL_FORMAT_ASTC_6x6_UNORM, 48, 36, 1, 1 );
	EXPECT_EQ( 8u * 16u, astc.GetMipPitch( 0 ) );
	EXPECT_EQ( 6u, astc.GetMipNumRows( 0 ) );
	EXPECT_EQ( 8u * 6u * 16u, astc.GetMipSize( 0 ) );

	// And a size that is not a multiple of 6 rounds up on both axes.
	const BitmapDimensions odd( TEX_TYPE_2D, PIXEL_FORMAT_ASTC_6x6_UNORM, 50, 50, 1, 1 );
	EXPECT_EQ( 9u * 16u, odd.GetMipPitch( 0 ) );
	EXPECT_EQ( 9u, odd.GetMipNumRows( 0 ) );
}

TEST( BitmapDimensionsBlockGeometry, AstcEightByEight )
{
	const BitmapDimensions astc( TEX_TYPE_2D, PIXEL_FORMAT_ASTC_8x8_UNORM, 64, 64, 1, 1 );
	EXPECT_EQ( 8u * 16u, astc.GetMipPitch( 0 ) );
	EXPECT_EQ( 8u, astc.GetMipNumRows( 0 ) );
	EXPECT_EQ( 8u * 8u * 16u, astc.GetMipSize( 0 ) );
}

TEST( BitmapDimensionsBlockGeometry, AnUncompressedFormatStillCountsTexelRows )
{
	// GetMipNumRows lost its branch -- GetBlockHeight is 1 for uncompressed, so the block form is
	// an identity there. This is the assertion that says so rather than trusting it.
	const BitmapDimensions rgba( TEX_TYPE_2D, PIXEL_FORMAT_B8G8R8A8_UNORM, 5, 12, 1, 1 );
	EXPECT_EQ( 5u * 4u, rgba.GetMipPitch( 0 ) );
	EXPECT_EQ( 12u, rgba.GetMipNumRows( 0 ) );
	EXPECT_EQ( 5u * 12u * 4u, rgba.GetMipSize( 0 ) );
}
