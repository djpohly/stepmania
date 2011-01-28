#include "global.h"
#include <cerrno>
#include <cstring>
#include "NotesWriterSM.h"
#include "BackgroundUtil.h"
#include "Foreach.h"
#include "GameManager.h"
#include "LocalizedString.h"
#include "NoteTypes.h"
#include "Profile.h"
#include "ProfileManager.h"
#include "RageFile.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Song.h"
#include "Steps.h"

static RString BackgroundChangeToString( const BackgroundChange &bgc )
{
	// TODO: Technically we need to double-escape the filename (because it might contain '=') and then
	// unescape the value returned by the MsdFile.
	RString s = ssprintf( 
		"%.3f=%s=%.3f=%d=%d=%d=%s=%s=%s=%s=%s", 
		bgc.m_fStartBeat, 
		SmEscape(bgc.m_def.m_sFile1).c_str(), 
		bgc.m_fRate, 
		bgc.m_sTransition == SBT_CrossFade,		// backward compat
		bgc.m_def.m_sEffect == SBE_StretchRewind, 	// backward compat
		bgc.m_def.m_sEffect != SBE_StretchNoLoop, 	// backward compat
		bgc.m_def.m_sEffect.c_str(), 
		bgc.m_def.m_sFile2.c_str(), 
		bgc.m_sTransition.c_str(),
		SmEscape(RageColor::NormalizeColorString(bgc.m_def.m_sColor1)).c_str(),
		SmEscape(RageColor::NormalizeColorString(bgc.m_def.m_sColor2)).c_str()
		);
	return s;
}

static void WriteGlobalTags( RageFile &f, const Song &out )
{
	//f.PutLine( ssprintf( "#SMVERSION:;", SmEscape(RString("ssc-v1.0")).c_str() ) );
	f.PutLine( ssprintf( "#TITLE:%s;", SmEscape(out.m_sMainTitle).c_str() ) );
	f.PutLine( ssprintf( "#SUBTITLE:%s;", SmEscape(out.m_sSubTitle).c_str() ) );
	f.PutLine( ssprintf( "#ARTIST:%s;", SmEscape(out.m_sArtist).c_str() ) );
	f.PutLine( ssprintf( "#TITLETRANSLIT:%s;", SmEscape(out.m_sMainTitleTranslit).c_str() ) );
	f.PutLine( ssprintf( "#SUBTITLETRANSLIT:%s;", SmEscape(out.m_sSubTitleTranslit).c_str() ) );
	f.PutLine( ssprintf( "#ARTISTTRANSLIT:%s;", SmEscape(out.m_sArtistTranslit).c_str() ) );
	f.PutLine( ssprintf( "#GENRE:%s;", SmEscape(out.m_sGenre).c_str() ) );
	f.PutLine( ssprintf( "#CREDIT:%s;", SmEscape(out.m_sCredit).c_str() ) );
	f.PutLine( ssprintf( "#BANNER:%s;", SmEscape(out.m_sBannerFile).c_str() ) );
	f.PutLine( ssprintf( "#BACKGROUND:%s;", SmEscape(out.m_sBackgroundFile).c_str() ) );
	f.PutLine( ssprintf( "#LYRICSPATH:%s;", SmEscape(out.m_sLyricsFile).c_str() ) );
	f.PutLine( ssprintf( "#CDTITLE:%s;", SmEscape(out.m_sCDTitleFile).c_str() ) );
	f.PutLine( ssprintf( "#MUSIC:%s;", SmEscape(out.m_sMusicFile).c_str() ) );
	
	{
		vector<RString> vs;
		FOREACH_ENUM( InstrumentTrack, it )
			if( out.HasInstrumentTrack(it) )
				vs.push_back( InstrumentTrackToString(it) + "=" + out.m_sInstrumentTrackFile[it] );
		if( !vs.empty() )
		{
			RString s = join( ",", vs );
			f.PutLine( "#INSTRUMENTTRACK:" + s + ";\n" );
		}
	}
	f.PutLine( ssprintf( "#OFFSET:%.6f;", out.m_Timing.m_fBeat0OffsetInSeconds ) );
	f.PutLine( ssprintf( "#SAMPLESTART:%.6f;", out.m_fMusicSampleStartSeconds ) );
	f.PutLine( ssprintf( "#SAMPLELENGTH:%.6f;", out.m_fMusicSampleLengthSeconds ) );
	if( out.m_fSpecifiedLastBeat > 0 )
		f.PutLine( ssprintf("#LASTBEATHINT:%.6f;", out.m_fSpecifiedLastBeat) );

	f.Write( "#SELECTABLE:" );
	switch(out.m_SelectionDisplay)
	{
		default: ASSERT(0); // fall through
		case Song::SHOW_ALWAYS:	f.Write( "YES" );		break;
		//case Song::SHOW_NONSTOP:	f.Write( "NONSTOP" );	break;
		case Song::SHOW_NEVER:		f.Write( "NO" );		break;
	}
	f.PutLine( ";" );

	switch( out.m_DisplayBPMType )
	{
	case Song::DISPLAY_ACTUAL:
		// write nothing
		break;
	case Song::DISPLAY_SPECIFIED:
		if( out.m_fSpecifiedBPMMin == out.m_fSpecifiedBPMMax )
			f.PutLine( ssprintf( "#DISPLAYBPM:%.6f;", out.m_fSpecifiedBPMMin ) );
		else
			f.PutLine( ssprintf( "#DISPLAYBPM:%.6f:%.6f;", out.m_fSpecifiedBPMMin, out.m_fSpecifiedBPMMax ) );
		break;
	case Song::DISPLAY_RANDOM:
		f.PutLine( ssprintf( "#DISPLAYBPM:*;" ) );
		break;
	}


	f.Write( "#BPMS:" );
	for( unsigned i=0; i<out.m_Timing.m_BPMSegments.size(); i++ )
	{
		const BPMSegment &bs = out.m_Timing.m_BPMSegments[i];

		f.PutLine( ssprintf( "%.6f=%.6f", NoteRowToBeat(bs.m_iStartRow), bs.GetBPM() ) );
		if( i != out.m_Timing.m_BPMSegments.size()-1 )
			f.Write( "," );
	}
	f.PutLine( ";" );

	f.Write( "#STOPS:" );
	for( unsigned i=0; i<out.m_Timing.m_StopSegments.size(); i++ )
	{
		const StopSegment &fs = out.m_Timing.m_StopSegments[i];

		if(!fs.m_bDelay)
		{
			f.PutLine( ssprintf( "%.6f=%.6f", NoteRowToBeat(fs.m_iStartRow), fs.m_fStopSeconds ) );
			if( i != out.m_Timing.m_StopSegments.size()-1 )
				f.Write( "," );
		}
	}
	f.PutLine( ";" );

	f.Write( "#DELAYS:" );
	for( unsigned i=0; i<out.m_Timing.m_StopSegments.size(); i++ )
	{
		const StopSegment &fs = out.m_Timing.m_StopSegments[i];

		if( fs.m_bDelay )
		{
			f.PutLine( ssprintf( "%.6f=%.6f", NoteRowToBeat(fs.m_iStartRow), fs.m_fStopSeconds ) );
			if( i != out.m_Timing.m_StopSegments.size()-1 )
				f.Write( "," );
		}
	}
	f.PutLine( ";" );

	/*
	f.Write( "#WARPS:" );
	for( unsigned i=0; i<out.m_Timing.m_WarpSegments.size(); i++ )
	{
		const WarpSegment &ws = out.m_Timing.m_WarpSegments[i];

		f.PutLine( ssprintf( "%.6f=%.6f", NoteRowToBeat(ws.m_iStartRow), ws.m_fWarpBeats ) );
		if( i != out.m_Timing.m_WarpSegments.size()-1 )
			f.Write( "," );
	}
	f.PutLine( ";" );
	*/

	ASSERT( !out.m_Timing.m_vTimeSignatureSegments.empty() );
	f.Write( "#TIMESIGNATURES:" );
	FOREACH_CONST( TimeSignatureSegment, out.m_Timing.m_vTimeSignatureSegments, iter )
	{
		f.PutLine( ssprintf( "%.6f=%d=%d", NoteRowToBeat(iter->m_iStartRow), iter->m_iNumerator, iter->m_iDenominator ) );
		vector<TimeSignatureSegment>::const_iterator iter2 = iter;
		iter2++;
		if( iter2 != out.m_Timing.m_vTimeSignatureSegments.end() )
			f.Write( "," );
	}
	f.PutLine( ";" );

	ASSERT( !out.m_Timing.m_TickcountSegments.empty() );
	f.Write( "#TICKCOUNTS:" );
	for( unsigned i=0; i<out.m_Timing.m_TickcountSegments.size(); i++ )
	{
		const TickcountSegment &ts = out.m_Timing.m_TickcountSegments[i];
		
		f.PutLine( ssprintf( "%.6f=%d", NoteRowToBeat(ts.m_iStartRow), ts.m_iTicks ) );
		if( i != out.m_Timing.m_TickcountSegments.size()-1 )
			f.Write( "," );
	}
	f.PutLine( ";" );
	
	FOREACH_BackgroundLayer( b )
	{
		if( b==0 )
			f.Write( "#BGCHANGES:" );
		else if( out.GetBackgroundChanges(b).empty() )
			continue;	// skip
		else
			f.Write( ssprintf("#BGCHANGES%d:", b+1) );

		FOREACH_CONST( BackgroundChange, out.GetBackgroundChanges(b), bgc )
			f.PutLine( BackgroundChangeToString(*bgc)+"," );

		/* If there's an animation plan at all, add a dummy "-nosongbg-" tag to indicate that
		 * this file doesn't want a song BG entry added at the end.  See SMLoader::TidyUpData.
		 * This tag will be removed on load.  Add it at a very high beat, so it won't cause
		 * problems if loaded in older versions. */
		if( b==0 && !out.GetBackgroundChanges(b).empty() )
			f.PutLine( "99999=-nosongbg-=1.000=0=0=0 // don't automatically add -songbackground-" );
		f.PutLine( ";" );
	}

	if( out.GetForegroundChanges().size() )
	{
		f.Write( "#FGCHANGES:" );
		FOREACH_CONST( BackgroundChange, out.GetForegroundChanges(), bgc )
		{
			f.PutLine( BackgroundChangeToString(*bgc)+"," );
		}
		f.PutLine( ";" );
	}

	f.Write( "#KEYSOUNDS:" );
	for( unsigned i=0; i<out.m_vsKeysoundFile.size(); i++ )
	{
		f.Write( out.m_vsKeysoundFile[i] );
		if( i != out.m_vsKeysoundFile.size()-1 )
			f.Write( "," );
	}
	f.PutLine( ";" );

	f.Write( "#ATTACKS:" );
	for( unsigned a=0; a < out.m_sAttackString.size(); a++ )
	{
		RString sData = out.m_sAttackString[a];
		f.Write( ssprintf( "%s", sData.c_str() ) );

		if( a != (out.m_sAttackString.size() - 1) )
			f.Write( ":" );	// Not the end, so write a divider ':'
	}
	f.PutLine( ";" );
}

static RString JoinLineList( vector<RString> &lines )
{
	for( unsigned i = 0; i < lines.size(); ++i )
		TrimRight( lines[i] );

	/* Skip leading blanks. */
	unsigned j = 0;
	while( j < lines.size() && lines.size() == 0 )
		++j;

	return join( "\r\n", lines.begin()+j, lines.end() );
}

static RString GetSMNotesTag( const Song &song, const Steps &in )
{
	vector<RString> lines;

	lines.push_back( "" );
	// Escape to prevent some clown from making a comment of "\r\n;"
	lines.push_back( ssprintf("//---------------%s - %s----------------",
		GAMEMAN->GetStepsTypeInfo(in.m_StepsType).szName, SmEscape(in.GetDescription()).c_str()) );
	lines.push_back( song.m_vsKeysoundFile.empty() ? "#NOTES:" : "#NOTES2:" );
	lines.push_back( ssprintf( "     %s:", GAMEMAN->GetStepsTypeInfo(in.m_StepsType).szName ) );
	lines.push_back( ssprintf( "     %s:", SmEscape(in.GetDescription()).c_str() ) );
	lines.push_back( ssprintf( "     %s:", DifficultyToString(in.GetDifficulty()).c_str() ) );
	lines.push_back( ssprintf( "     %d:", in.GetMeter() ) );
	
	vector<RString> asRadarValues;
	FOREACH_PlayerNumber( pn )
	{
		const RadarValues &rv = in.GetRadarValues( pn );
		FOREACH_ENUM( RadarCategory, rc )
			asRadarValues.push_back( ssprintf("%.3f", rv[rc]) );
	}
	lines.push_back( ssprintf( "     %s:", join(",",asRadarValues).c_str() ) );

	RString sNoteData;
	in.GetSMNoteData( sNoteData );

	split( sNoteData, "\n", lines, true );
	lines.push_back( ";" );

	return JoinLineList( lines );
}

bool NotesWriterSM::Write( RString sPath, const Song &out )
{
	int flags = RageFile::WRITE;

	flags |= RageFile::SLOW_FLUSH;

	RageFile f;
	if( !f.Open( sPath, flags ) )
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened for writing: %s", f.GetError().c_str() );
		return false;
	}

	WriteGlobalTags( f, out );
	

	// Save specified Steps to this file
	const vector<Steps*>& vpStepsToSave = out.GetAllSteps();
	FOREACH_CONST( Steps*, vpStepsToSave, s ) 
	{
		const Steps* pSteps = *s;
		RString sTag = GetSMNotesTag( out, *pSteps );
		f.PutLine( sTag );
	}
	if( f.Flush() == -1 )
		return false;

	return true;
}

void NotesWriterSM::GetEditFileContents( const Song *pSong, const Steps *pSteps, RString &sOut )
{
	sOut = "";
	RString sDir = pSong->GetSongDir();

	// "Songs/foo/bar"; strip off "Songs/".
	vector<RString> asParts;
	split( sDir, "/", asParts );
	if( asParts.size() )
		sDir = join( "/", asParts.begin()+1, asParts.end() );
	sOut += ssprintf( "#SONG:%s;\r\n", sDir.c_str() );
	sOut += GetSMNotesTag( *pSong, *pSteps );
}

RString NotesWriterSM::GetEditFileName( const Song *pSong, const Steps *pSteps )
{
	/* Try to make a unique name. This isn't guaranteed. Edit descriptions are
	 * case-sensitive, filenames on disk are usually not, and we decimate certain
	 * characters for FAT filesystems. */
	RString sFile = pSong->GetTranslitFullTitle() + " - " + pSteps->GetDescription();

	// HACK:
	if( pSteps->m_StepsType == StepsType_dance_double )
		sFile += " (doubles)";

	sFile += ".edit";

	MakeValidFilename( sFile );
	return sFile;
}

static LocalizedString DESTINATION_ALREADY_EXISTS	("NotesWriterSM", "Error renaming file.  Destination file '%s' already exists.");
static LocalizedString ERROR_WRITING_FILE		("NotesWriterSM", "Error writing file '%s'.");
bool NotesWriterSM::WriteEditFileToMachine( const Song *pSong, Steps *pSteps, RString &sErrorOut )
{
	RString sDir = PROFILEMAN->GetProfileDir( ProfileSlot_Machine ) + EDIT_STEPS_SUBDIR;

	RString sPath = sDir + GetEditFileName(pSong,pSteps);

	// Check to make sure that we're not clobering an existing file before opening.
	bool bFileNameChanging = 
		pSteps->GetSavedToDisk()  && 
		pSteps->GetFilename() != sPath;
	if( bFileNameChanging  &&  DoesFileExist(sPath) )
	{
		sErrorOut = ssprintf( DESTINATION_ALREADY_EXISTS.GetValue(), sPath.c_str() );
		return false;
	}

	RageFile f;
	if( !f.Open(sPath, RageFile::WRITE | RageFile::SLOW_FLUSH) )
	{
		sErrorOut = ssprintf( ERROR_WRITING_FILE.GetValue(), sPath.c_str() );
		return false;
	}

	RString sTag;
	GetEditFileContents( pSong, pSteps, sTag );
	if( f.PutLine(sTag) == -1 || f.Flush() == -1 )
	{
		sErrorOut = ssprintf( ERROR_WRITING_FILE.GetValue(), sPath.c_str() );
		return false;
	}

	/* If the file name of the edit has changed since the last save, then delete the old
	 * file after saving the new one. If we delete it first, then we'll lose data on error. */

	if( bFileNameChanging )
		FILEMAN->Remove( pSteps->GetFilename() );
	pSteps->SetFilename( sPath );

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
