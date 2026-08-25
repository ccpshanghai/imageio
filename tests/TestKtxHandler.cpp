// Copyright © 2026 CCP ehf.

#include "StdAfx.h"
#include "TestHelpers.h"
#include "MemoryStream.h"
#include "Tr2KtxHandler.h"

using namespace ImageIO;

namespace
{

// The other handlers' tests load a compiled-in file. There is no KTX2 asset to load yet -- this
// handler is the thing that will produce them -- so the fixtures are synthesised, which also
// means the payload can be a pattern chosen to make a misplaced mip obvious.
HostBitmap MakeAstc( PixelFormat format, uint32_t width, uint32_t height, uint32_t mipCount, uint8_t seed )
{
	HostBitmap bitmap;
	EXPECT_TRUE( bitmap.Create( width, height, mipCount, format ) );

	// A different value per mip level, so a mip written to the wrong offset is visible as a whole
	// level of the wrong byte rather than as a subtle difference.
	for( uint32_t level = 0; level < bitmap.GetTrueMipCount(); ++level )
	{
		uint8_t* data = reinterpret_cast<uint8_t*>( bitmap.GetMipRawData( level ) );
		const size_t size = bitmap.GetMipSize( level );
		for( size_t i = 0; i < size; ++i )
		{
			data[i] = uint8_t( seed + level * 17 + ( i & 0x3F ) );
		}
	}
	return bitmap;
}

void AssertRoundTrips( PixelFormat format, uint32_t width, uint32_t height, uint32_t mipCount )
{
	const HostBitmap written = MakeAstc( format, width, height, mipCount, 0x40 );
	ASSERT_TRUE( written.IsValid() );

	WriteMemoryStream out;
	ASSERT_TRUE( SaveImage( L"synthetic.ktx2", written, out ) );

	HostBitmap read;
	ReadMemoryStream in( out.GetData(), out.GetDataSize() );
	ASSERT_TRUE( ReadImage( in, LoadParameters( L"synthetic.ktx2" ), read ) );

	EXPECT_EQ( written.GetFormat(), read.GetFormat() );
	EXPECT_EQ( written.GetWidth(), read.GetWidth() );
	EXPECT_EQ( written.GetHeight(), read.GetHeight() );
	EXPECT_EQ( written.GetTrueMipCount(), read.GetTrueMipCount() );
	ASSERT_EQ( written.GetRawDataSize(), read.GetRawDataSize() );

	// Per level, not one memcmp over the whole buffer: the failure this is guarding against is
	// mips landing in the wrong order, and a single comparison would say "differs" without
	// saying where.
	for( uint32_t level = 0; level < written.GetTrueMipCount(); ++level )
	{
		const size_t size = written.GetMipSize( level );
		EXPECT_EQ( 0, memcmp( written.GetMipRawData( level ), read.GetMipRawData( level ), size ) )
			<< "level " << level << " of " << written.GetTrueMipCount();
	}
}

// A header good enough to reach the check under test, so that each refusal test changes exactly
// one field and cannot accidentally be refused for the wrong reason.
std::vector<uint8_t> MinimalHeader( uint32_t vkFormat, uint32_t supercompression )
{
	std::vector<uint8_t> bytes( 80 + 24, 0 );
	const uint8_t identifier[12] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
	memcpy( bytes.data(), identifier, sizeof( identifier ) );
	auto word = [&]( size_t offset, uint32_t value ) { memcpy( bytes.data() + offset, &value, 4 ); };
	word( 12, vkFormat );       // vkFormat
	word( 16, 1 );              // typeSize
	word( 20, 4 );              // pixelWidth
	word( 24, 4 );              // pixelHeight
	word( 28, 0 );              // pixelDepth
	word( 32, 0 );              // layerCount
	word( 36, 1 );              // faceCount
	word( 40, 1 );              // levelCount
	word( 44, supercompression );
	uint64_t offset = 104, length = 16;
	memcpy( bytes.data() + 80, &offset, 8 );
	memcpy( bytes.data() + 88, &length, 8 );
	memcpy( bytes.data() + 96, &length, 8 );
	return bytes;
}

Result ReadHeaderOnly( const std::vector<uint8_t>& bytes )
{
	HostBitmap bitmap;
	ReadMemoryStream in( bytes.data(), bytes.size() );
	return Ktx::ReadImage( in, LoadParameters( L"synthetic.ktx2" ), bitmap, nullptr );
}

}  // namespace


TEST( KtxHandler, TheExtensionIsKtx2AndNotKtx )
{
	EXPECT_TRUE( Ktx::IsKtxExtension( L"ktx2" ) );
	EXPECT_TRUE( Ktx::IsKtxExtension( L"KTX2" ) );

	// Plain .ktx is KTX version 1 -- a different header with a GL format enum and an endianness
	// field. Claiming it would mean being handed files this parser cannot read, and the failure
	// would look like a corrupt texture instead of the wrong container.
	EXPECT_FALSE( Ktx::IsKtxExtension( L"ktx" ) );
	EXPECT_FALSE( Ktx::IsKtxExtension( L"ktx22" ) );
	EXPECT_FALSE( Ktx::IsKtxExtension( L"dds" ) );
}

TEST( KtxHandler, RoundTripsAFourByFourPayload )
{
	AssertRoundTrips( PIXEL_FORMAT_ASTC_4x4_UNORM, 16, 16, 1 );
	AssertRoundTrips( PIXEL_FORMAT_ASTC_4x4_UNORM_SRGB, 16, 16, 1 );
}

TEST( KtxHandler, RoundTripsTheFootprintsThatAreNotFour )
{
	// 6x6 is M3's default block size and the one every `/ 4` in the tree used to get wrong, so it
	// is here at a size that is NOT a whole number of blocks: 50 texels is 9 blocks, not 8.
	AssertRoundTrips( PIXEL_FORMAT_ASTC_6x6_UNORM, 48, 36, 1 );
	AssertRoundTrips( PIXEL_FORMAT_ASTC_6x6_UNORM, 50, 50, 1 );
	AssertRoundTrips( PIXEL_FORMAT_ASTC_8x8_UNORM, 64, 64, 1 );
	AssertRoundTrips( PIXEL_FORMAT_ASTC_8x8_UNORM_SRGB, 33, 17, 1 );
}

TEST( KtxHandler, AnAstcImageNeedNotBeAWholeNumberOfBlocks )
{
	// HostBitmap::Create used to refuse any compressed image whose size was not a multiple of 4,
	// which is a convention this library chose for BC rather than a rule of the formats. ASTC does
	// not inherit it: the last block may be partial and the encoder pads it. 30x18 of ASTC 4x4 is
	// the case that was refused outright, and 50x50 of 6x6 is the one no `% n` test can express --
	// 256, the size the pipeline's textures actually are, is not a multiple of 6 either.
	AssertRoundTrips( PIXEL_FORMAT_ASTC_4x4_UNORM, 30, 18, 1 );
	AssertRoundTrips( PIXEL_FORMAT_ASTC_6x6_UNORM, 50, 50, 1 );
}

TEST( KtxHandler, AMipChainSurvivesTheSmallestFirstOrdering )
{
	// The trap this is for: KTX2 stores level images smallest-first while the level INDEX is
	// ordered largest-first. Reading the file sequentially produces exactly the right number of
	// bytes with every mip in the wrong place, so a size assertion cannot catch it -- only
	// comparing each level's contents can. MakeAstc gives every level a different byte for that
	// reason.
	AssertRoundTrips( PIXEL_FORMAT_ASTC_4x4_UNORM, 32, 32, 6 );
	AssertRoundTrips( PIXEL_FORMAT_ASTC_6x6_UNORM, 64, 64, 4 );
}

TEST( KtxHandler, RefusesSupercompressionByName )
{
	// Refused rather than half-read: a supercompressed level index describes compressed byte
	// ranges, so reading them as payload yields a bitmap of the right size full of wrong bytes.
	const Result result = ReadHeaderOnly( MinimalHeader( 157, 1 /* Basis LZ */ ) );
	EXPECT_EQ( Result::HEADER_NOT_SUPPORTED, result.code );
	EXPECT_NE( std::string::npos, result.GetErrorMessage().find( "supercompression" ) )
		<< result.GetErrorMessage();
}

TEST( KtxHandler, RefusesAFormatItHasNoPixelFormatFor )
{
	// 145 is VK_FORMAT_BC7_UNORM_BLOCK: a real format, legal in KTX2, and not one this handler
	// carries. The message names the number so "we do not read that yet" is distinguishable from
	// "that file is broken".
	const Result result = ReadHeaderOnly( MinimalHeader( 145, 0 ) );
	EXPECT_EQ( Result::HEADER_NOT_SUPPORTED, result.code );
	EXPECT_NE( std::string::npos, result.GetErrorMessage().find( "145" ) ) << result.GetErrorMessage();
}

TEST( KtxHandler, RefusesAFileThatIsNotKtx2 )
{
	std::vector<uint8_t> bytes = MinimalHeader( 157, 0 );
	bytes[1] = 'X';  // break the identifier
	EXPECT_EQ( Result::INVALID_HEADER, ReadHeaderOnly( bytes ).code );
}

TEST( KtxHandler, RefusesALevelWhoseByteLengthDisagreesWithTheFormat )
{
	// The check that would catch a transcode stage emitting 6x6 data under a 4x4 label: the byte
	// count is the only field in the file that disagrees when the footprint is mislabelled.
	std::vector<uint8_t> bytes = MinimalHeader( 157, 0 );
	const uint64_t wrong = 32;  // a 4x4 image of ASTC 4x4 is one 16-byte block, not two
	memcpy( bytes.data() + 88, &wrong, 8 );

	const Result result = ReadHeaderOnly( bytes );
	EXPECT_EQ( Result::INVALID_DATA, result.code );
	EXPECT_NE( std::string::npos, result.GetErrorMessage().find( "32" ) ) << result.GetErrorMessage();
}

TEST( KtxHandler, SavingRefusesWhatReadingWouldRefuse )
{
	// Symmetry on purpose: a container this library can write but not read back is a trap, and
	// the round-trip tests above are what that symmetry is for.
	HostBitmap bc;
	ASSERT_TRUE( bc.Create( 16, 16, 1, PIXEL_FORMAT_BC7_UNORM ) );
	EXPECT_EQ( Result::SAVE_NOT_SUPPORTED, Ktx::IsSaveSupported( bc ).code );

	HostBitmap rgba;
	ASSERT_TRUE( rgba.Create( 16, 16, 1, PIXEL_FORMAT_B8G8R8A8_UNORM ) );
	EXPECT_EQ( Result::SAVE_NOT_SUPPORTED, Ktx::IsSaveSupported( rgba ).code );

	HostBitmap astc;
	ASSERT_TRUE( astc.Create( 16, 16, 1, PIXEL_FORMAT_ASTC_6x6_UNORM ) );
	EXPECT_EQ( Result::OK, Ktx::IsSaveSupported( astc ).code );
}
