#include "global.h"
#include "NotesLoaderSSC.h"
#include "BackgroundUtil.h"
#include "GameManager.h"
#include "MsdFile.h"
#include "NoteTypes.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Song.h"
#include "SongManager.h"
#include "Steps.h"
#include "PrefsManager.h"

const int MAX_EDIT_STEPS_SIZE_BYTES = 60*1024; // 60 KB

bool LoadFromBGSSCChangesString( BackgroundChange &change, const RString &sBGChangeExpression )
{
	vector<RString> aBGChangeValues;
	split( sBGChangeExpression, "=", aBGChangeValues, false );
	
	aBGChangeValues.resize( min((int)aBGChangeValues.size(),11) );
	
	switch( aBGChangeValues.size() )
	{
		case 11:
			change.m_def.m_sColor2 = aBGChangeValues[10];
			change.m_def.m_sColor2.Replace( '^', ',' );
			change.m_def.m_sColor2 = RageColor::NormalizeColorString( change.m_def.m_sColor2 );
			// fall through
		case 10:
			change.m_def.m_sColor1 = aBGChangeValues[9];
			change.m_def.m_sColor1.Replace( '^', ',' );
			change.m_def.m_sColor1 = RageColor::NormalizeColorString( change.m_def.m_sColor1 );
			// fall through
		case 9:
			change.m_sTransition = aBGChangeValues[8];
			// fall through
		case 8:
			change.m_def.m_sFile2 = aBGChangeValues[7];
			// fall through
		case 7:
			change.m_def.m_sEffect = aBGChangeValues[6];
			// fall through
		case 6:
			// param 7 overrides this.
			// Backward compatibility:
			if( change.m_def.m_sEffect.empty() )
			{
				bool bLoop = atoi( aBGChangeValues[5] ) != 0;
				if( !bLoop )
					change.m_def.m_sEffect = SBE_StretchNoLoop;
			}
			// fall through
		case 5:
			// param 7 overrides this.
			// Backward compatibility:
			if( change.m_def.m_sEffect.empty() )
			{
				bool bRewindMovie = atoi( aBGChangeValues[4] ) != 0;
				if( bRewindMovie )
					change.m_def.m_sEffect = SBE_StretchRewind;
			}
			// fall through
		case 4:
			// param 9 overrides this.
			// Backward compatibility:
			if( change.m_sTransition.empty() )
				change.m_sTransition = (atoi( aBGChangeValues[3] ) != 0) ? "CrossFade" : "";
			// fall through
		case 3:
			change.m_fRate = StringToFloat( aBGChangeValues[2] );
			// fall through
		case 2:
			change.m_def.m_sFile1 = aBGChangeValues[1];
			// fall through
		case 1:
			change.m_fStartBeat = StringToFloat( aBGChangeValues[0] );
			// fall through
	}
	
	return aBGChangeValues.size() >= 2;
}

bool SSCLoader::LoadFromDir( const RString &sPath, Song &out )
{
	vector<RString> aFileNames;
	GetApplicableFiles( sPath,  aFileNames );
	
	if( aFileNames.size() > 1 )
	{
		LOG->UserLog( "Song", sPath, "has more than one SSC file. Only one SSC file is allowed per song." );
		return false;
	}
	
	ASSERT( aFileNames.size() == 1 ); // Ensure one was found entirely.
	
	return LoadFromSSCFile( sPath + aFileNames[0], out );
}

bool SSCLoader::LoadFromSSCFile( const RString &sPath, Song &out, bool bFromCache )
{
	LOG->Trace( "Song::LoadFromSSCFile(%s)", sPath.c_str() );
	
	MsdFile msd;
	if( !msd.ReadFile( sPath, true ) )
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}
	
	out.m_Timing.m_sFile = sPath; // songs still have their fallback timing.
	
	int state = GETTING_SONG_INFO;
	const unsigned values = msd.GetNumValues();
	Steps* pNewNotes;
	
	for( unsigned i = 0; i < values; i++ )
	{
		int iNumParams = msd.GetNumParams(i);
		const MsdFile::value_t &sParams = msd.GetValue(i);
		RString sValueName = sParams[0];
		sValueName.MakeUpper();
		
		switch (state)
		{
			case GETTING_SONG_INFO:
			{
				if( sValueName=="TITLE" )
				{
					out.m_sMainTitle = sParams[1];
				}
				
				else if( sValueName=="SUBTITLE" )
				{
					out.m_sSubTitle = sParams[1];
				}
				
				else if( sValueName=="ARTIST" )
				{
					out.m_sArtist = sParams[1];
				}
				
				else if( sValueName=="TITLETRANSLIT" )
				{
					out.m_sMainTitleTranslit = sParams[1];
				}
				
				else if( sValueName=="SUBTITLETRANSLIT" )
				{
					out.m_sSubTitleTranslit = sParams[1];
				}
				
				else if( sValueName=="ARTISTTRANSLIT" )
				{
					out.m_sArtistTranslit = sParams[1];
				}
				
				else if( sValueName=="GENRE" )
				{
					out.m_sGenre = sParams[1];
				}
				
				else if( sValueName=="CREDIT" )
				{
					out.m_sCredit = sParams[1];
				}
				
				else if( sValueName=="BANNER" )
				{
					out.m_sBannerFile = sParams[1];
				}
					
				else if( sValueName=="BACKGROUND" )
				{
					out.m_sBackgroundFile = sParams[1];
				}
				
				else if( sValueName=="LYRICSPATH" )
				{
					out.m_sLyricsFile = sParams[1];
				}
				
				else if( sValueName=="CDTITLE" )
				{
					out.m_sCDTitleFile = sParams[1];
				}
				
				else if( sValueName=="MUSIC" )
				{
					out.m_sMusicFile = sParams[1];
				}
				
				else if( sValueName=="INSTRUMENTTRACK" )
				{
					vector<RString> vs1;
					split( sParams[1], ",", vs1 );
					FOREACH_CONST( RString, vs1, s )
					{
						vector<RString> vs2;
						split( *s, "=", vs2 );
						if( vs2.size() >= 2 )
						{
							InstrumentTrack it = StringToInstrumentTrack( vs2[0] );
							if( it != InstrumentTrack_Invalid )
								out.m_sInstrumentTrackFile[it] = vs2[1];
						}
					}
				}
				
				else if( sValueName=="MUSICLENGTH" )
				{
					if( !bFromCache )
						continue;
					out.m_fMusicLengthSeconds = StringToFloat( sParams[1] );
				}
				
				else if( sValueName=="LASTBEATHINT" )
				{
					out.m_fSpecifiedLastBeat = StringToFloat( sParams[1] );
				}
				
				else if( sValueName=="MUSICBYTES" )
				{
					; /* ignore */
				}
				
				else if( sValueName=="SAMPLESTART" )
				{
					out.m_fMusicSampleStartSeconds = HHMMSSToSeconds( sParams[1] );
				}
				
				else if( sValueName=="SAMPLELENGTH" )
				{
					out.m_fMusicSampleLengthSeconds = HHMMSSToSeconds( sParams[1] );
				}
				
				else if( sValueName=="DISPLAYBPM" )
				{
					// #DISPLAYBPM:[xxx][xxx:xxx]|[*]; 
					if( sParams[1] == "*" )
						out.m_DisplayBPMType = Song::DISPLAY_RANDOM;
					else 
					{
						out.m_DisplayBPMType = Song::DISPLAY_SPECIFIED;
						out.m_fSpecifiedBPMMin = StringToFloat( sParams[1] );
						if( sParams[2].empty() )
							out.m_fSpecifiedBPMMax = out.m_fSpecifiedBPMMin;
						else
							out.m_fSpecifiedBPMMax = StringToFloat( sParams[2] );
					}
				}
				
				else if( sValueName=="SELECTABLE" )
				{
					if(!stricmp(sParams[1],"YES"))
						out.m_SelectionDisplay = out.SHOW_ALWAYS;
					else if(!stricmp(sParams[1],"NO"))
						out.m_SelectionDisplay = out.SHOW_NEVER;
					// ROULETTE from 3.9 is no longer in use.
					else if(!stricmp(sParams[1],"ROULETTE"))
						out.m_SelectionDisplay = out.SHOW_ALWAYS;
					/* The following two cases are just fixes to make sure simfiles that
					 * used 3.9+ features are not excluded here */
					else if(!stricmp(sParams[1],"ES") || !stricmp(sParams[1],"OMES"))
						out.m_SelectionDisplay = out.SHOW_ALWAYS;
					else if( atoi(sParams[1]) > 0 )
						out.m_SelectionDisplay = out.SHOW_ALWAYS;
					else
						LOG->UserLog( "Song file", sPath, "has an unknown #SELECTABLE value, \"%s\"; ignored.", sParams[1].c_str() );
				}
				
				else if( sValueName.Left(strlen("BGCHANGES"))=="BGCHANGES" || sValueName=="ANIMATIONS" )
				{
					BackgroundLayer iLayer = BACKGROUND_LAYER_1;
					if( sscanf(sValueName, "BGCHANGES%d", &*ConvertValue<int>(&iLayer)) == 1 )
						enum_add(iLayer, -1);	// #BGCHANGES2 = BACKGROUND_LAYER_2
					
					bool bValid = iLayer>=0 && iLayer<NUM_BackgroundLayer;
					if( !bValid )
					{
						LOG->UserLog( "Song file", sPath, "has a #BGCHANGES tag \"%s\" that is out of range.", sValueName.c_str() );
					}
					else
					{
						vector<RString> aBGChangeExpressions;
						split( sParams[1], ",", aBGChangeExpressions );
						
						for( unsigned b=0; b<aBGChangeExpressions.size(); b++ )
						{
							BackgroundChange change;
							if( LoadFromBGSSCChangesString( change, aBGChangeExpressions[b] ) )
								out.AddBackgroundChange( iLayer, change );
						}
					}
				}
				
				else if( sValueName=="FGCHANGES" )
				{
					vector<RString> aFGChangeExpressions;
					split( sParams[1], ",", aFGChangeExpressions );
					
					for( unsigned b=0; b<aFGChangeExpressions.size(); b++ )
					{
						BackgroundChange change;
						if( LoadFromBGSSCChangesString( change, aFGChangeExpressions[b] ) )
							out.AddForegroundChange( change );
					}
				}
				
				else if( sValueName=="KEYSOUNDS" )
				{
					split( sParams[1], ",", out.m_vsKeysoundFile );
				}
				
				// Attacks loaded from file
				else if( sValueName=="ATTACKS" )
				{
					// Build the RString vector here so we can write it to file again later
					for( unsigned s=1; s < sParams.params.size(); ++s )
						out.m_sAttackString.push_back( sParams[s] );
					
					Attack attack;
					float end = -9999;
					
					for( unsigned j=1; j < sParams.params.size(); ++j )
					{
						vector<RString> sBits;
						split( sParams[j], "=", sBits, false );
						
						// Need an identifer and a value for this to work
						if( sBits.size() < 2 )
							continue;
						
						TrimLeft( sBits[0] );
						TrimRight( sBits[0] );
						
						if( !sBits[0].CompareNoCase("TIME") )
							attack.fStartSecond = strtof( sBits[1], NULL );
						else if( !sBits[0].CompareNoCase("LEN") )
							attack.fSecsRemaining = strtof( sBits[1], NULL );
						else if( !sBits[0].CompareNoCase("END") )
							end = strtof( sBits[1], NULL );
						else if( !sBits[0].CompareNoCase("MODS") )
						{
							attack.sModifiers = sBits[1];
							
							if( end != -9999 )
							{
								attack.fSecsRemaining = end - attack.fStartSecond;
								end = -9999;
							}
							
							if( attack.fSecsRemaining < 0.0f )
								attack.fSecsRemaining = 0.0f;
							
							out.m_Attacks.push_back( attack );
						}
					}
				}
				
				/*
				 * Below are the song based timings that
				 * should only be used if the steps do
				 * not have their own timing.
				 */
				else if( sValueName=="OFFSET" )
				{
					out.m_Timing.m_fBeat0OffsetInSeconds = StringToFloat( sParams[1] );
				}
				else if( sValueName=="STOPS" )
				{
					vector<RString> arrayFreezeExpressions;
					split( sParams[1], ",", arrayFreezeExpressions );
					
					for( unsigned f=0; f<arrayFreezeExpressions.size(); f++ )
					{
						vector<RString> arrayFreezeValues;
						split( arrayFreezeExpressions[f], "=", arrayFreezeValues );
						if( arrayFreezeValues.size() != 2 )
						{
							// XXX: Hard to tell which file caused this.
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #%s value \"%s\" (must have exactly one '='), ignored.",
								     sValueName.c_str(), arrayFreezeExpressions[f].c_str() );
							continue;
						}
						
						const float fFreezeBeat = StringToFloat( arrayFreezeValues[0] );
						const float fFreezeSeconds = StringToFloat( arrayFreezeValues[1] );
						StopSegment new_seg( BeatToNoteRow(fFreezeBeat), fFreezeSeconds );
						
						if(fFreezeSeconds > 0.0f)
						{
							// LOG->Trace( "Adding a freeze segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );
							out.m_Timing.AddStopSegment( new_seg );
						}
						else
						{
							// negative stops (hi JS!) -aj
							if( PREFSMAN->m_bQuirksMode )
							{
								// LOG->Trace( "Adding a negative freeze segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );
								out.m_Timing.AddStopSegment( new_seg );
							}
							else
								LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid stop at beat %f, length %f.", fFreezeBeat, fFreezeSeconds );
						}
					}
				}
				else if( sValueName=="DELAYS" )
				{
					vector<RString> arrayDelayExpressions;
					split( sParams[1], ",", arrayDelayExpressions );
					
					for( unsigned f=0; f<arrayDelayExpressions.size(); f++ )
					{
						vector<RString> arrayDelayValues;
						split( arrayDelayExpressions[f], "=", arrayDelayValues );
						if( arrayDelayValues.size() != 2 )
						{
							// XXX: Hard to tell which file caused this.
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #%s value \"%s\" (must have exactly one '='), ignored.",
								     sValueName.c_str(), arrayDelayExpressions[f].c_str() );
							continue;
						}
						
						const float fFreezeBeat = StringToFloat( arrayDelayValues[0] );
						const float fFreezeSeconds = StringToFloat( arrayDelayValues[1] );
						
						StopSegment new_seg( BeatToNoteRow(fFreezeBeat), fFreezeSeconds, true );
						
						// LOG->Trace( "Adding a delay segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );
						
						if(fFreezeSeconds > 0.0f)
							out.m_Timing.AddStopSegment( new_seg );
						else
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid delay at beat %f, length %f.", fFreezeBeat, fFreezeSeconds );
					}
				}
				
				else if( sValueName=="BPMS" )
				{
					vector<RString> arrayBPMChangeExpressions;
					split( sParams[1], ",", arrayBPMChangeExpressions );
					
					for( unsigned b=0; b<arrayBPMChangeExpressions.size(); b++ )
					{
						vector<RString> arrayBPMChangeValues;
						split( arrayBPMChangeExpressions[b], "=", arrayBPMChangeValues );
						// XXX: Hard to tell which file caused this.
						if( arrayBPMChangeValues.size() != 2 )
						{
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #%s value \"%s\" (must have exactly one '='), ignored.",
								     sValueName.c_str(), arrayBPMChangeExpressions[b].c_str() );
							continue;
						}
						
						const float fBeat = StringToFloat( arrayBPMChangeValues[0] );
						const float fNewBPM = StringToFloat( arrayBPMChangeValues[1] );
						
						
						
						if(fNewBPM > 0.0f)
							out.m_Timing.AddBPMSegment( BPMSegment(BeatToNoteRow(fBeat), fNewBPM) );
						else
						{
							out.m_Timing.m_bHasNegativeBpms = true;
							// only add Negative BPMs in quirks mode -aj
							if( PREFSMAN->m_bQuirksMode )
								out.m_Timing.AddBPMSegment( BPMSegment(BeatToNoteRow(fBeat), fNewBPM) );
							else
								LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid BPM change at beat %f, BPM %f.", fBeat, fNewBPM );
						}
					}
				}
				
				else if( sValueName=="TIMESIGNATURES" )
				{
					vector<RString> vs1;
					split( sParams[1], ",", vs1 );
					
					FOREACH_CONST( RString, vs1, s1 )
					{
						vector<RString> vs2;
						split( *s1, "=", vs2 );
						
						if( vs2.size() < 3 )
						{
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid time signature change with %i values.", (int)vs2.size() );
							continue;
						}
						
						const float fBeat = StringToFloat( vs2[0] );
						
						TimeSignatureSegment seg;
						seg.m_iStartRow = BeatToNoteRow(fBeat);
						seg.m_iNumerator = atoi( vs2[1] ); 
						seg.m_iDenominator = atoi( vs2[2] ); 
						
						if( fBeat < 0 )
						{
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid time signature change with beat %f.", fBeat );
							continue;
						}
						
						if( seg.m_iNumerator < 1 )
						{
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid time signature change with beat %f, iNumerator %i.", fBeat, seg.m_iNumerator );
							continue;
						}
						
						if( seg.m_iDenominator < 1 )
						{
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid time signature change with beat %f, iDenominator %i.", fBeat, seg.m_iDenominator );
							continue;
						}
						
						out.m_Timing.AddTimeSignatureSegment( seg );
					}
				}
				
				else if( sValueName=="TICKCOUNTS" )
				{
					vector<RString> arrayTickcountExpressions;
					split( sParams[1], ",", arrayTickcountExpressions );
					
					for( unsigned f=0; f<arrayTickcountExpressions.size(); f++ )
					{
						vector<RString> arrayTickcountValues;
						split( arrayTickcountExpressions[f], "=", arrayTickcountValues );
						if( arrayTickcountValues.size() != 2 )
						{
							// XXX: Hard to tell which file caused this.
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #%s value \"%s\" (must have exactly one '='), ignored.",
								     sValueName.c_str(), arrayTickcountExpressions[f].c_str() );
							continue;
						}
						
						const float fTickcountBeat = StringToFloat( arrayTickcountValues[0] );
						const int iTicks = atoi( arrayTickcountValues[1] );
						TickcountSegment new_seg( BeatToNoteRow(fTickcountBeat), iTicks );
						
						if(iTicks >= 1 && iTicks <= ROWS_PER_BEAT ) // Constants
						{
							// LOG->Trace( "Adding a tickcount segment: beat: %f, ticks = %d", fTickcountBeat, iTicks );
							out.m_Timing.AddTickcountSegment( new_seg );
						}
						else
						{
							LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid tickcount at beat %f, ticks %d.", fTickcountBeat, iTicks );
						}
					}
				}
				
				/*
				 * The following are cache tags.
				 * Never fill their values directly:
				 * only from the cached version.
				 */
				else if( sValueName=="FIRSTBEAT" )
				{
					if( bFromCache )
						out.m_fFirstBeat = StringToFloat( sParams[1] );
				}
				
				else if( sValueName=="LASTBEAT" )
				{
					if( bFromCache )
						out.m_fLastBeat = StringToFloat( sParams[1] );
				}
				
				else if( sValueName=="SONGFILENAME" )
				{
					if( bFromCache )
						out.m_sSongFileName = sParams[1];
				}
				
				else if( sValueName=="HASMUSIC" )
				{
					if( bFromCache )
						out.m_bHasMusic = atoi( sParams[1] ) != 0;
				}
				
				else if( sValueName=="HASBANNER" )
				{
					if( bFromCache )
						out.m_bHasBanner = atoi( sParams[1] ) != 0;
				}
				
				// This tag will get us to the next section.
				else if( sValueName=="NOTEDATA" )
				{
					state = GETTING_STEP_INFO;
					pNewNotes = new Steps;
				}
				break;
			}
			case GETTING_STEP_INFO:
			{
				if( sValueName=="STEPSTYPE" )
				{
					pNewNotes->m_StepsType = GAMEMAN->StringToStepsType( sValueName );
				}
				
				else if( sValueName=="DESCRIPTION" )
				{
					pNewNotes->SetDescription( sValueName );
				}
				
				else if( sValueName=="DIFFICULTY" )
				{
					pNewNotes->SetDifficulty( DwiCompatibleStringToDifficulty( sValueName ) );
				}
				
				else if( sValueName=="METER" )
				{
					pNewNotes->SetMeter( atoi(sValueName) );
				}
				
				else if( sValueName=="RADARVALUES" )
				{
					; // skip for now: not happy with present implementation.
				}
				
				else if( sValueName=="CREDIT" )
				{
					; // Not implemented yet, but should be.
				}
				
				else if( sValueName=="NOTES" )
				{
					; // Copy timing from song to steps.
				}
				else if( sValueName=="BPMS" )
				{
					state = GETTING_STEP_TIMING_INFO;
					; // put BPM info here.
				}
				break;
			}
			case GETTING_STEP_TIMING_INFO:
			{
				break;
			}
		}
	}
	return false;
}

void SSCLoader::GetApplicableFiles( const RString &sPath, vector<RString> &out )
{
	GetDirListing( sPath + RString("*.ssc"), out );
}

/*
 * (c) 2011 spinal shark collective
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
