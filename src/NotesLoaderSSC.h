/* SSCLoader - Reads a Song and its Steps from a .SSC file. */

#ifndef NotesLoaderSSC_H
#define NotesLoaderSSC_H

#include "GameConstantsAndTypes.h"
#include "Json/Value.h"
#include "JsonUtil.h"
#include "NoteData.h"
#include "NoteTypes.h"
#include "TimingData.h"
#include "BackgroundUtil.h"
#include "Song.h"
#include "Steps.h"

class Song;
class Steps;
class TimingData;

namespace SSCLoader
{
	void GetApplicableFiles( const RString &sPath, vector<RString> &out );
	
	void Deserialize( BPMSegment &seg, const Json::Value &root );
	void Deserialize( StopSegment &seg, const Json::Value &root );
	void DeserializeDelay( StopSegment &seg, const Json::Value &root );
	// void Deserialize( DelaySegment &seg, const Json::Value &root );
	void Deserialize( TickcountSegment &seg, const Json::Value &root );
	void Deserialize( TimeSignatureSegment &seg, const Json::Value &root );
	void Deserialize( TimingData &seg, const Json::Value &root );
	void Deserialize( LyricSegment &seg, const Json::Value &root );
	void Deserialize( BackgroundDef &seg, const Json::Value &root );
	void Deserialize( BackgroundChange &seg, const Json::Value &root );
	void Deserialize( TapNote &seg, const Json::Value &root );
	void Deserialize( NoteData &seg, const Json::Value &root );
	void Deserialize( RadarValues &o, const Json::Value &root );
	void Deserialize( Steps &o, const Json::Value &root );
	void Deserialize( Song &out, const Json::Value &root );
	
	bool LoadFromJsonFile( const RString &sPath, Song &out );
	bool LoadFromDir( const RString &sPath, Song &out );
}

#endif

/*
 * (c) 2001-2011 Chris Danford, Glenn Maynard, spinal shark collective
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
