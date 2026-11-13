// Copyright © 2026 CCP ehf.

#include "StdAfx.h"

#include <cstdarg>
#include <string>

using namespace ImageIO;

// Result formats its message with printf rules, measuring first and writing second. Both passes
// read the same arguments: the measuring one reads a va_copy. Before, both read one va_list, and
// on x86-64 Linux the second pass printed whatever followed the arguments.

namespace
{

Result ThroughVAList( Result::Code code, const char* format, ... )
{
	va_list args;
	va_start( args, format );
	Result result = Result::FormatVAList( code, format, args );
	va_end( args );
	return result;
}

}

TEST( ImageIOResult, FormatsEveryArgument )
{
	const Result result( Result::INVALID_DATA, "level %u is %d bytes, expected %s (%.1f)", 2u, -32, "64", 1.5 );
	EXPECT_EQ( Result::INVALID_DATA, result.code );
	EXPECT_EQ( "level 2 is -32 bytes, expected 64 (1.5)", result.message );
	EXPECT_EQ( "invalid data in input stream: level 2 is -32 bytes, expected 64 (1.5)", result.GetErrorMessage() );
}

TEST( ImageIOResult, FormatVAListFormatsEveryArgument )
{
	const Result result = ThroughVAList( Result::HEADER_NOT_SUPPORTED, "VkFormat %u, %u levels", 145u, 4u );
	EXPECT_EQ( Result::HEADER_NOT_SUPPORTED, result.code );
	EXPECT_EQ( "VkFormat 145, 4 levels", result.message );
}

TEST( ImageIOResult, KeepsAMessageLongerThanAnyStackBuffer )
{
	const std::string word( 5000, 'x' );
	const Result result( Result::READ_FAILURE, "%s|%s|%d", word.c_str(), word.c_str(), 7 );
	EXPECT_EQ( word + "|" + word + "|7", result.message );
}

TEST( ImageIOResult, AnEmptyFormatLeavesNoMessage )
{
	const Result result( Result::UNKNOWN_FAILURE, "%s", "" );
	EXPECT_TRUE( result.message.empty() );
	EXPECT_EQ( "unknown failure", result.GetErrorMessage() );
}
