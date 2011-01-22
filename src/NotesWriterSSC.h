/* NotesWriterJson - Writes a Song and its Steps to a .ssc file. */

#ifndef NotesWriterJson_H
#define NotesWriterJson_H

#include "Json/Value.h"
#include "BackgroundUtil.h"
#include "JsonUtil.h"
#include "NoteData.h"
#include "RadarValues.h"
#include "TimingData.h"
#include "Song.h"

class Song;

namespace SSCWriter
{
	void Serialize(const BPMSegment &seg, Json::Value &root);
	void Serialize(const StopSegment &seg, Json::Value &root);
	void SerializeDelay(const StopSegment &seg, Json::Value &root);
	void Serialize(const TickcountSegment &seg, Json::Value &root );
	void Serialize(const TimeSignatureSegment &seg, Json::Value &root );
	void Serialize(const TimingData &td, Json::Value &root);
	void Serialize(const LyricSegment &o, Json::Value &root);
	void Serialize(const BackgroundDef &o, Json::Value &root);
	void Serialize(const BackgroundChange &o, Json::Value &root );
	void Serialize( const TapNote &o, Json::Value &root );
	void Serialize( const NoteData &o, Json::Value &root );
	void Serialize( const RadarValues &o, Json::Value &root );
	void Serialize( const Steps &o, Json::Value &root );
	
	bool Write( RString sPath, const Song &out, bool bSavingCache );
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
