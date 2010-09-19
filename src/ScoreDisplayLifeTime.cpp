#include "global.h"
#include "ScoreDisplayLifeTime.h"
// #include "LifeMeterTime.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "PlayerState.h"
#include "PlayerStageStats.h"
#include "CommonMetrics.h"
#include "ActorUtil.h"

// static const CString GAIN_LIFE_COMMAND_NAME = "GainLife";

ScoreDisplayLifeTime::ScoreDisplayLifeTime()
{
	LOG->Trace( "ScoreDisplayLifeTime::ScoreDisplayLifeTime()" );
}

void ScoreDisplayLifeTime::Init( const PlayerState* pPlayerState, const PlayerStageStats* pPlayerStageStats ) 
{
	ScoreDisplay::Init( pPlayerState, pPlayerStageStats );

	const RString sType = "ScoreDisplayLifeTime";

	m_sprFrame.Load( THEME->GetPathG(sType,"frame") );
	m_sprFrame->SetName( "Frame" );
	this->AddChild( m_sprFrame );
	ActorUtil::LoadAllCommandsAndOnCommand( m_sprFrame, sType );

	m_textTimeRemaining.LoadFromFont( THEME->GetPathF(sType, "TimeRemaining") );
	m_textTimeRemaining.SetName( "TimeRemaining" );
	this->AddChild( &m_textTimeRemaining );
	ActorUtil::LoadAllCommandsAndOnCommand( m_textTimeRemaining, sType );
	
	m_textDeltaSeconds.LoadFromFont( THEME->GetPathF(sType,"DeltaSeconds") );
	m_textDeltaSeconds.SetName( "DeltaSeconds" );
	this->AddChild( &m_textDeltaSeconds );
	ActorUtil::LoadAllCommandsAndOnCommand( m_textDeltaSeconds, sType );

	FOREACH_ENUM( TapNoteScore, tns )
	{
		const RString &sCommand = TapNoteScoreToString(tns);
		if( !m_textDeltaSeconds.HasCommand( sCommand ) )
			ActorUtil::LoadCommand( m_textDeltaSeconds, sType, sCommand );
	}
	FOREACH_ENUM( HoldNoteScore, hns )
	{
		const RString &sCommand = HoldNoteScoreToString(hns);
		if( !m_textDeltaSeconds.HasCommand( sCommand ) )
			ActorUtil::LoadCommand( m_textDeltaSeconds, sType, sCommand );
	}
}

void ScoreDisplayLifeTime::Update( float fDelta )
{
	ScoreDisplay::Update( fDelta );

	float fSecs = m_pPlayerStageStats->m_fLifeRemainingSeconds;

	RString s = SecondsToMSSMsMs(fSecs);
	m_textTimeRemaining.SetText( s );
}

void ScoreDisplayLifeTime::OnLoadSong()
{
	/* 
	if( STATSMAN->m_CurStageStats.m_player[m_pPlayerState->m_PlayerNumber].m_bFailed )
		return;

	Course* pCourse = GAMESTATE->m_pCurCourse;
	ASSERT( pCourse );
	const CourseEntry *pEntry = &pCourse->m_entries[GAMESTATE->GetCourseSongIndex()];
	*/

	// PlayGainLoss( GAIN_LIFE_COMMAND_NAME, pEntry->fGainSeconds );
}

void ScoreDisplayLifeTime::OnJudgment( TapNoteScore tns )
{
	/*
	if( STATSMAN->m_CurStageStats.m_player[m_pPlayerState->m_PlayerNumber].m_bFailed )
		return;

	float fMeterChange = 0;
	switch( tns )
	{
		// TODO: Interface with Prefsmanager.
		case TNS_W1:		fMeterChange = 0.2f		break;
		case TNS_W2:		fMeterChange = 0.0f		break;
		case TNS_W3:		fMeterChange = -0.5f	break;
		case TNS_W4:		fMeterChange = -1.0f	break;
		case TNS_W5:		fMeterChange = -2.0f	break;
		case TNS_Miss:		fMeterChange = -4.0f	break;
		case TNS_HitMine:	fMeterChange = -2.0f	break;
		default:	ASSERT(0);
	}
	*/

	// PlayGainLoss( TapNoteScoreToString(tns), fMeterChange );
}

void ScoreDisplayLifeTime::OnJudgment( HoldNoteScore hns, TapNoteScore tns )
{
	/* 
	if( STATSMAN->m_CurStageStats.m_player[m_pPlayerState->m_PlayerNumber].m_bFailed )
		return;

	float fMeterChange = 0;
	switch( hns )
	{
		// TODO: Interface with Prefsmanager.
		case HNS_Held:	fMeterChange = 0.0f	break;
		case HNS_LetGo:	fMeterChange = -4.0f break;
		default:	ASSERT(0);
	}
	*/

	// PlayGainLoss( HoldNoteScoreToString(hns), fMeterChange );
}

/* 
void ScoreDisplayLifeTime::PlayGainLoss( const CString &sCommand, float fChange )
{
	CString s;
	if( &fChange != 0 )
		s = ssprintf( "%+1.1fs", &fChange );

	m_textDeltaSeconds.SetText( s );
	m_textDeltaSeconds.PlayCommand( &sCommand );
}
*/

/*
 * (c) 2001-2004 Chris Danford
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
