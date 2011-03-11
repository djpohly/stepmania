/* MovieTexture_FFMpeg - FFMpeg movie renderer. */

#ifndef RAGE_MOVIE_TEXTURE_FFMPEG_H
#define RAGE_MOVIE_TEXTURE_FFMPEG_H

#include "MovieTexture_Generic.h"
struct RageSurface;

class RageMovieTextureDriver_FFMpeg: public RageMovieTextureDriver
{
public:
	virtual RageMovieTexture *Create( RageTextureID ID, RString &sError );
	static RageSurface *AVCodecCreateCompatibleSurface( int iTextureWidth, int iTextureHeight, bool bPreferHighColor, int &iAVTexfmt, MovieDecoderPixelFormatYCbCr &fmtout );
};

namespace avcodec
{
#if defined(MACOSX)
	// old shit
#include <ffmpeg/avformat.h>
#else
	/* ...shit. this is gonna break building on Linux and Mac, isn't it?
	 * (the way I have it set to include ffmpeg/crap/header.h instead of
	 * crap/header.h like in the patch: http://pastie.org/701873 )
	 */
	extern "C"
	{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
	}
#endif
};

#if defined(_MSC_VER) && !defined(XBOX)
#pragma comment(lib, "ffmpeg/lib/avcodec.lib")
#pragma comment(lib, "ffmpeg/lib/avformat.lib")
#if defined(USE_MODERN_FFMPEG)
#pragma comment(lib, "ffmpeg/lib/swscale.lib")
#endif
#endif // _MSC_VER && !XBOX

#if defined(XBOX)
/* NOTES: ffmpeg static libraries arent included in SVN. You have to build
 *them yourself or remove this file to produce the xbox build.
 * 
 * build ffmpeg with mingw32 ( howto http://ffmpeg.arrozcru.org/wiki/index.php?title=Main_Page )
 * ./configure --enable-memalign-hack --enable-static --disable-mmx --target-os=mingw32 --arch=x86
 * you can use various switches to enable/disable codecs/muxers/etc. 
 * 
 * libgcc.a and libmingwex.a comes from mingw installation
 * msys\mingw\lib\gcc\mingw32\3.4.5\libgcc.a */
#pragma comment(lib, "ffmpeg/lib/libavcodec.a")
#pragma comment(lib, "ffmpeg/lib/libavformat.a")
#pragma comment(lib, "ffmpeg/lib/libavutil.a")
#pragma comment(lib, "ffmpeg/lib/libswscale.a")
#pragma comment(lib, "ffmpeg/lib/libgcc.a")
#pragma comment(lib, "ffmpeg/lib/libmingwex.a")
#pragma comment(lib, "ffmpeg/lib/libcoldname.a")
#endif

#if !defined(MACOSX)
static const int sws_flags = SWS_BICUBIC; // XXX: Reasonable default?
#endif


int URLRageFile_read( avcodec::URLContext *h, unsigned char *buf, int size );
int URLRageFile_open( avcodec::URLContext *h, const char *filename, int flags );
int URLRageFile_close( avcodec::URLContext *h );

#endif

/*
 * (c) 2003-2005 Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
