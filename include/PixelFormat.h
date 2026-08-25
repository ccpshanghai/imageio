// Copyright © 2024 CCP ehf.

#pragma once

#include <cstdint>

namespace ImageIO
{


enum PixelFormat
{
	PIXEL_FORMAT_UNKNOWN = 0,
	PIXEL_FORMAT_R32G32B32A32_TYPELESS = 1,
	PIXEL_FORMAT_R32G32B32A32_FLOAT = 2,
	PIXEL_FORMAT_R32G32B32A32_UINT = 3,
	PIXEL_FORMAT_R32G32B32A32_SINT = 4,
	PIXEL_FORMAT_R32G32B32_TYPELESS = 5,
	PIXEL_FORMAT_R32G32B32_FLOAT = 6,
	PIXEL_FORMAT_R32G32B32_UINT = 7,
	PIXEL_FORMAT_R32G32B32_SINT = 8,
	PIXEL_FORMAT_R16G16B16A16_TYPELESS = 9,
	PIXEL_FORMAT_R16G16B16A16_FLOAT = 10,
	PIXEL_FORMAT_R16G16B16A16_UNORM = 11,
	PIXEL_FORMAT_R16G16B16A16_UINT = 12,
	PIXEL_FORMAT_R16G16B16A16_SNORM = 13,
	PIXEL_FORMAT_R16G16B16A16_SINT = 14,
	PIXEL_FORMAT_R32G32_TYPELESS = 15,
	PIXEL_FORMAT_R32G32_FLOAT = 16,
	PIXEL_FORMAT_R32G32_UINT = 17,
	PIXEL_FORMAT_R32G32_SINT = 18,
	PIXEL_FORMAT_R32G8X24_TYPELESS = 19,
	PIXEL_FORMAT_D32_FLOAT_S8X24_UINT = 20,
	PIXEL_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
	PIXEL_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
	PIXEL_FORMAT_R10G10B10A2_TYPELESS = 23,
	PIXEL_FORMAT_R10G10B10A2_UNORM = 24,
	PIXEL_FORMAT_R10G10B10A2_UINT = 25,
	PIXEL_FORMAT_R11G11B10_FLOAT = 26,
	PIXEL_FORMAT_R8G8B8A8_TYPELESS = 27,
	PIXEL_FORMAT_R8G8B8A8_UNORM = 28,
	PIXEL_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
	PIXEL_FORMAT_R8G8B8A8_UINT = 30,
	PIXEL_FORMAT_R8G8B8A8_SNORM = 31,
	PIXEL_FORMAT_R8G8B8A8_SINT = 32,
	PIXEL_FORMAT_R16G16_TYPELESS = 33,
	PIXEL_FORMAT_R16G16_FLOAT = 34,
	PIXEL_FORMAT_R16G16_UNORM = 35,
	PIXEL_FORMAT_R16G16_UINT = 36,
	PIXEL_FORMAT_R16G16_SNORM = 37,
	PIXEL_FORMAT_R16G16_SINT = 38,
	PIXEL_FORMAT_R32_TYPELESS = 39,
	PIXEL_FORMAT_D32_FLOAT = 40,
	PIXEL_FORMAT_R32_FLOAT = 41,
	PIXEL_FORMAT_R32_UINT = 42,
	PIXEL_FORMAT_R32_SINT = 43,
	PIXEL_FORMAT_R24G8_TYPELESS = 44,
	PIXEL_FORMAT_D24_UNORM_S8_UINT = 45,
	PIXEL_FORMAT_R24_UNORM_X8_TYPELESS = 46,
	PIXEL_FORMAT_X24_TYPELESS_G8_UINT = 47,
	PIXEL_FORMAT_R8G8_TYPELESS = 48,
	PIXEL_FORMAT_R8G8_UNORM = 49,
	PIXEL_FORMAT_R8G8_UINT = 50,
	PIXEL_FORMAT_R8G8_SNORM = 51,
	PIXEL_FORMAT_R8G8_SINT = 52,
	PIXEL_FORMAT_R16_TYPELESS = 53,
	PIXEL_FORMAT_R16_FLOAT = 54,
	PIXEL_FORMAT_D16_UNORM = 55,
	PIXEL_FORMAT_R16_UNORM = 56,
	PIXEL_FORMAT_R16_UINT = 57,
	PIXEL_FORMAT_R16_SNORM = 58,
	PIXEL_FORMAT_R16_SINT = 59,
	PIXEL_FORMAT_R8_TYPELESS = 60,
	PIXEL_FORMAT_R8_UNORM = 61,
	PIXEL_FORMAT_R8_UINT = 62,
	PIXEL_FORMAT_R8_SNORM = 63,
	PIXEL_FORMAT_R8_SINT = 64,
	PIXEL_FORMAT_A8_UNORM = 65,
	PIXEL_FORMAT_R1_UNORM = 66,
	PIXEL_FORMAT_R9G9B9E5_SHAREDEXP = 67,
	PIXEL_FORMAT_R8G8_B8G8_UNORM = 68,
	PIXEL_FORMAT_G8R8_G8B8_UNORM = 69,
	PIXEL_FORMAT_BC1_TYPELESS = 70,
	PIXEL_FORMAT_BC1_UNORM = 71,
	PIXEL_FORMAT_BC1_UNORM_SRGB = 72,
	PIXEL_FORMAT_BC2_TYPELESS = 73,
	PIXEL_FORMAT_BC2_UNORM = 74,
	PIXEL_FORMAT_BC2_UNORM_SRGB = 75,
	PIXEL_FORMAT_BC3_TYPELESS = 76,
	PIXEL_FORMAT_BC3_UNORM = 77,
	PIXEL_FORMAT_BC3_UNORM_SRGB = 78,
	PIXEL_FORMAT_BC4_TYPELESS = 79,
	PIXEL_FORMAT_BC4_UNORM = 80,
	PIXEL_FORMAT_BC4_SNORM = 81,
	PIXEL_FORMAT_BC5_TYPELESS = 82,
	PIXEL_FORMAT_BC5_UNORM = 83,
	PIXEL_FORMAT_BC5_SNORM = 84,
	PIXEL_FORMAT_B5G6R5_UNORM = 85,
	PIXEL_FORMAT_B5G5R5A1_UNORM = 86,
	PIXEL_FORMAT_B8G8R8A8_UNORM = 87,
	PIXEL_FORMAT_B8G8R8X8_UNORM = 88,
	PIXEL_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
	PIXEL_FORMAT_B8G8R8A8_TYPELESS = 90,
	PIXEL_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
	PIXEL_FORMAT_B8G8R8X8_TYPELESS = 92,
	PIXEL_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
	PIXEL_FORMAT_BC6H_TYPELESS = 94,
	PIXEL_FORMAT_BC6H_UF16 = 95,
	PIXEL_FORMAT_BC6H_SF16 = 96,
	PIXEL_FORMAT_BC7_TYPELESS = 97,
	PIXEL_FORMAT_BC7_UNORM = 98,
	PIXEL_FORMAT_BC7_UNORM_SRGB = 99,

	// ASTC (M3, spec §7.2): block-compressed formats the Metal backends sample natively.
	// KTX2 carries ASTC block payloads; DDS cannot. Block size is part of the format name
	// (ASTC 4x4/6x6/8x8). No ASTC 3D block compression (VK_EXT_texture_compression_astc_3d):
	// zero devices expose it (parent spec's ruling).
	PIXEL_FORMAT_ASTC_4x4_UNORM = 100,
	PIXEL_FORMAT_ASTC_4x4_UNORM_SRGB = 101,
	PIXEL_FORMAT_ASTC_6x6_UNORM = 102,
	PIXEL_FORMAT_ASTC_6x6_UNORM_SRGB = 103,
	PIXEL_FORMAT_ASTC_8x8_UNORM = 104,
	PIXEL_FORMAT_ASTC_8x8_UNORM_SRGB = 105,

	PIXEL_FORMAT_SENTINEL,

	PIXEL_FORMAT_FORCE_UINT = 0xffffffff
};

inline bool IsCompressedFormat( PixelFormat format )
{
	switch( format )
	{
	case PIXEL_FORMAT_BC1_TYPELESS:
	case PIXEL_FORMAT_BC1_UNORM:
	case PIXEL_FORMAT_BC1_UNORM_SRGB:
	case PIXEL_FORMAT_BC2_TYPELESS:
	case PIXEL_FORMAT_BC2_UNORM:
	case PIXEL_FORMAT_BC2_UNORM_SRGB:
	case PIXEL_FORMAT_BC3_TYPELESS:
	case PIXEL_FORMAT_BC3_UNORM:
	case PIXEL_FORMAT_BC3_UNORM_SRGB:
	case PIXEL_FORMAT_BC4_TYPELESS:
	case PIXEL_FORMAT_BC4_UNORM:
	case PIXEL_FORMAT_BC4_SNORM:
	case PIXEL_FORMAT_BC5_TYPELESS:
	case PIXEL_FORMAT_BC5_UNORM:
	case PIXEL_FORMAT_BC5_SNORM:
	case PIXEL_FORMAT_BC6H_TYPELESS:
	case PIXEL_FORMAT_BC6H_UF16:
	case PIXEL_FORMAT_BC6H_SF16:
	case PIXEL_FORMAT_BC7_TYPELESS:
	case PIXEL_FORMAT_BC7_UNORM:
	case PIXEL_FORMAT_BC7_UNORM_SRGB:
	case PIXEL_FORMAT_ASTC_4x4_UNORM:
	case PIXEL_FORMAT_ASTC_4x4_UNORM_SRGB:
	case PIXEL_FORMAT_ASTC_6x6_UNORM:
	case PIXEL_FORMAT_ASTC_6x6_UNORM_SRGB:
	case PIXEL_FORMAT_ASTC_8x8_UNORM:
	case PIXEL_FORMAT_ASTC_8x8_UNORM_SRGB:
		return true;

	default:
		return false;
	}
}

inline bool IsDds10Format( PixelFormat format )
{
	switch( format )
	{
	case PIXEL_FORMAT_BC6H_TYPELESS:
	case PIXEL_FORMAT_BC6H_UF16:
	case PIXEL_FORMAT_BC6H_SF16:
	case PIXEL_FORMAT_BC7_TYPELESS:
	case PIXEL_FORMAT_BC7_UNORM:
	case PIXEL_FORMAT_BC7_UNORM_SRGB:
		return true;

	default:
		return false;
	}
}

inline uint32_t GetBlockByteSize( PixelFormat format )
{
	switch( format )
	{
	case PIXEL_FORMAT_BC1_TYPELESS:
	case PIXEL_FORMAT_BC1_UNORM:
	case PIXEL_FORMAT_BC1_UNORM_SRGB:
	case PIXEL_FORMAT_BC4_TYPELESS:
	case PIXEL_FORMAT_BC4_UNORM:
	case PIXEL_FORMAT_BC4_SNORM:
		return 8;

	case PIXEL_FORMAT_BC2_TYPELESS:
	case PIXEL_FORMAT_BC2_UNORM:
	case PIXEL_FORMAT_BC2_UNORM_SRGB:
	case PIXEL_FORMAT_BC3_TYPELESS:
	case PIXEL_FORMAT_BC3_UNORM:
	case PIXEL_FORMAT_BC3_UNORM_SRGB:
	case PIXEL_FORMAT_BC5_TYPELESS:
	case PIXEL_FORMAT_BC5_UNORM:
	case PIXEL_FORMAT_BC5_SNORM:
	case PIXEL_FORMAT_BC6H_TYPELESS:
	case PIXEL_FORMAT_BC6H_UF16:
	case PIXEL_FORMAT_BC6H_SF16:
	case PIXEL_FORMAT_BC7_TYPELESS:
	case PIXEL_FORMAT_BC7_UNORM:
	case PIXEL_FORMAT_BC7_UNORM_SRGB:
	case PIXEL_FORMAT_ASTC_4x4_UNORM:
	case PIXEL_FORMAT_ASTC_4x4_UNORM_SRGB:
	case PIXEL_FORMAT_ASTC_6x6_UNORM:
	case PIXEL_FORMAT_ASTC_6x6_UNORM_SRGB:
	case PIXEL_FORMAT_ASTC_8x8_UNORM:
	case PIXEL_FORMAT_ASTC_8x8_UNORM_SRGB:
		return 16;

	default:
		// Not a block image.
		return 0;
	};
}

// The block's footprint in texels: how many texels wide and tall one compressed block covers.
//
// This is the half of a block format's geometry that GetBlockByteSize does not carry, and it did
// not need to exist until ASTC. Every BC format is 4x4, so for fourteen formats the answer was a
// constant and callers wrote it in by hand -- `width / 4`, `( width + 3 ) / 4`, or a local
// `const unsigned blockPixelSize = 4`. ASTC 4x4 keeps that correct and 6x6 and 8x8 do not: a
// 48-wide 6x6 image has 8 blocks per row, and `48 / 4` says 12. Getting it wrong gives a row
// pitch 1.5x or 2x too large -- garbled or over-read texels rather than a clean failure.
//
// The row pitch of a block image is
//     GetBlockCount( width, GetBlockExtent( f ).width ) * GetBlockByteSize( f )
// and the number of block rows is the same expression on height.
//
// The footprint does NOT follow from the byte size, which is why this cannot be folded into
// GetBlockByteSize: ASTC is 16 bytes at every footprint -- 6x6 stores 36 texels in the same 128
// bits that 4x4 uses for 16, which is the entire point of choosing a larger block.
//
// **{ 1, 1 } for a format that is not block compressed**, deliberately unlike GetBlockByteSize's
// 0. It makes the expression above degrade to `width * bytesPerPixel` with no branch, and it
// cannot divide by zero; 0 there is a sentinel for "not a block image", which IsCompressedFormat
// already answers.
//
// Every format is listed, with no compressed fallback. A new block format added to the enum and
// to IsCompressedFormat but forgotten here therefore lands on { 1, 1 } and fails
// TestPixelFormat's EveryCompressedFormatHasABlockExtentAboveOne, rather than quietly inheriting
// 4x4 and being wrong only for the footprints that are not 4.
struct BlockExtent
{
	uint32_t width;
	uint32_t height;
};

inline BlockExtent GetBlockExtent( PixelFormat format )
{
	switch( format )
	{
	case PIXEL_FORMAT_BC1_TYPELESS:
	case PIXEL_FORMAT_BC1_UNORM:
	case PIXEL_FORMAT_BC1_UNORM_SRGB:
	case PIXEL_FORMAT_BC2_TYPELESS:
	case PIXEL_FORMAT_BC2_UNORM:
	case PIXEL_FORMAT_BC2_UNORM_SRGB:
	case PIXEL_FORMAT_BC3_TYPELESS:
	case PIXEL_FORMAT_BC3_UNORM:
	case PIXEL_FORMAT_BC3_UNORM_SRGB:
	case PIXEL_FORMAT_BC4_TYPELESS:
	case PIXEL_FORMAT_BC4_UNORM:
	case PIXEL_FORMAT_BC4_SNORM:
	case PIXEL_FORMAT_BC5_TYPELESS:
	case PIXEL_FORMAT_BC5_UNORM:
	case PIXEL_FORMAT_BC5_SNORM:
	case PIXEL_FORMAT_BC6H_TYPELESS:
	case PIXEL_FORMAT_BC6H_UF16:
	case PIXEL_FORMAT_BC6H_SF16:
	case PIXEL_FORMAT_BC7_TYPELESS:
	case PIXEL_FORMAT_BC7_UNORM:
	case PIXEL_FORMAT_BC7_UNORM_SRGB:
	case PIXEL_FORMAT_ASTC_4x4_UNORM:
	case PIXEL_FORMAT_ASTC_4x4_UNORM_SRGB:
		return { 4, 4 };

	case PIXEL_FORMAT_ASTC_6x6_UNORM:
	case PIXEL_FORMAT_ASTC_6x6_UNORM_SRGB:
		return { 6, 6 };

	case PIXEL_FORMAT_ASTC_8x8_UNORM:
	case PIXEL_FORMAT_ASTC_8x8_UNORM_SRGB:
		return { 8, 8 };

	default:
		// Not a block image: one texel per "block", so the generic pitch expression is an
		// identity.
		return { 1, 1 };
	};
}

// Square for every format carried today. Returned as a pair anyway, because ASTC itself is not
// limited to square footprints -- 8x5, 10x6 and others exist in the specification -- and a caller
// written against a single number would have to be found again if one is ever added.
inline uint32_t GetBlockWidth( PixelFormat format )
{
	return GetBlockExtent( format ).width;
}

inline uint32_t GetBlockHeight( PixelFormat format )
{
	return GetBlockExtent( format ).height;
}

// Blocks needed to cover `texels` along one axis, rounding up. The rounding is not a nicety: an
// image is only a whole number of blocks when its size is a multiple of the footprint, and a 6x6
// mip chain stops being one almost immediately. A partial block is still a whole block in memory.
inline uint32_t GetBlockCount( uint32_t texels, uint32_t blockExtent )
{
	if( blockExtent == 0 )
	{
		return 0;
	}
	return ( texels + blockExtent - 1 ) / blockExtent;
}

inline unsigned GetBytesPerPixel( PixelFormat format )
{
	switch( format )
	{
	case PIXEL_FORMAT_R32G32B32A32_TYPELESS:
	case PIXEL_FORMAT_R32G32B32A32_FLOAT:
	case PIXEL_FORMAT_R32G32B32A32_UINT:
	case PIXEL_FORMAT_R32G32B32A32_SINT:
	case PIXEL_FORMAT_R32G32B32_TYPELESS:
	case PIXEL_FORMAT_R32G32B32_FLOAT:
	case PIXEL_FORMAT_R32G32B32_UINT:
	case PIXEL_FORMAT_R32G32B32_SINT:
		return 16;
	case PIXEL_FORMAT_R16G16B16A16_TYPELESS:
	case PIXEL_FORMAT_R16G16B16A16_FLOAT:
	case PIXEL_FORMAT_R16G16B16A16_UNORM:
	case PIXEL_FORMAT_R16G16B16A16_UINT:
	case PIXEL_FORMAT_R16G16B16A16_SNORM:
	case PIXEL_FORMAT_R16G16B16A16_SINT:
	case PIXEL_FORMAT_R32G32_TYPELESS:
	case PIXEL_FORMAT_R32G32_FLOAT:
	case PIXEL_FORMAT_R32G32_UINT:
	case PIXEL_FORMAT_R32G32_SINT:
	case PIXEL_FORMAT_R32G8X24_TYPELESS:
	case PIXEL_FORMAT_D32_FLOAT_S8X24_UINT:
	case PIXEL_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case PIXEL_FORMAT_X32_TYPELESS_G8X24_UINT:
		return 8;
	case PIXEL_FORMAT_R10G10B10A2_TYPELESS:
	case PIXEL_FORMAT_R10G10B10A2_UNORM:
	case PIXEL_FORMAT_R10G10B10A2_UINT:
	case PIXEL_FORMAT_R11G11B10_FLOAT:
	case PIXEL_FORMAT_R8G8B8A8_TYPELESS:
	case PIXEL_FORMAT_R8G8B8A8_UNORM:
	case PIXEL_FORMAT_R8G8B8A8_UNORM_SRGB:
	case PIXEL_FORMAT_R8G8B8A8_UINT:
	case PIXEL_FORMAT_R8G8B8A8_SNORM:
	case PIXEL_FORMAT_R8G8B8A8_SINT:
	case PIXEL_FORMAT_R16G16_TYPELESS:
	case PIXEL_FORMAT_R16G16_FLOAT:
	case PIXEL_FORMAT_R16G16_UNORM:
	case PIXEL_FORMAT_R16G16_UINT:
	case PIXEL_FORMAT_R16G16_SNORM:
	case PIXEL_FORMAT_R16G16_SINT:
	case PIXEL_FORMAT_R32_TYPELESS:
	case PIXEL_FORMAT_D32_FLOAT:
	case PIXEL_FORMAT_R32_FLOAT:
	case PIXEL_FORMAT_R32_UINT:
	case PIXEL_FORMAT_R32_SINT:
	case PIXEL_FORMAT_R24G8_TYPELESS:
	case PIXEL_FORMAT_D24_UNORM_S8_UINT:
	case PIXEL_FORMAT_R24_UNORM_X8_TYPELESS:
	case PIXEL_FORMAT_X24_TYPELESS_G8_UINT:
		return 4;
	case PIXEL_FORMAT_R8G8_TYPELESS:
	case PIXEL_FORMAT_R8G8_UNORM:
	case PIXEL_FORMAT_R8G8_UINT:
	case PIXEL_FORMAT_R8G8_SNORM:
	case PIXEL_FORMAT_R8G8_SINT:
	case PIXEL_FORMAT_R16_TYPELESS:
	case PIXEL_FORMAT_R16_FLOAT:
	case PIXEL_FORMAT_D16_UNORM:
	case PIXEL_FORMAT_R16_UNORM:
	case PIXEL_FORMAT_R16_UINT:
	case PIXEL_FORMAT_R16_SNORM:
	case PIXEL_FORMAT_R16_SINT:
		return 2;
	case PIXEL_FORMAT_R8_TYPELESS:
	case PIXEL_FORMAT_R8_UNORM:
	case PIXEL_FORMAT_R8_UINT:
	case PIXEL_FORMAT_R8_SNORM:
	case PIXEL_FORMAT_R8_SINT:
	case PIXEL_FORMAT_A8_UNORM:
		return 1;
	case PIXEL_FORMAT_R9G9B9E5_SHAREDEXP:
	case PIXEL_FORMAT_R8G8_B8G8_UNORM:
	case PIXEL_FORMAT_G8R8_G8B8_UNORM:
		return 4;
	case PIXEL_FORMAT_B5G6R5_UNORM:
	case PIXEL_FORMAT_B5G5R5A1_UNORM:
		return 2;
	case PIXEL_FORMAT_B8G8R8A8_UNORM:
	case PIXEL_FORMAT_B8G8R8X8_UNORM:
	case PIXEL_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case PIXEL_FORMAT_B8G8R8A8_TYPELESS:
	case PIXEL_FORMAT_B8G8R8A8_UNORM_SRGB:
	case PIXEL_FORMAT_B8G8R8X8_TYPELESS:
	case PIXEL_FORMAT_B8G8R8X8_UNORM_SRGB:
		return 4;
	default:
		return 0;
	}
}

}
