#include "global.h"
#include "ScoreKeeper.h"
#include "NoteData.h"
#include "PlayerState.h"
#include "NoteDataWithScoring.h"

ScoreKeeper::ScoreKeeper(PlayerState *pPlayerState, PlayerStageStats *pPlayerStageStats)
{
	m_pPlayerState = pPlayerState;
	m_pPlayerStageStats = pPlayerStageStats;
}

void ScoreKeeper::GetScoreOfLastTapInRow(const NoteData &nd, int iRow,
        TapNoteScore &tnsOut, int &iNumTapsInRowOut)
{
	int iNum = 0;

	for (int track = 0; track < nd.GetNumTracks(); ++track)
	{
		const TapNote &tn = nd.GetTapNote(track, iRow);

		if (tn.type != TapNote::tap && tn.type != TapNote::hold_head)
		{
			continue;
		}
		++iNum;
	}
	tnsOut = NoteDataWithScoring::LastTapNoteWithResult(nd, iRow).result.tns;
	iNumTapsInRowOut = iNum;
}

#include "ScoreKeeperNormal.h"
#include "ScoreKeeperRave.h"
#include "ScoreKeeperGuitar.h"
#include "ScoreKeeperShared.h"

ScoreKeeper* ScoreKeeper::MakeScoreKeeper(RString sClassName, PlayerState *pPlayerState, PlayerStageStats *pPlayerStageStats)
{
	if (sClassName == "ScoreKeeperNormal")
	{
		return new ScoreKeeperNormal(pPlayerState, pPlayerStageStats);
	}
	else if (sClassName == "ScoreKeeperRave")
	{
		return new ScoreKeeperRave(pPlayerState, pPlayerStageStats);
	}
	else if (sClassName == "ScoreKeeperGuitar")
	{
		return new ScoreKeeperGuitar(pPlayerState, pPlayerStageStats);
	}
	else if (sClassName == "ScoreKeeperShared")
	{
		return new ScoreKeeperShared(pPlayerState, pPlayerStageStats);
	}
	FAIL_M(sClassName);
}


/*
 * (c) 2006 Steve Checkoway
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
