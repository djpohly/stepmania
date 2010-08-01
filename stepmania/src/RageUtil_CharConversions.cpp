#include "global.h"
#include "RageUtil_CharConversions.h"

#include "RageUtil.h"
#include "RageLog.h"

#if defined(_WINDOWS)

#include "windows.h"

/* Convert from the given codepage to UTF-8.  Return true if successful. */
static bool CodePageConvert(RString &txt, int cp)
{
	if( txt.size() == 0 )
		return true;

	int size = MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, txt.data(), txt.size(), NULL, 0);
	if( size == 0 )
	{
		LOG->Trace("%s\n", werr_ssprintf(GetLastError(), "err: ").c_str());
		return false; /* error */
	}

	wstring out;
	out.append(size, ' ');
	/* Nonportable: */
	size = MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, txt.data(), txt.size(), (wchar_t *) out.data(), size);
	ASSERT( size != 0 );

	txt = WStringToRString(out);
	return true;
}

static bool AttemptEnglishConversion( RString &txt ) { return CodePageConvert( txt, 1252 ); }
static bool AttemptKoreanConversion( RString &txt ) { return CodePageConvert( txt, 949 ); }
static bool AttemptJapaneseConversion( RString &txt ) { return CodePageConvert( txt, 932 ); }

#elif defined(HAVE_ICONV)
#include <errno.h>
#include <iconv.h>

static bool ConvertFromCharset( RString &txt, const char *charset )
{
	if ( txt.size() == 0 )
		return true;

	iconv_t converter = iconv_open( "UTF-8", charset );
	if( converter == (iconv_t) -1 )
	{
		LOG->MapLog( ssprintf("conv %s", charset), "iconv_open(%s): %s", charset, strerror(errno) );
		return false;
	}

	/* Copy the string into a char* for iconv */
	ICONV_CONST char *txtin = const_cast<ICONV_CONST char*>( txt.data() );
	size_t inleft = txt.size();

	/* Create a new string with enough room for the new conversion */
	RString buf;
	buf.resize( txt.size() * 5 );

	char *txtout = const_cast<char*>( buf.data() );
	size_t outleft = buf.size();
	size_t size = iconv( converter, &txtin, &inleft, &txtout, &outleft );

	iconv_close( converter );

	if( size == (size_t)(-1) )
	{
		LOG->Trace( "%s\n", strerror( errno ) );
		return false; /* Returned an error */
	}

	if( inleft != 0 )
	{
		LOG->Warn( "iconv(UTF-8,%s) for \"%s\": whole buffer not converted (%i left)", charset, txt.c_str(), int(inleft) );
		return false;
	}

	if( buf.size() == outleft )
		return false; /* Conversion failed */

	buf.resize( buf.size()-outleft );

	txt = buf;
	return true;
}

static bool AttemptEnglishConversion( RString &txt ) { return ConvertFromCharset( txt, "CP1252" ); }
static bool AttemptKoreanConversion( RString &txt ) { return ConvertFromCharset( txt, "CP949" ); }
static bool AttemptJapaneseConversion( RString &txt ) { return ConvertFromCharset( txt, "CP932" ); }

#elif defined(MACOSX)
#include <CoreFoundation/CoreFoundation.h>

static bool ConvertFromCP( RString &txt, int cp )
{
	CFStringEncoding encoding = CFStringConvertWindowsCodepageToEncoding( cp );
	
	if( encoding == kCFStringEncodingInvalidId )
		return false;
	
	CFStringRef old = CFStringCreateWithCString( kCFAllocatorDefault, txt, encoding );
	
	if( old == NULL )
		return false;
	const size_t size = CFStringGetMaximumSizeForEncoding( CFStringGetLength(old), kCFStringEncodingUTF8 );
	
	char *buf = new char[size];
	if( !CFStringGetCString(old, buf, size, kCFStringEncodingUTF8) )
	{
		delete[] buf;
		return false;
	}
	txt = buf;
	delete[] buf;
	return true;
}

static bool AttemptEnglishConversion( RString &txt ) { return ConvertFromCP( txt, 1252 ); }
static bool AttemptKoreanConversion( RString &txt ) { return ConvertFromCP( txt, 949 ); }
static bool AttemptJapaneseConversion( RString &txt ) { return ConvertFromCP( txt, 932 ); }

#else

/* No converters are available, so all fail--we only accept UTF-8. */
static bool AttemptEnglishConversion( RString &txt ) { return false; }
static bool AttemptKoreanConversion( RString &txt ) { return false; }
static bool AttemptJapaneseConversion( RString &txt ) { return false; }

#endif

bool ConvertString(RString &str, const RString &encodings)
{
	vector<RString> lst;
	split(encodings, ",", lst);

	for(unsigned i = 0; i < lst.size(); ++i)
	{
		if( lst[i] == "utf-8" )
		{
			/* Is the string already valid utf-8? */
			if( utf8_is_valid(str) )
				return true;
			continue;
		}
		if( lst[i] == "english" )
		{
			if(AttemptEnglishConversion(str))
				return true;
			continue;
		}

		if( lst[i] == "japanese" )
		{
			if(AttemptJapaneseConversion(str))
				return true;
			continue;
		}

		if( lst[i] == "korean" )
		{
			if(AttemptKoreanConversion(str))
				return true;
			continue;
		}

		RageException::Throw( "Unexpected conversion string \"%s\" (string \"%s\")",
						lst[i].c_str(), str.c_str() );
	}

	return false;
}

/* Written by Glenn Maynard.  In the public domain; there are so many
 * simple conversion interfaces that restricting them is silly. */
