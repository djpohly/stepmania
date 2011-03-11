/** @brief SSCLoader - Reads a Song and its Steps from a .SSC file. */
#ifndef NotesLoaderSSC_H
#define NotesLoaderSSC_H

#include "BackgroundUtil.h"
#include "GameConstantsAndTypes.h"

class MsdFile;
class Song;
class Steps;
class TimingData;

/**
 * @brief The various states while parsing a .ssc file.
 */
enum SSCLoadingStates
{
	GETTING_SONG_INFO, /**< Retrieving song information. */
	GETTING_STEP_INFO, /**< Retrieving step information. */
	GETTING_STEP_TIMING_INFO, /**< Retrieving a step's individual timing information. */
	GETTING_NOTE_INFO, /**< Retrieving the specific notes. This state may be deprecated. */
	NUM_SSCLoadingStates /**< The number of states used. */
};

const float VERSION_RADAR_FAKE = 0.53f;

/**
 * @brief The SSCLoader handles all of the parsing needed for .ssc files.
 */
namespace SSCLoader
{
	/**
	 * @brief Attempt to load a song from a specified path.
	 * @param sPath a const reference to the path on the hard drive to check.
	 * @param out a reference to the Song that will retrieve the song information.
	 * @return its success or failure.
	 */
	bool LoadFromDir( const RString &sPath, Song &out );
	/**
	 * @brief Attempt to load the specified ssc file.
	 * @param sPath a const reference to the path on the hard drive to check.
	 * @param out a reference to the Song that will retrieve the song information.
	 * @param bFromCache a check to see if we are getting certain information from the cache file.
	 * @return its success or failure.
	 */
	bool LoadFromSSCFile( const RString &sPath, Song &out, bool bFromCache = false );
	/**
	 * @brief Retrieve the list of .ssc files.
	 * @param sPath a const reference to the path on the hard drive to check.
	 * @param out a vector of files found in the path.
	 */
	void GetApplicableFiles( const RString &sPath, vector<RString> &out );
	/**
	 * @brief Attempt to load an edit from the hard drive.
	 * @param sEditFilePath a path on the hard drive to check.
	 * @param slot the Profile of the user with the edit.
	 * @param bAddStepsToSong a flag to determine if we add the edit steps to the song file.
	 * @return its success or failure.
	 */
	bool LoadEditFromFile( RString sEditFilePath, ProfileSlot slot, bool bAddStepsToSong );
	/**
	 * @brief Attempt to parse the edit file in question.
	 * @param msd the edit file itself.
	 * @param sEditFilePath a const reference to a path on the hard drive to check.
	 * @param slot the Profile of the user with the edit.
	 * @param bAddStepsToSong a flag to determine if we add the edit steps to the song file.
	 * @return its success or failure.
	 */
	bool LoadEditFromMsd( const MsdFile &msd, const RString &sEditFilePath, ProfileSlot slot, bool bAddStepsToSong );
	/**
	 * @brief Perform some cleanup on the loaded song.
	 * @param song a reference to the song that may need cleaning up.
	 * @param bFromCache a flag to determine if this song is loaded from a cache file.
	 */
	void TidyUpData( Song &song, bool bFromCache );
}

/**
 * @brief Attempt to load any background changes in use by this song.
 * 
 * This code is right now copied from NotesLoaderSM. There may be a time
 * when we add to this code, or perhaps just refactor it properly.
 * @param change a reference to the background change.
 * @param sBGChangeExpression a reference to the list of changes to be made.
 * @return its success or failure.
 */
bool LoadFromBGSSCChangesString( BackgroundChange &change, const RString &sBGChangeExpression );

#endif
/**
 * @file
 * @author Jason Felds (c) 2011
 *
 * @section LICENSE
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
