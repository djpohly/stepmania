#include "global.h"
#include "NotesLoaderBMS.h"
#include "NoteData.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameManager.h"
#include "RageFile.h"
#include "SongUtil.h"
#include "StepsUtil.h"
#include "Song.h"
#include "Steps.h"
#include "RageUtil_CharConversions.h"
#include "NoteTypes.h"
#include "NotesLoader.h"
#include "PrefsManager.h"

typedef multimap<RString, RString> NameToData_t;
typedef map<int, float> MeasureToTimeSig_t;

/* BMS encoding:	tap-hold
 * 4&8panel:	Player1		Player2
 * Left			11-51		21-61
 * Down			13-53		23-63
 * Up			15-55		25-65
 * Right		16-56		26-66
 * 
 * 6panel:		Player1
 * Left			11-51
 * Left+Up		12-52
 * Down			13-53
 * Up			14-54
 * Up+Right		15-55
 * Right		16-56
 * 
 * Notice that 15 and 25 have double meanings!  What were they thinking???
 * While reading in, use the 6 panel mapping.  After reading in, detect if 
 * only 4 notes are used.  If so, shift the Up+Right column back to the Up
 * column
 * 
 * BMSes are used for games besides dance and so we're borking up BMSes that are for popn/beat/etc.
 * 
 * popn-nine:		11-15,22-25
 * popn-five:   	13-15,21-22
 * beat-single5:	11-16
 * beat-double5:	11-16,21-26
 * beat-single7:	11-16,18-19
 * beat-double7:	11-16,18-19,21-26,28-29
 * 
 * So the magics for these are:
 * popn-nine: nothing >5, with 12, 14, 22 and/or 24
 * popn-five: nothing >5, with 14 and/or 22
 * beat-*: can't tell difference between beat-single and dance-solo
 * 	18/19 marks beat-single7, 28/29 marks beat-double7
 * 	beat-double uses 21-26.
*/

enum BmsTrack
{
	BMS_P1_KEY1 = 0,
	BMS_P1_KEY2,
	BMS_P1_KEY3,
	BMS_P1_KEY4,
	BMS_P1_KEY5,
	BMS_P1_TURN,
	BMS_P1_KEY6,
	BMS_P1_KEY7,
	BMS_P2_KEY1,
	BMS_P2_KEY2,
	BMS_P2_KEY3,
	BMS_P2_KEY4,
	BMS_P2_KEY5,
	BMS_P2_TURN,
	BMS_P2_KEY6,
	BMS_P2_KEY7,
	// max 4 simultaneous auto keysounds
	BMS_AUTO_KEYSOUND_1,
	BMS_AUTO_KEYSOUND_2,
	BMS_AUTO_KEYSOUND_3,
	BMS_AUTO_KEYSOUND_4,
	BMS_AUTO_KEYSOUND_5,
	BMS_AUTO_KEYSOUND_6,
	BMS_AUTO_KEYSOUND_7,
	BMS_AUTO_KEYSOUND_LAST,
	NUM_BMS_TRACKS,
};

const int NUM_NON_AUTO_KEYSOUND_TRACKS = BMS_AUTO_KEYSOUND_1;	
const int NUM_AUTO_KEYSOUND_TRACKS = NUM_BMS_TRACKS - NUM_NON_AUTO_KEYSOUND_TRACKS;	

static bool ConvertRawTrackToTapNote( int iRawTrack, BmsTrack &bmsTrackOut, bool &bIsHoldOut )
{
	if( iRawTrack > 40 )
	{
		bIsHoldOut = true;
		iRawTrack -= 40;
	}
	else
	{
		bIsHoldOut = false;
	}

	switch( iRawTrack )
	{
	case 1:  	bmsTrackOut = BMS_AUTO_KEYSOUND_1;	break;
	case 11:	bmsTrackOut = BMS_P1_KEY1;		break;
	case 12:	bmsTrackOut = BMS_P1_KEY2;		break;
	case 13:	bmsTrackOut = BMS_P1_KEY3;		break;
	case 14:	bmsTrackOut = BMS_P1_KEY4;		break;
	case 15:	bmsTrackOut = BMS_P1_KEY5;		break;
	case 16:	bmsTrackOut = BMS_P1_TURN;		break;
	case 18:	bmsTrackOut = BMS_P1_KEY6;		break;
	case 19:	bmsTrackOut = BMS_P1_KEY7;		break;
	case 21:	bmsTrackOut = BMS_P2_KEY1;		break;
	case 22:	bmsTrackOut = BMS_P2_KEY2;		break;
	case 23:	bmsTrackOut = BMS_P2_KEY3;		break;
	case 24:	bmsTrackOut = BMS_P2_KEY4;		break;
	case 25:	bmsTrackOut = BMS_P2_KEY5;		break;
	case 26:	bmsTrackOut = BMS_P2_TURN;		break;
	case 28:	bmsTrackOut = BMS_P2_KEY6;		break;
	case 29:	bmsTrackOut = BMS_P2_KEY7;		break;
	default:	// unknown track
		return false;
	}
	return true;
}

// Find the largest common substring at the start of both strings.
static RString FindLargestInitialSubstring( const RString &string1, const RString &string2 )
{
	// First see if the whole first string matches an appropriately-sized
	// substring of the second, then keep chopping off the last character of
	// each until they match.
	unsigned i;
	for( i = 0; i < string1.size() && i < string2.size(); ++i )
		if( string1[i] != string2[i] )
			break;

	return string1.substr( 0, i );
}

static StepsType DetermineStepsType( int iPlayer, const NoteData &nd, const RString &sPath, const int iNumNonEmptyTracks )
{
	ASSERT( NUM_BMS_TRACKS == nd.GetNumTracks() );

	switch( iPlayer )
	{
	case 1:	// "1 player"
		/* Track counts:
		 * 4 - dance 4-panel
		 * 5 - pop 5-key
		 * 6 - dance 6-panel, beat 5-key
		 * 7 - beat 7-key (scratch unused)
		 * 8 - beat 7-key
		 * 9 - popn 9-key */
		switch( iNumNonEmptyTracks ) 
		{
		case 4:		return StepsType_dance_single;
		case 5:		return StepsType_popn_five;
		case 6:
			// FIXME: There's no way to distinguish between these types.
			// They use the same tracks.  Assume it's a Beat type since they
			// are more common.
			//return StepsType_dance_solo;
			return StepsType_beat_single5;
		case 7:
		case 8:		return StepsType_beat_single7;
		case 9:		return StepsType_popn_nine;
		default:	return StepsType_Invalid;
		}
	case 2:	// couple/battle
		return StepsType_dance_couple;
	case 3:	// double
		/* Track counts:
		 * 8 - dance Double
		 * 12 - beat Double 5-key
		 * 16 - beat Double 7-key */
		switch( iNumNonEmptyTracks ) 
		{
		case 8:		return StepsType_beat_single7;
		case 12:	return StepsType_beat_double5;
		case 16:	return StepsType_beat_double7;
		case 5:		return StepsType_popn_five;
		case 9:		return StepsType_popn_nine;
		default:	return StepsType_Invalid;
		}
	default:
		LOG->UserLog( "Song file", sPath, "has an invalid #PLAYER value %d.", iPlayer );
		return StepsType_Invalid;
	}
}

static bool GetTagFromMap( const NameToData_t &mapNameToData, const RString &sName, RString &sOut )
{
	NameToData_t::const_iterator it;
	it = mapNameToData.find( sName );
	if( it == mapNameToData.end() )
		return false;

	sOut = it->second;

	return true;
}

/* Finds the longest common match for the given tag in all files. If the given tag
 * was found in at least one file, returns true; otherwise returns false. */
static bool GetCommonTagFromMapList( const vector<NameToData_t> &aBMSData, const RString &sName, RString &sOut )
{
	bool bFoundOne = false;
	for( unsigned i=0; i < aBMSData.size(); i++ )
	{
		RString sTag;
		if( !GetTagFromMap( aBMSData[i], sName, sTag ) )
			continue;

		if( !bFoundOne )
		{
			bFoundOne = true;
			sOut = sTag;
		}
		else
		{
			sOut = FindLargestInitialSubstring( sOut, sTag );
		}
	}

	return bFoundOne;
}


static float GetBeatsPerMeasure( const MeasureToTimeSig_t &sigs, int iMeasure, const MeasureToTimeSig_t &sigAdjustments )
{
	map<int, float>::const_iterator time_sig = sigs.find( iMeasure );

	float fRet = 4.0f;
	if( time_sig != sigs.end() )
		fRet *= time_sig->second;

	time_sig = sigAdjustments.find( iMeasure );
	if( time_sig != sigAdjustments.end() )
		fRet *= time_sig->second;

	return fRet;
}

static int GetMeasureStartRow( const MeasureToTimeSig_t &sigs, int iMeasureNo, const MeasureToTimeSig_t &sigAdjustments )
{
	int iRowNo = 0;
	for( int i = 0; i < iMeasureNo; ++i )
		iRowNo += BeatToNoteRow( GetBeatsPerMeasure(sigs, i, sigAdjustments) );
	return iRowNo;
}


static void SearchForDifficulty( RString sTag, Steps *pOut )
{
	sTag.MakeLower();

	// Only match "Light" in parentheses.
	if( sTag.find( "(light" ) != sTag.npos )
	{
		pOut->SetDifficulty( Difficulty_Easy );
	}
	else if( sTag.find( "another" ) != sTag.npos )
	{
		pOut->SetDifficulty( Difficulty_Hard );
	}
	else if( sTag.find( "(solo)" ) != sTag.npos )
	{
		pOut->SetDescription( "Solo" );
		pOut->SetDifficulty( Difficulty_Edit );
	}

	LOG->Trace( "Tag \"%s\" is %s", sTag.c_str(), DifficultyToString(pOut->GetDifficulty()).c_str() );
}

static bool ReadBMSFile( const RString &sPath, NameToData_t &mapNameToData )
{
	RageFile file;
	if( !file.Open(sPath) )
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened: %s", file.GetError().c_str() );
		return false;
	}

	while( !file.AtEOF() )
	{
		RString line;
		if( file.GetLine(line) == -1 )
		{
			LOG->UserLog( "Song file", sPath, "had a read error: %s", file.GetError().c_str() );
			return false;
		}

		StripCrnl( line );

		// BMS value names can be separated by a space or a colon.
		size_t iIndexOfSeparator = line.find_first_of( ": " );
		RString value_name = line.substr( 0, iIndexOfSeparator );
		RString value_data;
		if( iIndexOfSeparator != line.npos )
			value_data = line.substr( iIndexOfSeparator+1 );

		value_name.MakeLower();
		mapNameToData.insert( make_pair(value_name, value_data) );
	}

	return true;
}

enum
{
	BMS_TRACK_TIME_SIG = 2,
	BMS_TRACK_BPM = 3,
	BMS_TRACK_BPM_REF = 8,
	BMS_TRACK_STOP = 9
};

/* Time signatures are often abused to tweak sync. Real time signatures should
 * cause us to adjust the row offsets so one beat remains one beat. Fake time signatures,
 * like 1.001 or 0.999, should be removed and converted to BPM changes. This is much
 * more accurate, and prevents the whole song from being shifted off of the beat, causing
 * BeatToNoteType to be wrong.
 *
 * Evaluate each time signature, and guess which time signatures should be converted
 * to BPM changes. This isn't perfect, but errors aren't fatal. */
static void SetTimeSigAdjustments( const MeasureToTimeSig_t &sigs, Song &out, MeasureToTimeSig_t &sigAdjustmentsOut )
{
	return;
#if 0
	sigAdjustmentsOut.clear();
	
	MeasureToTimeSig_t::const_iterator it;
	for( it = sigs.begin(); it != sigs.end(); ++it )
	{
		int iMeasure = it->first;
		float fFactor = it->second;
#if 1
		static const float ValidFactors[] =
		{
			0.25f,  /* 1/4 */
			0.5f,   /* 2/4 */
			0.75f,  /* 3/4 */
			0.875f, /* 7/8 */
			1.0f,
			1.5f,   /* 6/4 */
			1.75f   /* 7/4 */
		};

		bool bValidTimeSignature = false;
		for( unsigned i = 0; i < ARRAYLEN(ValidFactors); ++i )
			if( fabsf(fFactor-ValidFactors[i]) < 0.001 )
				bValidTimeSignature = true;

		if( bValidTimeSignature )
			continue;
#else
		/* Alternate approach that I tried first: see if the ratio is sane.  However,
		 * some songs have values like "1.4", which comes out to 7/4 and is not a valid
		 * time signature. */
		// Convert the factor to a ratio, and reduce it.
		int iNum = lrintf( fFactor * 1000 ), iDen = 1000;
		int iDiv = gcd( iNum, iDen );
		iNum /= iDiv;
		iDen /= iDiv;
		
		/* Real time signatures usually come down to 1/2, 3/4, 7/8, etc. Bogus
		 * signatures that are only there to adjust sync usually look like 99/100. */
		if( iNum <= 8 && iDen <= 8 )
			continue;
#endif

		/* This time signature is bogus. Convert it to a BPM adjustment for this
		 * measure. */
		LOG->Trace("Converted time signature %f in measure %i to a BPM segment.", fFactor, iMeasure );

		/* Note that this GetMeasureStartRow will automatically include any adjustments
		 * that we've made previously in this loop; as long as we make the timing
		 * adjustment and the BPM adjustment together, everything remains consistent.
		 * Adjust sigAdjustmentsOut first, or fAdjustmentEndBeat will be wrong. */
		sigAdjustmentsOut[iMeasure] = 1.0f / fFactor;
		int iAdjustmentStartRow = GetMeasureStartRow( sigs, iMeasure, sigAdjustmentsOut );
		int iAdjustmentEndRow = GetMeasureStartRow( sigs, iMeasure+1, sigAdjustmentsOut );
		out.m_Timing.MultiplyBPMInBeatRange( iAdjustmentStartRow, iAdjustmentEndRow, 1.0f / fFactor );
	}
#endif
}

static void ReadTimeSigs( const NameToData_t &mapNameToData, MeasureToTimeSig_t &out )
{
	/* some songs have BGA starting before the music, so convertors a put weird time signature
	 * at first measure, something like #00002:1.55. that made all subsequent notes 192th.
	 * here, find the lowest measure for notes track, and make it skip the time signatures before it. */
	int iStartMeasureNo = 999;
	NameToData_t::const_iterator it;
	for( it = mapNameToData.lower_bound("#00000"); it != mapNameToData.end(); ++it )
	{
		const RString &sName = it->first;
		if( sName.size() != 6 || sName[0] != '#' || !IsAnInt( sName.substr(1,5) ) )
			continue;
		// this is step or offset data.  Looks like "#00705"
		int iMeasureNo	= StringToInt( sName.substr(1, 3) );
		int iBMSTrackNo	= StringToInt( sName.substr(4, 2) );
		RString nData = it->second;
		int totalPairs = nData.size() / 2;
		if( iBMSTrackNo != BMS_TRACK_TIME_SIG && iBMSTrackNo != 7 )
		{
			for( int i = 0; i < totalPairs; ++i )
			{
				RString sPair = nData.substr( i*2, 2 );
				if (sPair == "00")
				{
					continue;
				}
				if( iMeasureNo < iStartMeasureNo ) iStartMeasureNo = iMeasureNo;
			}
		}
	}
	for( it = mapNameToData.lower_bound("#00000"); it != mapNameToData.end(); ++it )
	{
		const RString &sName = it->first;
		if( sName.size() != 6 || sName[0] != '#' || !IsAnInt(sName.substr(1, 5)) )
			continue;

		// this is step or offset data.  Looks like "#00705"
		const RString &sData = it->second;
		int iMeasureNo	= StringToInt( sName.substr(1, 3) );
		if( iMeasureNo < iStartMeasureNo )
			continue;
		int iBMSTrackNo	= StringToInt( sName.substr(4, 2) );
		if( iBMSTrackNo == BMS_TRACK_TIME_SIG )
			out[iMeasureNo] = StringToFloat( sData );
	}
}

static const int BEATS_PER_MEASURE = 4;
static const int ROWS_PER_MEASURE = ROWS_PER_BEAT * BEATS_PER_MEASURE;

static bool SearchForKeysound( const RString &sPath, RString nDataOriginal, map<RString, int> &mapFilenameToKeysoundIndex, Song &out, int &outKeysoundIndex )
{

	// Search for memoized file names:
	{
		RString nDataToSearchFor = nDataOriginal;
		nDataToSearchFor.MakeLower();
		map<RString, int>::iterator it = mapFilenameToKeysoundIndex.find(nDataToSearchFor);
		if (it != mapFilenameToKeysoundIndex.end()) {
			outKeysoundIndex = it->second;
			return true;
		}
	}

	// FIXME: garbled song names seem to crash the app.
	// this might not be the best place to put this code.
	if( !utf8_is_valid(nDataOriginal) )
		return false;
	
	/* Due to bugs in some programs, many BMS files have a "WAV" extension
	 * on files in the BMS for files that actually have some other extension.
	 * Do a search. Don't do a wildcard search; if sData is "song.wav",
	 * we might also have "song.png", which we shouldn't match. */
	RString nData = nDataOriginal;
	if( !IsAFile(out.GetSongDir()+nData) )
	{
		const char *exts[] = { "oga", "ogg", "wav", "mp3", NULL }; // XXX: stop duplicating these everywhere
		for( unsigned i = 0; exts[i] != NULL; ++i )
		{
			RString fn = SetExtension( nData, exts[i] );
			if( IsAFile(out.GetSongDir()+fn) )
			{
				nData = fn;
				break;
			}
		}
	}
	
	if( !IsAFile(out.GetSongDir()+nData) )
	{
		LOG->UserLog( "Song file", out.GetSongDir(), "references key \"%s\" that can't be found", nData.c_str() );
		return false;
	}
	
	// Let's again search for memoized file names (we got the normalized one!):
	{
		RString nDataToSearchFor = nData;
		nDataToSearchFor.MakeLower();
		map<RString, int>::iterator it = mapFilenameToKeysoundIndex.find(nDataToSearchFor);
		if (it != mapFilenameToKeysoundIndex.end()) {
			outKeysoundIndex = it->second;
			
			{
				RString nDataToAdd = nDataOriginal;
				nDataToAdd.MakeLower();
				mapFilenameToKeysoundIndex[nDataToAdd] = outKeysoundIndex;
			}
			
			return true;
		}
	}

	// Now this is a new sample.
	out.m_vsKeysoundFile.push_back( nData );
	outKeysoundIndex = out.m_vsKeysoundFile.size() - 1;
	
	{
		RString nDataToAdd = nDataOriginal;
		nDataToAdd.MakeLower();
		mapFilenameToKeysoundIndex[nDataToAdd] = outKeysoundIndex;
	}
	
	{
		RString nDataToAdd = nData;
		nDataToAdd.MakeLower();
		mapFilenameToKeysoundIndex[nDataToAdd] = outKeysoundIndex;
	}
	
	return true;

}

static bool SearchForKeysound( const RString &sPath, RString sNoteId, const NameToData_t &mapNameToData, map<RString, int> &mapIdToKeysoundIndex, map<RString, int> &mapFilenameToKeysoundIndex, Song &out, int &outKeysoundIndex )
{

	sNoteId.MakeLower();
	{
		map<RString, int>::iterator it = mapIdToKeysoundIndex.find(sNoteId);
		if (it != mapIdToKeysoundIndex.end())
		{
			outKeysoundIndex = it->second;
			return outKeysoundIndex >= 0;
		}
	}

	RString sTagToLookFor = ssprintf( "#wav%s", sNoteId.c_str() );
	RString nDataOriginal;
	if( !GetTagFromMap( mapNameToData, sTagToLookFor, nDataOriginal ) )
	{
		LOG->UserLog( "Song file", sPath.c_str(), "has tag \"%s\" which cannot be found.", sTagToLookFor.c_str() );
		return false;
	}
	
	bool retval = SearchForKeysound(sPath, nDataOriginal, mapFilenameToKeysoundIndex, out, outKeysoundIndex);
	mapIdToKeysoundIndex[sNoteId] = outKeysoundIndex;
	return retval;
	
}

static bool LoadFromBMSFile( const RString &sPath, const NameToData_t &mapNameToData, Steps &out, Song &outSong, map<RString, int> &mapFilenameToKeysoundIndex )
{

	map<RString, int> mapIdToKeysoundIndex;
	map<int, float> mapNoteRowToBPM;
	MeasureToTimeSig_t sigAdjustments;

	LOG->Trace( "Steps::LoadFromBMSFile( '%s' )", sPath.c_str() );

	out.m_StepsType = StepsType_Invalid;

	// BMS player code.  Fill in below and use to determine StepsType.
	int iPlayer = -1;
	RString sData;
	if( GetTagFromMap( mapNameToData, "#player", sData ) )
		iPlayer = StringToInt(sData);
	if( GetTagFromMap( mapNameToData, "#playlevel", sData ) )
		out.SetMeter( StringToInt(sData) );

	NoteData ndNotes;
	ndNotes.SetNumTracks( NUM_BMS_TRACKS );

	// Read BPM
	if( GetTagFromMap(mapNameToData, "#bpm", sData) )
	{
		const float fBPM = StringToFloat( sData );

		if( fBPM > 0.0f )
		{
			BPMSegment newSeg( 0, fBPM );
			out.m_Timing.AddBPMSegment( newSeg );
			LOG->Trace( "Inserting new BPM change at beat %f, BPM %f", NoteRowToBeat(0), fBPM );
		}
		else
		{
			LOG->UserLog( "Song file", sPath.c_str(), "has an invalid BPM change at beat %f, BPM %f.",
					  NoteRowToBeat(0), fBPM );
		}
	}


	/* Read time signatures. Note that these can differ across files in the same
	 * song. */
	MeasureToTimeSig_t mapMeasureToTimeSig;
	ReadTimeSigs( mapNameToData, mapMeasureToTimeSig );

	for( NameToData_t::const_iterator it = mapNameToData.lower_bound("#00000"); it != mapNameToData.end(); ++it )
	{
		const RString &sName = it->first;
		if( sName.size() != 6 || sName[0] != '#' || !IsAnInt( sName.substr(1,5) ) )
			 continue;
		// this is step or offset data.  Looks like "#00705"
		int iMeasureNo	= atoi( sName.substr(1, 3).c_str() );
		int iBMSTrackNo	= atoi( sName.substr(4, 2).c_str() );
		int iStepIndex = GetMeasureStartRow( mapMeasureToTimeSig, iMeasureNo, sigAdjustments );
		float fBeatsPerMeasure = GetBeatsPerMeasure( mapMeasureToTimeSig, iMeasureNo, sigAdjustments );
		int iRowsPerMeasure = BeatToNoteRow( fBeatsPerMeasure );

		RString nData = it->second;
		int totalPairs = nData.size() / 2;
		for( int i = 0; i < totalPairs; ++i )
		{
			RString sPair = nData.substr( i*2, 2 );

			int iRow = iStepIndex + (i * iRowsPerMeasure) / totalPairs;
			float fBeat = NoteRowToBeat( iRow );			
			int iVal = 0;
			sscanf( sPair, "%x", &iVal );

			if (sPair == "00")
			{
				continue;
			}
			
			switch( iBMSTrackNo )
			{
			case BMS_TRACK_BPM:
				if( iVal > 0 )
				{
					mapNoteRowToBPM[ BeatToNoteRow(fBeat) ] = iVal;
					LOG->Trace( "Inserting new BPM change at beat %f, BPM %i", fBeat, iVal );
				}
				else
				{
					LOG->UserLog( "Song file", sPath.c_str(), "has an invalid BPM change at beat %f, BPM %d.",
						      fBeat, iVal );
				}
				break;

			case BMS_TRACK_BPM_REF:
			{
				RString sTagToLookFor = ssprintf( "#bpm%s", sPair.c_str() );
				RString sBPM;
				if( GetTagFromMap( mapNameToData, sTagToLookFor, sBPM ) )
				{
					float fBPM = StringToFloat( sBPM );
					mapNoteRowToBPM[ BeatToNoteRow(fBeat) ] = fBPM;
				}
				else
				{
					LOG->UserLog( "Song file", sPath.c_str(), "has tag \"%s\" which cannot be found.", sTagToLookFor.c_str() );
				}
				break;
			}
			case BMS_TRACK_STOP:
			{
				if( iVal == 0 )
				{
					break;
				}
				RString sTagToLookFor = ssprintf( "#stop%02x", iVal );
				RString sBeats;
				if( GetTagFromMap( mapNameToData, sTagToLookFor, sBeats ) )
				{
					// find the BPM at the time of this freeze
					float fBPS = out.m_Timing.GetBPMAtBeat(fBeat) / 60.0f;
					float fBeats = StringToFloat( sBeats ) / 48.0f;
					float fFreezeSecs = fBeats / fBPS;

					StopSegment newSeg( BeatToNoteRow(fBeat), fFreezeSecs );
					out.m_Timing.AddStopSegment( newSeg );
					LOG->Trace( "Inserting new Freeze at beat %f, secs %f", fBeat, newSeg.GetPause() );
				}
				else
				{
					LOG->UserLog( "Song file", sPath.c_str(), "has tag \"%s\" which cannot be found.", sTagToLookFor.c_str() );
				}
				break;
			}
			}
		}

	}
	
	for( map<int, float>::iterator it = mapNoteRowToBPM.begin(); it != mapNoteRowToBPM.end(); it ++ )
	{
		out.m_Timing.SetBPMAtRow( it->first, it->second );
	}

	// Now that we're done reading BPMs, factor out weird time signatures.
	SetTimeSigAdjustments( mapMeasureToTimeSig, outSong, sigAdjustments );

	int iHoldStarts[NUM_BMS_TRACKS];
	TapNote iHoldHeads[NUM_BMS_TRACKS];

	for( int i = 0; i < NUM_BMS_TRACKS; ++i )
	{
		iHoldStarts[i] = -1;
		iHoldHeads[i] = TAP_EMPTY;
	}

	NameToData_t::const_iterator it;
	
	bool hasBGM = false;
	for( it = mapNameToData.lower_bound("#00000"); it != mapNameToData.end(); ++it )
	{
		const RString &sName = it->first;
		if( sName.size() != 6 || sName[0] != '#' || !IsAnInt( sName.substr(1, 5) ) )
			 continue;

		// this is step or offset data.  Looks like "#00705"
		int iMeasureNo = StringToInt( sName.substr(1,3) );
		int iRawTrackNum = StringToInt( sName.substr(4,2) );
		int iRowNo = GetMeasureStartRow( mapMeasureToTimeSig, iMeasureNo, sigAdjustments );
		float fBeatsPerMeasure = GetBeatsPerMeasure( mapMeasureToTimeSig, iMeasureNo, sigAdjustments );
		const RString &sNoteData = it->second;

		vector<TapNote> vTapNotes;
		for( size_t i=0; i+1<sNoteData.size(); i+=2 )
		{
			RString sNoteId = sNoteData.substr( i, 2 );
			if( sNoteId != "00" )
			{
				TapNote tn = TAP_ORIGINAL_TAP;
				SearchForKeysound( sPath, sNoteId, mapNameToData, mapIdToKeysoundIndex, mapFilenameToKeysoundIndex, outSong, tn.iKeysoundIndex );
				vTapNotes.push_back( tn );
			}
			else
			{
				vTapNotes.push_back( TAP_EMPTY );
			}
		}

		const unsigned iNumNotesInThisMeasure = vTapNotes.size();
		for( unsigned j=0; j<iNumNotesInThisMeasure; j++ )
		{
			float fPercentThroughMeasure = (float)j/(float)iNumNotesInThisMeasure;

			int row = iRowNo + lrintf( fPercentThroughMeasure * fBeatsPerMeasure * ROWS_PER_BEAT );

			// some BMS files seem to have funky alignment, causing us to write gigantic cache files.
			// Try to correct for this by quantizing. // not always
			// row = Quantize( row, ROWS_PER_MEASURE/64 );

			BmsTrack bmsTrack;
			bool bIsHold;
			if( ConvertRawTrackToTapNote(iRawTrackNum, bmsTrack, bIsHold) )
			{
				TapNote &tn = vTapNotes[j];
				if( tn.type != TapNote::empty )
				{
					if( bmsTrack == BMS_AUTO_KEYSOUND_1 )
					{
						hasBGM = true;
						// shift the auto keysound as far right as possible
						int iLastEmptyTrack = -1;
						if( ndNotes.GetTapLastEmptyTrack(row, iLastEmptyTrack)  &&
							iLastEmptyTrack >= BMS_AUTO_KEYSOUND_1 )
						{
							tn.type = TapNote::autoKeysound;
							bmsTrack = (BmsTrack)iLastEmptyTrack;
						}
						else
						{
							// no room for this note.  Drop it.
							continue;
						}
					}
					else if( bIsHold )
					{
						tn.type = TapNote::hold_head;
						tn.subType = TapNote::hold_head_hold;
					}
				}
				// Don't bother inserting empty taps.
				if( tn.type != TapNote::empty )
					ndNotes.SetTapNote( bmsTrack, row, tn );
			}
		}
	}
	
	if (!hasBGM)
	{
		LOG->Warn("The song at %s is missing a #XXX01 tag! We're unable to load.", sPath.c_str());
		return false;
	}
	
	/* Handles hold notes like uBMPlay.
	 * Different BMS simulators support hold notes differently.
	 * See http://nvyu.net/rdm/ex.php for more info.
	 */
	for( int t=BMS_P1_KEY1; t<=BMS_P2_KEY7; t++ )
	{
		int iHoldHeadRow = -1;
		TapNote tnHoldHead;
		FOREACH_NONEMPTY_ROW_IN_TRACK( ndNotes, t, row )
		{
			TapNote tn = ndNotes.GetTapNote( t, row );
			if ( tn.type == TapNote::hold_head || tn.type == TapNote::tap )
			{
				if ( iHoldHeadRow != -1 )
				{
					// Delete head and tail, and add the hold note there.
					ndNotes.SetTapNote( t, row, TAP_EMPTY );
					ndNotes.SetTapNote( t, iHoldHeadRow, TAP_EMPTY );
					if ( iHoldHeadRow < row )
					{
						ndNotes.AddHoldNote( t, iHoldHeadRow, row, tnHoldHead );
					}
					iHoldHeadRow = -1;
				}
				else if ( tn.type == TapNote::hold_head )
				{
					// Head of the hold note, store it to find the tail.
					iHoldHeadRow = row;
					tnHoldHead = tn;
					
					// Replace with a tap note.
					tn.type = TapNote::tap;
					tn.subType = TapNote::SubType_Invalid;
					ndNotes.SetTapNote( t, row, tn );
				}
			}
		}
	}
	
	bool bTrackHasNote[NUM_NON_AUTO_KEYSOUND_TRACKS];
	ZERO( bTrackHasNote );
	
	int iLastRow = ndNotes.GetLastRow();
	for( int t=0; t<NUM_NON_AUTO_KEYSOUND_TRACKS; t++ )
	{
		for( int r=0; r<=iLastRow; r++ )
		{
			if( ndNotes.GetTapNote(t, r).type != TapNote::empty )
			{
				bTrackHasNote[t] = true;
				break;
			}
		}
	}
	
	int iNumNonEmptyTracks = 0;
	for( int t=0; t<NUM_NON_AUTO_KEYSOUND_TRACKS; t++ )
		if( bTrackHasNote[t] )
			iNumNonEmptyTracks++;
	
	out.m_StepsType = DetermineStepsType( iPlayer, ndNotes, sPath, iNumNonEmptyTracks );
	if( out.m_StepsType == StepsType_beat_single5 && GetTagFromMap( mapNameToData, "#title", sData ) )
	{
		// Hack: guess at 6-panel.

		// extract the Steps description (looks like 'Music <BASIC>')
		const size_t iOpenBracket = sData.find_first_of( "<(" );
		const size_t iCloseBracket = sData.find_first_of( ">)", iOpenBracket );

		// if there's a 6 in the description, it's probably part of "6panel" or "6-panel"
		if( sData.find('6', iOpenBracket) < iCloseBracket )
			out.m_StepsType = StepsType_dance_solo;
	}

	if( out.m_StepsType == StepsType_Invalid )
	{
		LOG->UserLog( "Song file", sPath, "has an unknown steps type" );
		return false;
	}

	int iNumNewTracks = GAMEMAN->GetStepsTypeInfo( out.m_StepsType ).iNumTracks;
	vector<int> iTransformNewToOld;
	iTransformNewToOld.resize( iNumNewTracks, -1 );

	switch( out.m_StepsType )
	{
	case StepsType_dance_single:
		iTransformNewToOld[0] = BMS_P1_KEY1;
		iTransformNewToOld[1] = BMS_P1_KEY3;
		iTransformNewToOld[2] = BMS_P1_KEY5;
		iTransformNewToOld[3] = BMS_P1_TURN;
		break;
	case StepsType_dance_double:
	case StepsType_dance_couple:
		iTransformNewToOld[0] = BMS_P1_KEY1;
		iTransformNewToOld[1] = BMS_P1_KEY3;
		iTransformNewToOld[2] = BMS_P1_KEY5;
		iTransformNewToOld[3] = BMS_P1_TURN;
		iTransformNewToOld[4] = BMS_P2_KEY1;
		iTransformNewToOld[5] = BMS_P2_KEY3;
		iTransformNewToOld[6] = BMS_P2_KEY5;
		iTransformNewToOld[7] = BMS_P2_TURN;
		break;
	case StepsType_dance_solo:
	case StepsType_beat_single5:
		// Hey! Why are these exactly the same? :-)
		iTransformNewToOld[0] = BMS_P1_KEY1;
		iTransformNewToOld[1] = BMS_P1_KEY2;
		iTransformNewToOld[2] = BMS_P1_KEY3;
		iTransformNewToOld[3] = BMS_P1_KEY4;
		iTransformNewToOld[4] = BMS_P1_KEY5;
		iTransformNewToOld[5] = BMS_P1_TURN;
		break;
	case StepsType_popn_five:
		iTransformNewToOld[0] = BMS_P1_KEY3;
		iTransformNewToOld[1] = BMS_P1_KEY4;
		iTransformNewToOld[2] = BMS_P1_KEY5;
		// fix these columns!
		iTransformNewToOld[3] = BMS_P2_KEY2;
		iTransformNewToOld[4] = BMS_P2_KEY3;
		break;
	case StepsType_popn_nine:
		iTransformNewToOld[0] = BMS_P1_KEY1; // lwhite
		iTransformNewToOld[1] = BMS_P1_KEY2; // lyellow
		iTransformNewToOld[2] = BMS_P1_KEY3; // lgreen
		iTransformNewToOld[3] = BMS_P1_KEY4; // lblue
		iTransformNewToOld[4] = BMS_P1_KEY5; // red
		// fix these columns!
		iTransformNewToOld[5] = BMS_P2_KEY2; // rblue
		iTransformNewToOld[6] = BMS_P2_KEY3; // rgreen
		iTransformNewToOld[7] = BMS_P2_KEY4; // ryellow
		iTransformNewToOld[8] = BMS_P2_KEY5; // rwhite
		break;
	case StepsType_beat_double5:
		iTransformNewToOld[0] = BMS_P1_KEY1;
		iTransformNewToOld[1] = BMS_P1_KEY2;
		iTransformNewToOld[2] = BMS_P1_KEY3;
		iTransformNewToOld[3] = BMS_P1_KEY4;
		iTransformNewToOld[4] = BMS_P1_KEY5;
		iTransformNewToOld[5] = BMS_P1_TURN;
		iTransformNewToOld[6] = BMS_P2_KEY1;
		iTransformNewToOld[7] = BMS_P2_KEY2;
		iTransformNewToOld[8] = BMS_P2_KEY3;
		iTransformNewToOld[9] = BMS_P2_KEY4;
		iTransformNewToOld[10] = BMS_P2_KEY5;
		iTransformNewToOld[11] = BMS_P2_TURN;
		break;
	case StepsType_beat_single7:
		if( !bTrackHasNote[BMS_P1_KEY7] && bTrackHasNote[BMS_P1_TURN] )
		{
			/* special case for o2mania style charts:
			 * the turntable is used for first key while the real 7th key is not used. */
			iTransformNewToOld[0] = BMS_P1_TURN;
			iTransformNewToOld[1] = BMS_P1_KEY1;
			iTransformNewToOld[2] = BMS_P1_KEY2;
			iTransformNewToOld[3] = BMS_P1_KEY3;
			iTransformNewToOld[4] = BMS_P1_KEY4;
			iTransformNewToOld[5] = BMS_P1_KEY5;
			iTransformNewToOld[6] = BMS_P1_KEY6;
			iTransformNewToOld[7] = BMS_P1_KEY7;
		}
		else
		{
			iTransformNewToOld[0] = BMS_P1_KEY1;
			iTransformNewToOld[1] = BMS_P1_KEY2;
			iTransformNewToOld[2] = BMS_P1_KEY3;
			iTransformNewToOld[3] = BMS_P1_KEY4;
			iTransformNewToOld[4] = BMS_P1_KEY5;
			iTransformNewToOld[5] = BMS_P1_KEY6;
			iTransformNewToOld[6] = BMS_P1_KEY7;
			iTransformNewToOld[7] = BMS_P1_TURN;
		}
		break;
	case StepsType_beat_double7:
		iTransformNewToOld[0] = BMS_P1_KEY1;
		iTransformNewToOld[1] = BMS_P1_KEY2;
		iTransformNewToOld[2] = BMS_P1_KEY3;
		iTransformNewToOld[3] = BMS_P1_KEY4;
		iTransformNewToOld[4] = BMS_P1_KEY5;
		iTransformNewToOld[5] = BMS_P1_KEY6;
		iTransformNewToOld[6] = BMS_P1_KEY7;
		iTransformNewToOld[7] = BMS_P1_TURN;
		iTransformNewToOld[8] = BMS_P2_KEY1;
		iTransformNewToOld[9] = BMS_P2_KEY2;
		iTransformNewToOld[10] = BMS_P2_KEY3;
		iTransformNewToOld[11] = BMS_P2_KEY4;
		iTransformNewToOld[12] = BMS_P2_KEY5;
		iTransformNewToOld[13] = BMS_P2_KEY6;
		iTransformNewToOld[14] = BMS_P2_KEY7;
		iTransformNewToOld[15] = BMS_P2_TURN;
		break;
	default:
		ASSERT_M(0, ssprintf("Invalid StepsType when parsing BMS file %s!", sPath.c_str()));
	}

	// shift all of the autokeysound tracks onto the main tracks
	// Moved here so that most sounds are inserted.
	for( int t=BMS_AUTO_KEYSOUND_1+NUM_AUTO_KEYSOUND_TRACKS-1; t>=BMS_AUTO_KEYSOUND_1; t-- )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK( ndNotes, t, row )
		{
			TapNote tn = ndNotes.GetTapNote( t, row );
			int iEmptyTrack = -1;
			for( int i=0; i<iNumNewTracks; i++ )
			{
				if ( ndNotes.GetTapNote(iTransformNewToOld[i], row) == TAP_EMPTY && !ndNotes.IsHoldNoteAtRow(iTransformNewToOld[i], row) )
				{
					iEmptyTrack = iTransformNewToOld[i];
					break;
				}
			}
			if( iEmptyTrack > -1 )
			{
				ndNotes.SetTapNote( iEmptyTrack, row, tn );
				ndNotes.SetTapNote( t, row, TAP_EMPTY );
			}
			else
			{
				LOG->UserLog( "Song file", sPath, "has too much simultaneous autokeysound tracks." );
			}
		}
	}

	NoteData noteData2;
	noteData2.SetNumTracks( iNumNewTracks );
	noteData2.LoadTransformed( ndNotes, iNumNewTracks, &*iTransformNewToOld.begin() );

	out.SetNoteData( noteData2 );

	out.TidyUpData();

	out.SetSavedToDisk( true );	// we're loading from disk, so this is by definintion already saved

	return true;
}

static void ReadGlobalTags( const NameToData_t &mapNameToData, Song &out )
{
	RString sData;
	if( GetTagFromMap(mapNameToData, "#title", sData) )
		NotesLoader::GetMainAndSubTitlesFromFullTitle( sData, out.m_sMainTitle, out.m_sSubTitle );

	GetTagFromMap( mapNameToData, "#artist", out.m_sArtist );
	GetTagFromMap( mapNameToData, "#genre", out.m_sGenre );
	GetTagFromMap( mapNameToData, "#backbmp", out.m_sBackgroundFile );
	GetTagFromMap( mapNameToData, "#wav", out.m_sMusicFile );

}

static void SlideDuplicateDifficulties( Song &p )
{
	/* BMS files have to guess the Difficulty from the meter; this is inaccurate,
	* and often leads to duplicates. Slide duplicate difficulties upwards. We
	* only do this with BMS files, since a very common bug was having *all*
	* difficulties slid upwards due to (for example) having two beginner steps.
	* We do a second pass in Song::TidyUpData to eliminate any remaining duplicates
	* after this. */
	FOREACH_ENUM( StepsType,st )
	{
		FOREACH_ENUM( Difficulty, dc )
		{
			if( dc == Difficulty_Edit )
				continue;

			vector<Steps*> vSteps;
			SongUtil::GetSteps( &p, vSteps, st, dc );

			StepsUtil::SortNotesArrayByDifficulty( vSteps );
			for( unsigned k=1; k<vSteps.size(); k++ )
			{
				Steps* pSteps = vSteps[k];

				Difficulty dc2 = min( (Difficulty)(dc+1), Difficulty_Challenge );
				pSteps->SetDifficulty( dc2 );
			}
		}
	}
}

void BMSLoader::GetApplicableFiles( const RString &sPath, vector<RString> &out )
{
	GetDirListing( sPath + RString("*.bms"), out );
	GetDirListing( sPath + RString("*.bme"), out );
	GetDirListing( sPath + RString("*.bml"), out );
}

bool BMSLoader::LoadNoteDataFromSimfile( const RString & cachePath, Steps & out )
{
	Song dummy;
	// TODO: Simplify this copy/paste from LoadFromDir.
	
	vector<NameToData_t> BMSData;
	BMSData.push_back(NameToData_t());
	ReadBMSFile(cachePath, BMSData.back());
	
	RString commonSubstring;
	GetCommonTagFromMapList( BMSData, "#title", commonSubstring );
	
	Steps *copy = dummy.CreateSteps();
	
	copy->SetDifficulty( Difficulty_Medium );
	RString sTag;
	if( GetTagFromMap( BMSData[0], "#title", sTag ) && sTag.size() != commonSubstring.size() )
	{
		sTag = sTag.substr( commonSubstring.size(), sTag.size() - commonSubstring.size() );
		sTag.MakeLower();
		
		if( sTag.find('l') != sTag.npos )
		{
			unsigned lPos = sTag.find('l');
			if( lPos > 2 && sTag.substr(lPos-2,4) == "solo" )
			{
				copy->SetDifficulty( Difficulty_Edit );
			}
			else
			{
				copy->SetDifficulty( Difficulty_Easy );
			}
		}
		else if( sTag.find('a') != sTag.npos )
			copy->SetDifficulty( Difficulty_Hard );
		else if( sTag.find('b') != sTag.npos )
			copy->SetDifficulty( Difficulty_Beginner );
	}
	if( commonSubstring == "" )
	{
		copy->SetDifficulty(Difficulty_Medium);
		RString sTag;
		if (GetTagFromMap(BMSData[0], "#title#", sTag))
			SearchForDifficulty(sTag, copy);
	}
	ReadGlobalTags( BMSData[0], dummy );
	if( commonSubstring.size() > 2 && commonSubstring[commonSubstring.size() - 2] == ' ' )
	{
		switch( commonSubstring[commonSubstring.size() - 1] )
		{
			case '[':
			case '(':
			case '<':
				commonSubstring = commonSubstring.substr(0, commonSubstring.size() - 2);
			default:
				break;
		}
	}
	map<RString, int> mapFilenameToKeysoundIndex;

	
	const bool ok = LoadFromBMSFile( cachePath, BMSData[0], *copy, dummy, mapFilenameToKeysoundIndex );
	if( ok )
	{
		out.SetNoteData(copy->GetNoteData());
	}
	return ok;
}

bool BMSLoader::LoadFromDir( const RString &sDir, Song &out )
{
	LOG->Trace( "Song::LoadFromBMSDir(%s)", sDir.c_str() );

	ASSERT( out.m_vsKeysoundFile.empty() );

	vector<RString> arrayBMSFileNames;
	GetApplicableFiles( sDir, arrayBMSFileNames );

	/* We should have at least one; if we had none, we shouldn't have been
	 * called to begin with. */
	ASSERT( arrayBMSFileNames.size() );

	// Read all BMS files.
	vector<NameToData_t> aBMSData;
	for( unsigned i=0; i<arrayBMSFileNames.size(); i++ )
	{
		aBMSData.push_back( NameToData_t() );
		ReadBMSFile( out.GetSongDir() + arrayBMSFileNames[i], aBMSData.back() );
	}

	RString commonSubstring;
	GetCommonTagFromMapList( aBMSData, "#title", commonSubstring );

	if( commonSubstring == "" )
	{
		// All bets are off; the titles don't match at all.
		// At this rate we're lucky if we even get the title right.
		LOG->UserLog( "Song", sDir, "has BMS files with inconsistent titles." );
	}

	// Create a Steps for each.
	vector<Steps*> apSteps;
	for( unsigned i=0; i<arrayBMSFileNames.size(); i++ )
		apSteps.push_back( out.CreateSteps() );

	// Now, with our fancy little substring, trim the titles and
	// figure out where each goes.
	for( unsigned i=0; i<aBMSData.size(); i++ )
	{
		Steps *pSteps = apSteps[i];
		pSteps->SetDifficulty( Difficulty_Medium );
		RString sTag;
		if( GetTagFromMap( aBMSData[i], "#title", sTag ) && sTag.size() != commonSubstring.size() )
		{
			sTag = sTag.substr( commonSubstring.size(), sTag.size() - commonSubstring.size() );
			sTag.MakeLower();

			// XXX: We should do this with filenames too, I have plenty of examples.
			// however, filenames will be trickier, as stuff at the beginning AND
			// end change per-file, so we'll need a fancier FindLargestInitialSubstring()

			// XXX: This matches (double), but I haven't seen it used. Again, MORE EXAMPLES NEEDED
			if( sTag.find('l') != sTag.npos )
			{
				unsigned lPos = sTag.find('l');
				if( lPos > 2 && sTag.substr(lPos-2,4) == "solo" )
				{
					// (solo) -- an edit, apparently (Thanks Glenn!)
					pSteps->SetDifficulty( Difficulty_Edit );
				}
				else
				{
					// Any of [L7] [L14] (LIGHT7) (LIGHT14) (LIGHT) [L] <LIGHT7> <L7>... you get the idea.
					pSteps->SetDifficulty( Difficulty_Easy );
				}
			}
			// [A] <A> (A) [ANOTHER] <ANOTHER> (ANOTHER) (ANOTHER7) Another (DP ANOTHER) (Another) -ANOTHER- [A7] [A14] etc etc etc
			else if( sTag.find('a') != sTag.npos )
				pSteps->SetDifficulty( Difficulty_Hard );
			// XXX: Can also match (double), but should match [B] or [B7]
			else if( sTag.find('b') != sTag.npos )
				pSteps->SetDifficulty( Difficulty_Beginner );
			// Other tags I've seen here include (5KEYS) (10KEYS) (7keys) (14keys) (dp) [MIX] [14] (14 Keys Mix)
			// XXX: I'm sure [MIX] means something... anyone know?
		}
	}

	if( commonSubstring == "" )
	{
		// As said before, all bets are off.
		// From here on in, it's nothing but guesswork.

		// Try to figure out the difficulty of each file.
		for( unsigned i=0; i<arrayBMSFileNames.size(); i++ )
		{
			// XXX: Is this really effective if Common Substring parsing failed?
			Steps *pSteps = apSteps[i];
			pSteps->SetDifficulty( Difficulty_Medium );
			RString sTag;
			if( GetTagFromMap( aBMSData[i], "#title", sTag ) )
				SearchForDifficulty( sTag, pSteps );
		}
	}

	/* Prefer to read global tags from a Difficulty_Medium file. These tend to
	 * have the least cruft in the #TITLE tag, so it's more likely to get a clean
	 * title. */
	int iMainDataIndex = 0;
	for( unsigned i=1; i<apSteps.size(); i++ )
		if( apSteps[i]->GetDifficulty() == Difficulty_Medium )
			iMainDataIndex = i;

	ReadGlobalTags( aBMSData[iMainDataIndex], out );
	out.m_sSongFileName = out.GetSongDir() + arrayBMSFileNames[iMainDataIndex];

	// The brackets before the difficulty are in common substring, so remove them if it's found.
	if( commonSubstring.size() > 2 && commonSubstring[commonSubstring.size() - 2] == ' ' )
	{
		switch( commonSubstring[commonSubstring.size() - 1] )
		{
		case '[':
		case '(':
		case '<':
			commonSubstring = commonSubstring.substr(0, commonSubstring.size() - 2);
		}
	}
	
	// Override what that global tag said about the title if we have a good substring.
	// Prevents clobbering and catches "MySong (7keys)" / "MySong (Another) (7keys)"
	// Also catches "MySong (7keys)" / "MySong (14keys)"
	if( commonSubstring != "" )
		NotesLoader::GetMainAndSubTitlesFromFullTitle( commonSubstring, out.m_sMainTitle, out.m_sSubTitle );

	// Now that we've parsed the keysound data, load the Steps from the rest 
	// of the .bms files.
	map<RString, int> mapFilenameToKeysoundIndex;
	for( unsigned i=0; i<arrayBMSFileNames.size(); i++ )
	{
		Steps* pNewNotes = apSteps[i];
		const bool ok = LoadFromBMSFile( out.GetSongDir() + arrayBMSFileNames[i], aBMSData[i], *pNewNotes, out, mapFilenameToKeysoundIndex );
		if( ok )
		{
			// set song's timing data to the main file.
			if( i == static_cast<unsigned>(iMainDataIndex) )
				out.m_SongTiming = pNewNotes->m_Timing;
				
			pNewNotes->SetFilename(out.GetSongDir() + arrayBMSFileNames[i]);
			out.AddSteps( pNewNotes );
		}
		else
			delete pNewNotes;
	}

	SlideDuplicateDifficulties( out );

	ConvertString( out.m_sMainTitle, "utf-8,japanese" );
	ConvertString( out.m_sArtist, "utf-8,japanese" );
	ConvertString( out.m_sGenre, "utf-8,japanese" );

	return true;
}


/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
