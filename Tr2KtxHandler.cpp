// Copyright © 2026 CCP ehf.

#include "StdAfx.h"
#include "Tr2KtxHandler.h"
#include "HostBitmap.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace
{

// The twelve-byte identifier from the KTX 2.0 specification, section 3.1. Spelled as bytes rather
// than as a string literal because four of them are not printable and one of them is 0xBB, which
// is the second byte of a UTF-8 BOM -- a source file that happened to be re-saved as UTF-8 could
// otherwise corrupt it invisibly.
const uint8_t s_identifier[12] = {
	0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
};

// KTX2 is little-endian by specification -- there is no endianness field and no swapped variant,
// unlike KTX1. So these structures are read as bytes and used directly, and the only platforms
// this library builds for are little-endian anyway.
#pragma pack( push, 1 )
struct KtxHeader
{
	uint8_t identifier[12];
	uint32_t vkFormat;
	uint32_t typeSize;
	uint32_t pixelWidth;
	uint32_t pixelHeight;
	uint32_t pixelDepth;
	uint32_t layerCount;
	uint32_t faceCount;
	uint32_t levelCount;
	uint32_t supercompressionScheme;

	// Index, section 3.9. Read but almost unused: this first cut writes no data format
	// descriptor beyond the required minimum and no key/value data, and refuses anything with a
	// supercompression global data block.
	uint32_t dfdByteOffset;
	uint32_t dfdByteLength;
	uint32_t kvdByteOffset;
	uint32_t kvdByteLength;
	uint64_t sgdByteOffset;
	uint64_t sgdByteLength;
};

struct KtxLevel
{
	uint64_t byteOffset;
	uint64_t byteLength;
	uint64_t uncompressedByteLength;
};
#pragma pack( pop )

static_assert( sizeof( KtxHeader ) == 80, "KTX2 header is 80 bytes: 12 identifier + 9 words + 4 words + 2 quadwords" );
static_assert( sizeof( KtxLevel ) == 24, "KTX2 level index entry is three 64-bit values" );

// The VkFormat values, which is what KTX2 identifies a format by. Verified against
// vulkan_core.h rather than transcribed from the specification: 157/158, 165/166, 171/172.
//
// Only the three footprints this library carries a PixelFormat for. ASTC defines many more
// (5x4, 6x5, 8x5, 10x6, 12x12 and the 3D ones) and each would need an enum row, a block extent
// and a Metal table row before it could be read; a file using one is refused by name rather
// than mapped to something close.
struct FormatPair
{
	uint32_t vkFormat;
	ImageIO::PixelFormat pixelFormat;
};

const FormatPair s_formats[] = {
	{ 157, ImageIO::PIXEL_FORMAT_ASTC_4x4_UNORM },
	{ 158, ImageIO::PIXEL_FORMAT_ASTC_4x4_UNORM_SRGB },
	{ 165, ImageIO::PIXEL_FORMAT_ASTC_6x6_UNORM },
	{ 166, ImageIO::PIXEL_FORMAT_ASTC_6x6_UNORM_SRGB },
	{ 171, ImageIO::PIXEL_FORMAT_ASTC_8x8_UNORM },
	{ 172, ImageIO::PIXEL_FORMAT_ASTC_8x8_UNORM_SRGB },
};

ImageIO::PixelFormat ToPixelFormat( uint32_t vkFormat )
{
	for( const FormatPair& pair : s_formats )
	{
		if( pair.vkFormat == vkFormat )
		{
			return pair.pixelFormat;
		}
	}
	return ImageIO::PIXEL_FORMAT_UNKNOWN;
}

uint32_t ToVkFormat( ImageIO::PixelFormat pixelFormat )
{
	for( const FormatPair& pair : s_formats )
	{
		if( pair.pixelFormat == pixelFormat )
		{
			return pair.vkFormat;
		}
	}
	return 0;  // VK_FORMAT_UNDEFINED
}

}  // namespace


namespace ImageIO
{
namespace Ktx
{

// --------------------------------------------------------------------------------------
// Description:
//   Registers the KTX2 handler with ImageIO.
// --------------------------------------------------------------------------------------
void RegisterHandler()
{
	static bool s_registered = false;
	if( !s_registered )
	{
		ImageFormatFunctions funcs = { &IsKtxExtension, &ReadImage, &IsSaveSupported, &Save };
		RegisterImageHandler( funcs );
		s_registered = true;
	}
}

// --------------------------------------------------------------------------------------
// Description:
//   Checks if the provided extension (without leading dot) is the KTX2 extension.
// Arguments:
//   ext - File extension
// Return Value:
//   true if the extension is ktx2
// --------------------------------------------------------------------------------------
bool IsKtxExtension( const wchar_t* ext )
{
	// "ktx2" only. Plain ".ktx" is KTX version 1 -- a different header, a GL format enum instead
	// of a Vulkan one, and an endianness field. Claiming it here would mean this handler is
	// handed files it cannot parse and the failure would look like a corrupt texture rather than
	// like the wrong container.
	return ( ext[0] == L'k' || ext[0] == L'K' ) &&
		( ext[1] == L't' || ext[1] == L'T' ) &&
		( ext[2] == L'x' || ext[2] == L'X' ) &&
		ext[3] == L'2' &&
		ext[4] == 0;
}

// --------------------------------------------------------------------------------------
// Description:
//   Reads a KTX2 image from the stream.
// Arguments:
//   stream - Stream used for reading
//   loadParameters - various loading parameters
//   bitmap - (out) Destination bitmap
//   metadata - (out) Optional image metadata
// Return Value:
//   Result of the operation
// --------------------------------------------------------------------------------------
Result ReadImage( ICcpStream& stream, const ImageIO::LoadParameters& loadParameters, ImageIO::HostBitmap& bitmap, ImageIO::Metadata* metadata )
{
	KtxHeader header = KtxHeader();
	if( stream.Read( &header, sizeof( header ) ) == -1 )
	{
		return Result::READ_FAILURE;
	}

	if( std::memcmp( header.identifier, s_identifier, sizeof( s_identifier ) ) != 0 )
	{
		return Result::INVALID_HEADER;
	}

	// Refused before anything else is looked at, because a supercompressed file's level index
	// describes compressed byte ranges and reading them as payload would produce a bitmap of the
	// right size full of the wrong bytes -- the failure would appear as a corrupt texture.
	if( header.supercompressionScheme != 0 )
	{
		// Result carries a message, which is how this library says which thing was unsupported.
		// It never logs, and a bare HEADER_NOT_SUPPORTED would lose exactly the number needed to
		// tell "we do not read Zstd yet" from "this file is not ASTC".
		return Result( Result::HEADER_NOT_SUPPORTED,
			"KTX2 supercompression scheme %u; this handler reads plain payloads only (scheme 0)",
			header.supercompressionScheme );
	}

	const PixelFormat format = ToPixelFormat( header.vkFormat );
	if( format == PIXEL_FORMAT_UNKNOWN )
	{
		return Result( Result::HEADER_NOT_SUPPORTED,
			"KTX2 VkFormat %u is not one of the ASTC formats this handler carries: 157/158 4x4, "
			"165/166 6x6, 171/172 8x8",
			header.vkFormat );
	}

	// faceCount 6 is a cube map and layerCount above 0 is an array; both are legal KTX2 and
	// neither is what the M3 transcode stage emits. Named rather than silently read as a taller
	// 2D image, which is what ignoring them would amount to.
	if( header.faceCount > 1 || header.layerCount > 1 )
	{
		return Result( Result::HEADER_NOT_SUPPORTED,
			"KTX2 faceCount %u layerCount %u; this handler reads plain 2D images only",
			header.faceCount, header.layerCount );
	}

	if( header.pixelWidth == 0 || header.pixelDepth > 1 )
	{
		return Result::INVALID_HEADER;
	}

	// levelCount 0 means "generate the mip chain at load time", which nothing here does; one
	// level is stored either way, so it is read as one and the caller gets what the file holds.
	const uint32_t storedLevels = header.levelCount == 0 ? 1 : header.levelCount;

	std::vector<KtxLevel> levels( storedLevels );
	if( stream.Read( levels.data(), ptrdiff_t( sizeof( KtxLevel ) * storedLevels ) ) == -1 )
	{
		return Result::READ_FAILURE;
	}

	// mipCount is IN/OUT: it goes in as the number of levels the file holds and comes back as
	// the number to load. Passing 0 does not mean "decide for me" -- it means "this file has no
	// mip chain", and the levels loop then runs zero times while the bitmap is still created with
	// a full generated chain. The DDS handler seeds it the same way (Tr2DdsHandler.cpp:488).
	uint32_t skipCount = 0;
	uint32_t mipCount = storedLevels;
	loadParameters.GetMipLevelRange( header.pixelWidth, header.pixelHeight, skipCount, mipCount );
	if( skipCount >= storedLevels )
	{
		// Asked to skip past everything the file has. Not an error in the DDS handler's sense --
		// the caller wanted a smaller image than exists -- but there is nothing to return.
		return Result::INVALID_DATA;
	}
	mipCount = std::min( mipCount, storedLevels - skipCount );

	const uint32_t width = std::max( header.pixelWidth >> skipCount, 1u );
	const uint32_t height = std::max( std::max( header.pixelHeight, 1u ) >> skipCount, 1u );

	const BitmapDimensions dimensions( TEX_TYPE_2D, format, width, height, 1, mipCount );
	if( !bitmap.CreateFromBitmapDimensions( dimensions ) )
	{
		return Result::ERROR_CREATING_BITMAP;
	}

	if( metadata )
	{
		metadata->cutout = Cutout();
	}

	// Level by level, through the index's byte offsets, and NOT by reading forward from here.
	//
	// KTX2 stores its level images smallest-first -- the largest mip is at the end of the file
	// (specification section 3.10, "levelImages are ordered from the smallest to the largest").
	// The level INDEX is ordered the other way, level 0 first. Reading sequentially would
	// therefore fill mip 0 with the smallest image and get progressively more wrong, while
	// producing exactly the right number of bytes. Seeking per level makes the physical order
	// irrelevant, which is what the index is for.
	for( uint32_t level = 0; level < mipCount; ++level )
	{
		const KtxLevel& entry = levels[skipCount + level];
		const size_t expected = dimensions.GetMipSize( level );

		if( entry.byteLength != expected )
		{
			// Checked rather than trusted, and this is the check that would catch a transcode
			// stage emitting the wrong block footprint: the byte count is the only thing in the
			// file that disagrees when 6x6 data is labelled 4x4.
			bitmap.Destroy();
			return Result( Result::INVALID_DATA,
				"KTX2 level %u holds %llu bytes; %llu are needed for %ux%u of this format",
				level, (unsigned long long)entry.byteLength, (unsigned long long)expected,
				dimensions.GetMipWidth( level ), dimensions.GetMipHeight( level ) );
		}

		if( stream.Seek( ptrdiff_t( entry.byteOffset ), ICcpStream::SO_BEGIN ) == -1 )
		{
			bitmap.Destroy();
			return Result::READ_FAILURE;
		}
		if( stream.Read( bitmap.GetMipRawData( level ), ptrdiff_t( expected ) ) == -1 )
		{
			bitmap.Destroy();
			return Result::READ_FAILURE;
		}
	}

	return Result::OK;
}

// --------------------------------------------------------------------------------------
// Description:
//   Checks if saving an image as KTX2 is supported.
// Arguments:
//   dimensions - Image dimensions/type/format
// Return Value:
//   Result of the operation (OK if image saving is supported)
// --------------------------------------------------------------------------------------
Result IsSaveSupported( const BitmapDimensions& bd )
{
	// Deliberately the same narrow set Read accepts. A container that can be written but not read
	// back by the same library is a trap, and the round-trip test in the suite is what this
	// symmetry exists for.
	if( ToVkFormat( bd.GetFormat() ) == 0 )
	{
		return Result::SAVE_NOT_SUPPORTED;
	}
	if( bd.GetType() != TEX_TYPE_2D || bd.GetArraySize() > 1 )
	{
		return Result::SAVE_NOT_SUPPORTED;
	}
	return Result::OK;
}

// --------------------------------------------------------------------------------------
// Description:
//   Saves a bitmap as a KTX2 file.
// Arguments:
//   image - Bitmap to save
//   output - Destination stream
//   metadata - Optional metadata (unused: this first cut writes no key/value data)
// Return Value:
//   Result of the operation
// --------------------------------------------------------------------------------------
Result Save( const ImageIO::HostBitmap& image, ICcpStream& output, const Metadata* metadata )
{
	if( !image.IsValid() )
	{
		return Result::INVALID_BITMAP;
	}

	// HostBitmap derives from BitmapDimensions, so the image is its own dimensions -- named as a
	// reference here only so the two dozen uses below read as geometry rather than as pixels.
	const BitmapDimensions& dimensions = image;
	IMAGE_IO_CR_RETURN_RESULT( IsSaveSupported( dimensions ) );

	const uint32_t levelCount = std::max( dimensions.GetTrueMipCount(), 1u );

	KtxHeader header = KtxHeader();
	std::memcpy( header.identifier, s_identifier, sizeof( s_identifier ) );
	header.vkFormat = ToVkFormat( dimensions.GetFormat() );
	// 1 for a block-compressed format: typeSize is the size of the *channel* type for endian
	// conversion, and a block has no channels to swap (specification section 3.3).
	header.typeSize = 1;
	header.pixelWidth = dimensions.GetWidth();
	header.pixelHeight = dimensions.GetHeight();
	header.pixelDepth = 0;   // 0, not 1: KTX2 spells "not a 3D texture" as zero.
	header.layerCount = 0;   // 0, not 1: likewise "not an array".
	header.faceCount = 1;
	header.levelCount = levelCount;
	header.supercompressionScheme = 0;

	// No data format descriptor and no key/value data. The specification requires a DFD, and a
	// reader that needs one to identify the format will reject this file -- but vkFormat already
	// carries everything both halves of this library use, and writing a correct ASTC DFD means
	// the whole KHR_DF block layout for no reader that exists yet. Recorded as the deliberate
	// gap it is: dfdByteLength 0 is what makes it visible rather than subtly malformed.
	const uint32_t levelIndexBytes = uint32_t( sizeof( KtxLevel ) * levelCount );
	uint64_t offset = sizeof( KtxHeader ) + levelIndexBytes;

	// Levels are written smallest-first, as the specification requires, so the offsets are
	// computed from the far end of the level list inward.
	std::vector<KtxLevel> levels( levelCount );
	for( uint32_t i = 0; i < levelCount; ++i )
	{
		const uint32_t level = levelCount - 1 - i;  // smallest first
		const uint64_t size = dimensions.GetMipSize( level );
		levels[level].byteOffset = offset;
		levels[level].byteLength = size;
		levels[level].uncompressedByteLength = size;  // equal: nothing is supercompressed
		offset += size;
	}

	if( output.Write( &header, sizeof( header ) ) == -1 )
	{
		return Result::WRITE_FAILURE;
	}
	if( output.Write( levels.data(), levelIndexBytes ) == -1 )
	{
		return Result::WRITE_FAILURE;
	}

	// And the payload in the same smallest-first order the offsets just described.
	for( uint32_t i = 0; i < levelCount; ++i )
	{
		const uint32_t level = levelCount - 1 - i;
		if( output.Write( image.GetMipRawData( level ), size_t( dimensions.GetMipSize( level ) ) ) == -1 )
		{
			return Result::WRITE_FAILURE;
		}
	}

	return Result::OK;
}

}
}
