// Copyright © 2026 CCP ehf.

#pragma once

#ifndef Tr2KtxHandler_h_
#define Tr2KtxHandler_h_

#include "Tr2ImageHandler.h"

// KTX2 (KHR_texture_basisu's container, without the supercompression), for one reason: it is the
// only container in this library that can carry ASTC. DDS cannot -- its DX10 header enumerates
// DXGI formats and DXGI has no ASTC at all -- so a format the Apple backends sample natively had
// nowhere to live. Everything else about this handler mirrors Tr2DdsHandler on purpose.
//
// First cut, deliberately narrow (M3 spec 7.1): plain ASTC payloads only. Supercompressed files
// (Basis LZ, Zstd) are refused by name rather than half-read, because supporting them means a
// dependency and a transcoder and neither is needed to ship a texture the GPU reads directly.
namespace ImageIO
{
namespace Ktx
{

void RegisterHandler();
bool IsKtxExtension( const wchar_t* extension );
Result ReadImage( ICcpStream& src, const ImageIO::LoadParameters& loadParameters, ImageIO::HostBitmap& bitmap, ImageIO::Metadata* metadata );
Result IsSaveSupported( const BitmapDimensions& bd );
Result Save( const ImageIO::HostBitmap& image, ICcpStream& output, const Metadata* metadata );

}
}

#endif
