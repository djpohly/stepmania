#include "global.h"

#include "RageUtil_DB.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "arch/arch.h"
#include "arch/ArchHooks/ArchHooks.h"

/** @brief Include a global access to the database. */
Database * DATABASE = NULL;

Database::Database()
{
	m_pDatabase = NULL;
	m_pResult = NULL;
	m_pCurrentRow = NULL;
	m_Connected = connect();
	
	if( m_Connected == SQLITE_OK )
		LOG->Trace("Successfully connected with database '%s'",
			   SpecialFiles::DATABASE_NAME.c_str());
	else
		LOG->Warn("Error connecting with database '%s'",
			  SpecialFiles::DATABASE_NAME.c_str());
}

Database::~Database()
{
	close();
	m_pCurrentRow = NULL;
	SAFE_DELETE(m_pResult);
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

bool Database::query( RString sQuery, int iCols )
{
	bool bReturn = false;
	if( m_Connected == SQLITE_OK )
	{
		ASSERT(m_pDatabase);
		sqlite3* sqlDatabase = reinterpret_cast<sqlite3*>(m_pDatabase);
		sqlite3_stmt* sqlStatement;
		if (this->GetResult() != NULL)
		{
			this->clearResult();
			this->SetResult(new QueryResult);
			
		}
		if( sqlite3_prepare_v2(sqlDatabase, sQuery, -1, &sqlStatement, 0) == SQLITE_OK )
		{
			iCols = sqlite3_column_count(sqlStatement);
			while(sqlite3_step(sqlStatement) == SQLITE_ROW)
			{
				QueryRow sqlRow;
				for(int col=0; col<iCols; col++)
				{
					sqlRow.push_back(sqlite3_column_blob(sqlStatement, col));
				}
				m_pResult->push_back(sqlRow);
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
	if (this->query(sql, 1))
	{
		// query worked fine.
		if (this->m_pResult->size() > 0)
		{
			// the table & row exists.
			this->setCurrentRow(0);
			if (this->getColValueAsUnsignedInt(0) == DATABASE_VERSION)
			{
				// no need to re-create.
				return;
			}
		}
	}
	
	// TODO: Add the tables.
}

void Database::setCurrentRow( unsigned uRow )
{
	QueryResult * r = this->GetResult();
	ASSERT(r);
	ASSERT(uRow < r->size());
	m_pCurrentRow = &r->at(uRow);
}

const void* Database::getColValue( unsigned uCol )
{
	ASSERT(m_pCurrentRow);
	ASSERT(uCol < m_pCurrentRow->size());
	return &m_pCurrentRow->at(uCol);
}

void Database::clearResult()
{
	m_pCurrentRow = NULL;
	SAFE_DELETE(m_pResult);
}

/*
 * Copyright (c) 2011 Aldo Fregoso
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
