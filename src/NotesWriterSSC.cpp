#include "global.h"
#include "NotesWriterSSC.h"
#include "TimingData.h"
#include "Json/Value.h"
#include "JsonUtil.h"
#include "Song.h"
#include "BackgroundUtil.h"
#include "Steps.h"
#include "NoteData.h"
#include "GameManager.h"

void SSCWriter::Serialize(const BPMSegment &seg, Json::Value &root)
{
	root["StartRow"] = seg.m_iStartRow;
	root["BPS"] = seg.m_fBPS;
}

void SSCWriter::Serialize(const StopSegment &seg, Json::Value &root)
{
	if (seg.m_bDelay) return;
	root["StartRow"] = seg.m_iStartRow;
	root["StopSeconds"] = seg.m_fStopSeconds;
}

void SSCWriter::SerializeDelay(const StopSegment &seg, Json::Value &root)
{
	if (!seg.m_bDelay) return;
	root["StartRow"] = seg.m_iStartRow;
	root["StopSeconds"] = seg.m_fStopSeconds;
}

void SSCWriter::Serialize(const TickcountSegment &seg, Json::Value &root )
{
	root["StartRow"] = seg.m_iStartRow;
	root["Ticks"] = seg.m_iTicks;
}

void SSCWriter::Serialize(const TimeSignatureSegment &seg, Json::Value &root )
{
	root["StartRow"] = seg.m_iStartRow;
	root["Numerator"] = seg.m_iNumerator;
	root["Denominator"] = seg.m_iDenominator;
}

void SSCWriter::Serialize(const TimingData &td, Json::Value &root)
{
	JsonUtil::SerializeVectorObjects( td.m_BPMSegments, Serialize, root["BPMs"] );
	JsonUtil::SerializeVectorObjects( td.m_StopSegments, Serialize, root["Stops"] );
	JsonUtil::SerializeVectorObjects( td.m_StopSegments, SerializeDelay, root["Delays"] );
	JsonUtil::SerializeVectorObjects( td.m_TickcountSegments, Serialize, root["Tickcounts"] );
	JsonUtil::SerializeVectorObjects( td.m_vTimeSignatureSegments, Serialize, root["TimeSignatures"] );
}

void SSCWriter::Serialize(const LyricSegment &o, Json::Value &root)
{
	root["StartTime"] = (float)o.m_fStartTime;
	root["Lyric"] = o.m_sLyric;
	root["Color"] = o.m_Color.ToString();
}

void SSCWriter::Serialize(const BackgroundDef &o, Json::Value &root)
{
	root["Effect"] = o.m_sEffect;
	root["File1"] = o.m_sFile1;
	root["File2"] = o.m_sFile2;
	root["Color1"] = o.m_sColor1;
}

void SSCWriter::Serialize(const BackgroundChange &o, Json::Value &root )
{
	Serialize( o.m_def, root["Def"] );
	root["StartBeat"] = o.m_fStartBeat;
	root["Rate"] = o.m_fRate;
	root["Transition"] = o.m_sTransition;
}

void SSCWriter::Serialize( const TapNote &o, Json::Value &root )
{
	root = Json::Value(Json::objectValue);

	if( o.type != TapNote::tap )
		root["Type"] = (int)o.type;
	if( o.type == TapNote::hold_head )
		root["SubType"] = (int)o.subType;
	//root["Source"] = (int)source;
	if( !o.sAttackModifiers.empty() )
		root["AttackModifiers"] = o.sAttackModifiers;
	if( o.fAttackDurationSeconds > 0 )
		root["AttackDurationSeconds"] = o.fAttackDurationSeconds;
	if( o.iKeysoundIndex )
		root["KeysoundIndex"] = o.iKeysoundIndex;
	if( o.iDuration > 0 )
		root["Duration"] = o.iDuration;
	if( o.pn != PLAYER_INVALID )
		root["PlayerNumber"] = (int)o.pn;
}

void SSCWriter::Serialize( const NoteData &o, Json::Value &root )
{
	root = Json::Value(Json::arrayValue);
	int iNumTracks = o.GetNumTracks();
	root.resize( iNumTracks );
	for(int t=0; t<iNumTracks; t++ )
	{
		NoteData::TrackMap tm = o.GetTrack(t);
		Json::Value &root2 = root[t];
		FOREACHM_CONST( int, TapNote, tm, iter )
		{
			int row = iter->first;
			TapNote tn = iter->second;
			RString sKey = ssprintf("%d",row);
			Json::Value &root3 = root2[sKey];
			Serialize( tn, root3 );
		}
	}
}

void SSCWriter::Serialize( const RadarValues &o, Json::Value &root )
{
	
	FOREACH_ENUM( RadarCategory, rc )
	{
		root[ RadarCategoryToString(rc) ] = o.m_Values.f[rc];
	}
}

void SSCWriter::Serialize( const Steps &o, Json::Value &root )
{
	root["StepsType"] = GAMEMAN->GetStepsTypeInfo(o.m_StepsType).szName;

	o.Decompress();
	
	Serialize( o.m_Timing, root["TimingData"] );

	NoteData nd;
	o.GetNoteData( nd );
	Serialize( nd, root["NoteData"] );
	root["Hash"] = o.GetHash();
	root["Description"] = o.GetDescription();
	root["Difficulty"] = DifficultyToString(o.GetDifficulty());
	root["Meter"] = o.GetMeter();
	FOREACH_PlayerNumber( pn )
		Serialize( o.GetRadarValues(pn), root["RadarValues"] );
}


bool SSCWriter::Write( RString sFile, const Song &out, bool bSavingCache )
{
	Json::Value root;
	root["SongDir"] = out.GetSongDir();
	root["GroupName"] = out.m_sGroupName;
	root["Title"] = out.m_sMainTitle;
	root["SubTitle"] = out.m_sSubTitle;
	root["Artist"] = out.m_sArtist;
	root["TitleTranslit"] = out.m_sMainTitleTranslit;
	root["SubTitleTranslit"] = out.m_sSubTitleTranslit;
	root["Genre"] = out.m_sGenre;
	root["Credit"] = out.m_sCredit;
	root["Banner"] = out.m_sBannerFile;
	root["Background"] = out.m_sBackgroundFile;
	root["LyricsFile"] = out.m_sLyricsFile;
	root["CDTitle"] = out.m_sCDTitleFile;
	root["Music"] = out.m_sMusicFile;
	root["Offset"] = out.m_Timing.m_fBeat0OffsetInSeconds;
	root["SampleStart"] = out.m_fMusicSampleStartSeconds;
	root["SampleLength"] = out.m_fMusicSampleLengthSeconds;
	
	root["Selectable"] = out.m_SelectionDisplay == Song::SHOW_ALWAYS ? "YES" : "NO";
	
	root["FirstBeat"] = out.m_fFirstBeat;
	root["LastBeat"] = out.m_fLastBeat;
	root["SongFileName"] = out.m_sSongFileName;
	root["HasMusic"] = out.m_bHasMusic;
	root["HasBanner"] = out.m_bHasBanner;
	root["MusicLengthSeconds"] = out.m_fMusicLengthSeconds;

	switch (out.m_DisplayBPMType)
	{
		case Song::DISPLAY_RANDOM:
		{
			root["DisplayBpmTYpe"] = "RANDOM";
			break;
		}
		case Song::DISPLAY_SPECIFIED:
		{
			root["DisplayBpmType"] = "SPECIFIED";
			root["SpecifiedBpmMin"] = out.m_fSpecifiedBPMMin;
			root["SpecifiedBpmMax"] = out.m_fSpecifiedBPMMax;
			break;
		}
		case Song::DISPLAY_ACTUAL:
		{
			root["DisplayBpmType"] = "ACTUAL";
			root["SpecifiedBpmMin"] = out.m_fSpecifiedBPMMin;
			root["SpecifiedBpmMax"] = out.m_fSpecifiedBPMMax;
			break;
		}
	}

	Serialize( out.m_Timing, root["TimingData"] );
	JsonUtil::SerializeVectorObjects( out.m_LyricSegments, Serialize, root["LyricSegments"] );

	{
		Json::Value &root2 = root["BackgroundChanges"];
		FOREACH_BackgroundLayer( bl )
		{
			Json::Value &root3 = root2[bl];
			const vector<BackgroundChange> &vBgc = out.GetBackgroundChanges(bl);
			JsonUtil::SerializeVectorObjects( vBgc, Serialize, root3 );
		}
	}

	{
		const vector<BackgroundChange> &vBgc = out.GetForegroundChanges();
		JsonUtil::SerializeVectorObjects( vBgc, Serialize, root["ForegroundChanges"] );
	}

	JsonUtil::SerializeArrayValues( out.m_vsKeysoundFile, root["KeySounds"] );

	{
		vector<const Steps*> vpSteps;
		FOREACH_CONST( Steps*, out.GetAllSteps(), iter )
		{
			if( (*iter)->IsAutogen() )
				continue;
			vpSteps.push_back( *iter );
		}
		JsonUtil::SerializeVectorPointers<Steps>( vpSteps, Serialize, root["Steps"] );
	}

	bool bMinified = bSavingCache;
	return JsonUtil::WriteFile( root, sFile, bMinified );
}

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
