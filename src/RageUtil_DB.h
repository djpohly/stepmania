#ifndef RAGE_UTIL_DB
#define RAGE_UTIL_DB

#include "global.h"
#include "SpecialFiles.h" // contains database location.
#include "sqlite3.h"

/** @brief Include a simple way to get the row of a result. */
typedef vector<const void *> QueryRow;
/** @brief Include a simple way to get the result of a query. */
typedef vector<QueryRow> QueryResult;

/**
 * @brief The internal version of the database cache for StepMania.
 *
 * Increment this value to invalidate the current cache. */
const unsigned int DATABASE_VERSION = 1;

/** @brief The controls to access the database. */
class Database
{
	/**
	 * @brief Attempt to connect to the database.
	 * @return the result code from opening the database. */
	int connect();
	/** @brief Close the database upon its completion. */
	void close();
	
	/** @brief Are we connected to the database itself? */
	int			m_Connected;
	/** @brief The database pointer itself. */
	void*			m_pDatabase;
	/** @brief The result from a query. */
	QueryResult*	m_pResult;
	/** @brief The row we're iterating over. */
	QueryRow*		m_pCurrentRow;
	
	/** 
	 * @brief Check for a specific table and recreate the DB as needed.
	 *
	 * This allows for database versioning. */
	void CreateTablesIfNeeded();
	
	/**
	 * @brief Prevent the Database from being copied.
	 * @param rhs unused. */
	Database(const Database& rhs);
	/**
	 * @brief Prevent the Database from being assigned.
	 * @param rhs unused. */
	Database& operator=(const Database& rhs);
	
public:
	/** @brief Set up the database. */
	Database();
	/** @brief Tear down the database. */
	~Database();
	
	/**
	 * @brief Get the result of the connection attempt.
	 * @return the connection result attempt. */
	int GetConnectionResult() const { return this->m_Connected; }
	
	/**
	 * @brief Helper function to start a transaction.
	 * @param kind the type of transaction.
	 * @return the result of the query. */
	bool BeginTransaction( RString kind = "DEFERRED" );
	
	/**
	 * @brief Helper function to commit a transaction.
	 * @return the result of the query. */
	bool CommitTransaction();
	
	/**
	 * @brief Helper function to abort a transaction.
	 *
	 * Only call this is something goes horribly wrong.
	 * @return the result of the query. */
	bool RollbackTransaction();
	
	/**
	 * @brief Call a query and get an expected result.
	 * @param sQuery the query to call.
	 * @param iCols the number of columns expected.
	 * @return true if successful in calling the query, false otherwise. */
	bool query( RString sQuery, int iCols );
	/**
	 * @brief Call a query. The query returns no results.
	 * @param sQuery the query to call.
	 * @return true if successful in calling the query, false otherwise. */
	bool queryNoResult( RString sQuery );
	void setCurrentRow( unsigned uRow );
	const void* getColValue( unsigned uCol );
	
	/**
	 * @brief Retrieve the current result.
	 * @return the current result. */
	QueryResult * GetResult() { return m_pResult; }
	
	/**
	 * @brief Set up the new current result.
	 * @param r the new result. */
	void SetResult(QueryResult * r) { m_pResult = r; }
	
	/** @brief Clear the result when we're done with it. */
	void clearResult();
	
	int getColValueAsInt( unsigned uCol )
	{
		return reinterpret_cast<int>(getColValue(uCol));
	}
	unsigned getColValueAsUnsignedInt( unsigned uCol )
	{
		return reinterpret_cast<unsigned>(getColValue(uCol));
	}
	double getColValueAsDouble( unsigned uCol )
	{
		return *(double*)reinterpret_cast<const double*>(getColValue(uCol));
	}
	float getColValueAsFloat( unsigned uCol )
	{
		return (float)getColValueAsDouble(uCol);
	}
	char* getColValueAsChar( unsigned uCol )
	{
		return (char*)reinterpret_cast<const char*>(getColValue(uCol));
	}
	RString getColValueAsString( unsigned uCol )
	{
		return RString(getColValueAsChar(uCol));
	}
	unsigned char* getColValueAsUnsignedChar( unsigned uCol )
	{
		return (unsigned char*)reinterpret_cast<const unsigned char*>(getColValue(uCol));
	}
};

/** @brief Be sure that we can access the global variable. */
extern Database * DATABASE;

#endif

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
