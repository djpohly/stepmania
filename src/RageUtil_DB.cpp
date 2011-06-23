#include "global.h"

#include "RageUtil_DB.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "arch/arch.h"
#include "arch/ArchHooks/ArchHooks.h"

/** @brief Include a global access to the database. */
Database * DATABASE = NULL;

Database::Database()
{
	m_pDatabase = NULL;
	m_Connected = connect();

	if( m_Connected == SQLITE_OK )
	{
		LOG->Trace("Successfully connected with database '%s'",
			   SpecialFiles::DATABASE_PATH.c_str());
		this->CreateTablesIfNeeded();
	}
	else
		LOG->Warn("Error connecting with database '%s'",
			  SpecialFiles::DATABASE_PATH.c_str());
}

Database::~Database()
{
	close();
}

int Database::connect()
{
	RString path = SpecialFiles::CACHE_DIR + SpecialFiles::DATABASE_PATH;
	if (!DoesFileExist(path))
	{
		RageFile f;
		f.Open(path, RageFile::WRITE);
	}
#if !defined(WIN32)
	path = HOOKS->GetCacheDir() + "/" + SpecialFiles::DATABASE_PATH;
#endif
	return sqlite3_open(path, reinterpret_cast<sqlite3**>(&m_pDatabase));
}

void Database::close()
{
	m_Connected = -1;
	sqlite3_close(reinterpret_cast<sqlite3*>(m_pDatabase));
}

RString Database::EscapeQuote(RString tmp)
{
	tmp.Replace("'", "''");
	return tmp;
}

bool Database::query( RString sQuery, int iCols, vector<ColumnTypes> v )
{
	bool bReturn = false;
	if( m_Connected == SQLITE_OK )
	{
		ASSERT_M(m_pDatabase, "The database was lost! Unable to continue.");
		sqlite3* sqlDatabase = reinterpret_cast<sqlite3*>(m_pDatabase);
		sqlite3_stmt* sqlStatement;
		if (!this->GetResult().empty())
		{
			this->clearResult();
		}
		if( sqlite3_prepare_v2(sqlDatabase, sQuery, -1, &sqlStatement, 0) == SQLITE_OK )
		{
			iCols = sqlite3_column_count(sqlStatement);
			while(sqlite3_step(sqlStatement) == SQLITE_ROW)
			{
				QueryRow sqlRow;
				for(int col=0; col<iCols; col++)
				{
					ColumnData data(v[col]);
					switch (v[col])
					{
						case COL_INT:
						case COL_UNSIGNED:
						{
							data.i = sqlite3_column_int(sqlStatement, col);
							break;
						}
						case COL_FLOAT:
						case COL_DOUBLE:
						{
							data.f = static_cast<float>
								(sqlite3_column_double(sqlStatement, col));
							break;
						}
						case COL_STRING:
						{
							data.text = reinterpret_cast<const char *>
								(sqlite3_column_text(sqlStatement, col));
							break;
						}
						case COL_NULL:
						{
							// what DOES happen here?
							break;
						}
					}

					sqlRow.push_back(data);
				}
				m_pResult.push_back(sqlRow);
			}
			sqlite3_finalize(sqlStatement);
			bReturn = true;
		}
		else
		{
			LOG->Warn("SQLite Error: %s\n--\nQuery: %s",
				  sqlite3_errmsg(sqlDatabase),
				  sQuery.c_str() );
		}
	}
	return bReturn;
}

bool Database::queryNoResult( RString sQuery )
{
	bool bReturn = false;
	if( m_Connected == SQLITE_OK )
	{
		ASSERT_M(m_pDatabase, "The database was lost! Unable to continue.");
		sqlite3* sqlDatabase = reinterpret_cast<sqlite3*>(m_pDatabase);
		sqlite3_stmt* sqlStatement;
		if( sqlite3_prepare_v2(sqlDatabase, sQuery, -1, &sqlStatement, 0) == SQLITE_OK )
		{
			sqlite3_step(sqlStatement);
			sqlite3_finalize(sqlStatement);
			bReturn = true;
		}
		else
		{
			LOG->Warn("SQLite Error: %s\n--\nQuery: %s",
				  sqlite3_errmsg(sqlDatabase),
				  sQuery.c_str() );
		}
	}
	return bReturn;
}

bool Database::BeginTransaction(RString kind)
{
	if (kind != "DEFERRED" && kind != "IMMEDIATE" && kind != "EXCLUSIVE")
		kind = "DEFERRED";
	return this->queryNoResult("BEGIN " + kind + " TRANSACTION;");
}

bool Database::CommitTransaction()
{
	return this->queryNoResult("COMMIT TRANSACTION;");
}

bool Database::RollbackTransaction()
{
	return this->queryNoResult("ROLLBACK TRANSACTION;");
}

void Database::CreateTablesIfNeeded()
{
	RString sql = "SELECT \"value\" FROM \"globals\" WHERE \"key\" = 'db_version';";
	vector<ColumnTypes> v;
	v.push_back(COL_STRING);
	if (this->query(sql, 1, v))
	{
		// table exists.
		if (!this->GetResult().empty())
		{
			// row exists.
			this->setCurrentRow(0);
			int tmp = StringToInt(this->getColValueAsString(0));
			if (tmp == DATABASE_VERSION)
			{
				// no need to re-create.
				return;
			}
		}
	}

	this->queryNoResult("DROP TABLE IF EXISTS \"course_songs\";");
	this->queryNoResult("DROP TABLE IF EXISTS \"courses\";");
	this->queryNoResult("DROP TABLE IF EXISTS \"steps\";");
	this->queryNoResult("DROP TABLE IF EXISTS \"songs\";");
	this->queryNoResult("DROP TABLE IF EXISTS \"globals\";");

	this->BeginTransaction();

/** @brief Provide a way to get out of a transaction on failure. */
#define RollbackIfFailure if (!this->queryNoResult(sql)) \
{ \
	this->RollbackTransaction(); \
	return; \
}

	// XXX: Is there a better way for multiline RString intros?
	sql = "CREATE TABLE \"globals\" " \
			"( \"key\" TEXT NOT NULL UNIQUE, \"value\" TEXT NOT NULL );";
	RollbackIfFailure;

	RString base = "INSERT INTO \"globals\" (\"key\", \"value\") VALUES ";
	sql = base + "('db_version', " + IntToString(DATABASE_VERSION) + ");";
	RollbackIfFailure;

	sql = base + "('song_cache_version', " + IntToString(FILE_CACHE_VERSION) + ");";
	RollbackIfFailure;

	sql = base + "('ssc_file_version', " + FloatToString(STEPFILE_VERSION_NUMBER) + ");";
	RollbackIfFailure;

	const RString blankText = " TEXT NOT NULL DEFAULT '', ";
	const RString blankFloat = " REAL NOT NULL DEFAULT 0, ";
	const RString noTextNull = " TEXT NOT NULL, ";
	const RString PK = "\"ID\" INTEGER PRIMARY KEY AUTOINCREMENT, ";
	const RString boolFalse = " INTEGER NOT NULL DEFAULT 0, ";

	// songs table
	sql = "CREATE TABLE \"songs\" ( " \
		+ PK + "\"file_hash\"" + noTextNull \
		+ "\"version\" REAL NOT NULL DEFAULT " + FloatToString(STEPFILE_VERSION_NUMBER) \
		+ ", \"song_title\"" + noTextNull + "\"song_subtitle\"" + blankText \
		+ "\"song_artist\"" + blankText + "\"song_title_translit\"" + blankText \
		+ "\"song_subtitle_translit\"" + blankText + "\"song_artist_translit\"" + blankText \
		+ "\"genre\"" + blankText + "\"origin\"" + blankText + "\"credit\"" + blankText \
		+ "\"banner\"" + blankText + "\"background\"" + blankText + "\"lyrics_path\"" + blankText \
		+ "\"cdtitle\"" + blankText + "\"music\"" + blankText \
		+ "\"sample_start\" REAL NOT NULL DEFAULT 30, \"sample_length\" REAL NOT NULL DEFAULT 12, " \
		+ "\"display_bpm\"" + blankText + "\"selectable\" TEXT NOT NULL DEFAULT 'YES', " \
		+ "\"first_beat\"" + blankFloat + "\"last_beat\"" + blankFloat \
		+ "\"song_file_name\"" + noTextNull + "\"has_music\"" + boolFalse \
		+ "\"has_banner\"" + boolFalse + "\"music_length\"" + blankFloat \
		+ "\"bpms\"" + blankText + "\"stops\"" + blankText + "\"delays\"" + blankText \
		+ "\"warps\"" + blankText + "\"time_signatures\"" + blankText \
		+ "\"tickcounts\"" + blankText + "\"combos\"" + blankText \
		+ "\"speeds\"" + blankText + "\"scrolls\"" + blankText \
		+ "\"fakes\"" + blankText + "\"labels\"" + blankText \
		+ "\"attacks\"" + blankText \
		+ "\"offset\" REAL NOT NULL DEFAULT 0);";

	RollbackIfFailure;

	// courses table (probably needs redoing)
	sql = "CREATE TABLE \"courses\" ( ";
	sql += PK + "\"banner\"" + blankText + "\"lives\" INTEGER NOT NULL DEFAULT -1, ";
	sql += "\"course\"" + noTextNull;
	sql += "\"is_endless\"" + boolFalse;
	sql += "\"gain_seconds\" REAL NOT NULL DEFAULT -1);";

	RollbackIfFailure;

	// course songs table (which songs are in which course?)
	sql = "CREATE TABLE \"course_songs\" ( " \
		+ PK + "\"course_ID\" INTEGER NOT NULL, \"song_ID\" INTEGER DEFAULT NULL, " \
		+ "\"song_order\" INTEGER NOT NULL DEFAULT 0, " \
		+ "\"song_special\"" + blankText + "\"difficulty\"" + blankText \
		+ "\"mods\"" + blankText + "\"gain_lives\" INTEGER NOT NULL DEFAULT 0, " \
		+ "\"gain_seconds\"" + blankFloat \
		+ "FOREIGN KEY(\"course_ID\") REFERENCES \"courses\"(\"ID\"), " \
		+ "FOREIGN KEY(\"song_ID\") REFERENCES \"songs\"(\"ID\") );";

	RollbackIfFailure;

	// steps table
	sql = "CREATE TABLE \"steps\" ( " \
		+ PK + "\"song_ID\" INTEGER NOT NULL, \"step_hash\"" + blankText \
		+ "\"is_autogen\"" + boolFalse + "\"steps_type\"" + noTextNull \
		+ "\"description\"" + blankText + "\"chart_style\"" + blankText \
		+ "\"difficulty\"" + blankText + "\"meter\" INTEGER NOT NULL DEFAULT 1, " \
		+ "\"radar_values\"" + blankText + "\"credit\"" + blankText \
		+ "\"bpms\"" + blankText + "\"stops\"" + blankText + "\"delays\"" + blankText \
		+ "\"warps\"" + blankText + "\"time_signatures\"" + blankText \
		+ "\"tickcounts\"" + blankText + "\"combos\"" + blankText \
		+ "\"speeds\"" + blankText + "\"scrolls\"" + blankText \
		+ "\"fakes\"" + blankText + "\"labels\"" + blankText \
		+ "\"attacks\"" + blankText + "\"step_file_name\"" + blankText \
		+ "\"offset\"" + blankFloat \
		+ "FOREIGN KEY(\"song_ID\") REFERENCES \"songs\"(\"ID\") );";

	RollbackIfFailure;

#undef RollbackIfFailure

	this->CommitTransaction();
}

void Database::setCurrentRow( unsigned uRow )
{
	QueryResult r = this->GetResult();
	ASSERT(!r.empty());
	ASSERT(uRow < r.size());
	m_pCurrentRow = r.at(uRow);
}

void Database::clearResult()
{
	m_pCurrentRow.empty();
	m_pResult.empty();
}

bool Database::AddSongToCache(const Song &s, const vector<Steps*>& vpStepsToSave)
{
	// TODO: Finish this.
	const RString blank = "";
	const TimingData &timing = s.m_SongTiming;
	const RString NotYet = "TODO";
	
	RString displayBPM = "";
	
	switch (s.m_DisplayBPMType)
	{
		case DISPLAY_BPM_SPECIFIED:
		{
			displayBPM = FloatToString(s.m_fSpecifiedBPMMin);
			if (s.m_fSpecifiedBPMMax != s.m_fSpecifiedBPMMin)
			{
				displayBPM += ":" + FloatToString(s.m_fSpecifiedBPMMax);
			}
			break;
		}
		case DISPLAY_BPM_RANDOM:
		{
			displayBPM = "*";
			break;
		}
		default:
			break;
	}

	RString sql = "INSERT INTO \"songs\" (\"file_hash\", \"song_title\", " \
		+ blank + "\"song_subtitle\", \"song_artist\", \"song_title_translit\", " \
		+ blank + "\"song_subtitle_translit\", \"song_artist_translit\", " \
		+ blank + "\"genre\", \"origin\", \"credit\", \"banner\", \"background\", " \
		+ blank + "\"lyrics_path\", \"cdtitle\", \"music\", \"sample_start\", " \
		+ blank + "\"sample_length\", \"display_bpm\", \"selectable\", " \
		+ blank + "\"first_beat\", \"last_beat\", \"song_file_name\", " \
		+ blank + "\"has_music\", \"has_banner\", \"music_length\", " \
		+ blank + "\"bpms\", \"stops\", \"delays\", \"warps\", " \
		+ blank + "\"time_signatures\", \"tickcounts\", \"combos\", " \
		+ blank + "\"speeds\", \"scrolls\", \"fakes\", \"labels\", " \
		+ blank + "\"attacks\", \"offset\") VALUES ('" \
		+ NotYet + "', '" + this->EscapeQuote(s.m_sMainTitle) + "', '" \
		+ this->EscapeQuote(s.m_sSubTitle) + "', '" \
		+ this->EscapeQuote(s.m_sArtist) + "', '" \
		+ this->EscapeQuote(s.m_sMainTitleTranslit) + "', '" \
		+ this->EscapeQuote(s.m_sSubTitleTranslit) + "', '" \
		+ this->EscapeQuote(s.m_sArtistTranslit) + "', '" \
		+ this->EscapeQuote(s.m_sGenre) + "', '" \
		+ this->EscapeQuote(s.m_sOrigin) + "', '" \
		+ this->EscapeQuote(s.m_sCredit) + "', '" \
		+ this->EscapeQuote(s.m_sBannerFile) + "', '" \
		+ this->EscapeQuote(s.m_sBackgroundFile) + "', '" \
		+ this->EscapeQuote(s.m_sLyricsFile) + "', '" \
		+ this->EscapeQuote(s.m_sCDTitleFile) + "', '" \
		+ this->EscapeQuote(s.m_sMusicFile) + "', " \
		+ FloatToString(s.m_fMusicSampleStartSeconds) + ", " \
		+ FloatToString(s.m_fMusicSampleLengthSeconds) + ", '" \
		+ this->EscapeQuote(displayBPM)
	
	;
//	);";

	return false;
}

/**
 * @file
 * @author Aldo Fregoso, Jason Felds (c) 2011
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
