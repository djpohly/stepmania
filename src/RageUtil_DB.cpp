#include "global.h"

#include "RageUtil_DB.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "Song.h" // song & SSC stepfile versions
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
			   SpecialFiles::DATABASE_NAME.c_str());
		this->CreateTablesIfNeeded();
	}
	else
		LOG->Warn("Error connecting with database '%s'",
			  SpecialFiles::DATABASE_NAME.c_str());
}

Database::~Database()
{
	close();
}

int Database::connect()
{
	RString path = HOOKS->GetCacheDir() + "/" + SpecialFiles::DATABASE_NAME;
	return sqlite3_open(path, reinterpret_cast<sqlite3**>(&m_pDatabase));
}

void Database::close()
{
	m_Connected = -1;
	sqlite3_close(reinterpret_cast<sqlite3*>(m_pDatabase));
}

bool Database::query( RString sQuery, int iCols, vector<ColumnTypes> v )
{
	bool bReturn = false;
	if( m_Connected == SQLITE_OK )
	{
		ASSERT(m_pDatabase);
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
		ASSERT(m_pDatabase);
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
	sql = "CREATE TABLE \"globals\" ";
	sql += "( \"key\" TEXT NOT NULL UNIQUE, \"value\" TEXT NOT NULL );";
	RollbackIfFailure;
	
	RString base = "INSERT INTO \"globals\" (\"key\", \"value\") VALUES ";
	sql = base + "('db_version', " + IntToString(DATABASE_VERSION) + ");";
	RollbackIfFailure;
	
	sql = base + "('song_cache_version', " + IntToString(FILE_CACHE_VERSION) + ");";
	RollbackIfFailure;
	
	sql = base + "('ssc_file_version', " + FloatToString(STEPFILE_VERSION_NUMBER) + ");";
	RollbackIfFailure;
	
	const RString blankText = "TEXT NOT NULL DEFAULT '',";
	
	// songs table
	sql = "CREATE TABLE \"songs\" ";
	sql += "( \"ID\" INTEGER PRIMARY KEY AUTOINCREMENT, \"file_hash\" TEXT NOT NULL, ";
	sql += "\"version\" REAL NOT NULL DEFAULT " + FloatToString(STEPFILE_VERSION_NUMBER);
	sql += ", \"song_title\" TEXT NOT NULL, \"song_subtitle\" " + blankText;
	sql += "\"song_artist\" " + blankText + " \"song_title_translit\" " + blankText;
	sql += "\"song_subtitle_translit\" " + blankText + "\"song_artist_translit\" " + blankText;
	
	sql += "\"offset\" REAL NOT NULL DEFAULT 0);";
	
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
