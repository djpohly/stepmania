#ifndef RAGE_UTIL_DB
#define RAGE_UTIL_DB

#include "global.h"
#include "SpecialFiles.h" // contains database location.
#include "Song.h" // song & SSC stepfile versions, cache.
#include "sqlite3.h"

/** @brief The expected column types for SQLite. */
enum ColumnTypes
{
	COL_INT, /**< We want an integer. */
	COL_UNSIGNED, /**< We want an unsigned integer. */
	COL_FLOAT, /**< We want a float value. */
	COL_DOUBLE, /**< We want a double value. */
	COL_STRING, /**< We want a string. */
	COL_NULL /**< We want nothing. */
};

/** @brief Retrieve our column results. */
struct ColumnData
{
	/** @brief The specific data type. */
	ColumnTypes type;
	/** @brief Is the underlying result a number? */
	bool isNum;
	/**
	 * @brief The text that is stored.
	 * This is not in the union below due to technical issues. */
	RString text;
	union 
	{
		/** @brief The integer that is stored. */
		int i;
		/** @brief the float that is stored. */
		float f;
	};

	ColumnData() : type(COL_NULL), isNum(false), text(""), i(0) {}
	ColumnData(ColumnTypes c) : type(c), isNum(false), text(""), i(0)
	{
		if (c < COL_STRING) isNum = true;
	}
	ColumnData(ColumnTypes c, RString tmp) : type(c), isNum(false), text(tmp), i(0) {}
	ColumnData(ColumnTypes c, int tmp) : type(c), isNum(true), text(""), i(tmp) {}
	ColumnData(ColumnTypes c, float tmp) : type(c), isNum(true), text(""), f(tmp) {}
};

/** @brief Include a simple way to get the row of a result. */
typedef vector<ColumnData> QueryRow;
/** @brief Include a simple way to get the result of a query. */
typedef vector<QueryRow> QueryResult;

/**
 * @brief The internal version of the database cache for StepMania.
 *
 * Increment this value to invalidate the current cache. */
const int DATABASE_VERSION = 14;

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
	QueryResult	m_pResult;
	/** @brief The row we're iterating over. */
	QueryRow		m_pCurrentRow;

	/** 
	 * @brief Check for a specific table and recreate the DB as needed.
	 * This allows for database versioning. */
	void CreateTablesIfNeeded();

	RString EscapeQuote(RString tmp);
	
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
	bool query( RString sQuery, int iCols, vector<ColumnTypes> v );
	/**
	 * @brief Call a query. The query returns no results.
	 * @param sQuery the query to call.
	 * @return true if successful in calling the query, false otherwise. */
	bool queryNoResult( RString sQuery );
	void setCurrentRow( unsigned uRow );

	/**
	 * @brief Retrieve the current result.
	 * @return the current result. */
	QueryResult GetResult() { return m_pResult; }

	/**
	 * @brief Set up the new current result.
	 * @param r the new result. */
	void SetResult(QueryResult r) { m_pResult = r; }

	/** @brief Clear the result when we're done with it. */
	void clearResult();

	int getColValueAsInt( unsigned uCol )
	{
		ASSERT(!m_pCurrentRow.empty());
		ASSERT(uCol < m_pCurrentRow.size());
		return m_pCurrentRow.at(uCol).i;
	}
	unsigned int getColValueAsUnsignedInt( unsigned uCol )
	{
		return static_cast<unsigned int>(this->getColValueAsInt(uCol));
	}
	double getColValueAsDouble( unsigned uCol )
	{
		return static_cast<double>(this->getColValueAsFloat(uCol));
	}
	float getColValueAsFloat( unsigned uCol )
	{
		ASSERT(!m_pCurrentRow.empty());
		ASSERT(uCol < m_pCurrentRow.size());
		return m_pCurrentRow.at(uCol).f;
	}
	RString getColValueAsString( unsigned uCol )
	{
		ASSERT(!m_pCurrentRow.empty());
		ASSERT(uCol < m_pCurrentRow.size());
		return m_pCurrentRow.at(uCol).text;
	}
	
	bool AddSongToCache(const Song &s, const vector<Steps*>& vpStepsToSave);
};

/** @brief Be sure that we can access the global variable. */
extern Database * DATABASE;

#endif

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
