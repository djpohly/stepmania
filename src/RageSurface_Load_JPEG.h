/* RageSurface_Load_JPEG - JPEG file loader */

#ifndef RAGE_SURFACE_LOAD_JPEG_H
#define RAGE_SURFACE_LOAD_JPEG_H

#include "RageSurface_Load.h"
RageSurfaceUtils::OpenResult RageSurface_Load_JPEG( const RString &sPath, RageSurface *&ret, bool bHeaderOnly, RString &error );

// Don't let jpeglib.h define the boolean type on Xbox.
#if defined(_XBOX)
#  define HAVE_BOOLEAN
#endif

#if defined(WIN32)
/* work around namespace bugs in win32/libjpeg: */
#define XMD_H
#undef FAR
#include "libjpeg/jpeglib.h"
#include "libjpeg/jerror.h"

#if defined(_MSC_VER)
#if !defined(XBOX)
#pragma comment(lib, "libjpeg/jpeg.lib")
#else
#pragma comment(lib, "libjpeg/xboxjpeg.lib")
#endif
#endif

#pragma warning(disable: 4611) /* interaction between '_setjmp' and C++ object destruction is non-portable */
#else
extern "C" {
#include <jpeglib.h>
#include <jerror.h>
}
#endif

void my_output_message( j_common_ptr cinfo );
void my_error_exit( j_common_ptr cinfo );
void RageFile_JPEG_init_source( j_decompress_ptr cinfo );
boolean RageFile_JPEG_fill_input_buffer( j_decompress_ptr cinfo );
void RageFile_JPEG_skip_input_data( j_decompress_ptr cinfo, long num_bytes );
void RageFile_JPEG_term_source( j_decompress_ptr cinfo );

#endif

/*
 * (c) 2004 Glenn Maynard
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
