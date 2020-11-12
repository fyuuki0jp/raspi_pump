#include <plog/Log.h>
#include "SQLite.h"

CSQLite::CSQLite(void)
    :m_pDB(NULL)
    ,m_pStmt(NULL)
{

}

CSQLite::~CSQLite(void)
{

}

bool CSQLite::Open(const char* filename)
{
    bool ret = true;

    int err = sqlite3_open(filename, &m_pDB);

    if(err != SQLITE_OK)
	{
 //       fprintf(stderr, "OPEN ERR! %d\n", err);
        ret = false;
        /* TODO:ÉGÉâÅ[èàóù */
    }

    return ret;
}

void CSQLite::Close(void)
{
    sqlite3_close(m_pDB);
}

bool CSQLite::Exec(const char* sql)
{
    char *errMsg = NULL;
    int err = 0;
    bool ret = true;
    err = sqlite3_exec(m_pDB,sql,NULL,NULL,&errMsg);

    if(err != SQLITE_OK)
    {
//        fprintf(stderr, "exec failed\n");
//        fprintf(stderr, errMsg);
        LOG_ERROR << "exec failed : " << errMsg;
        ret = false;
    }
    return ret;
}

bool CSQLite::CreateStmt(const char* sql)
{
    bool ret = true;
    int err = sqlite3_prepare_v2(m_pDB,
        sql, -1,
        &m_pStmt, NULL);

    if(err != SQLITE_OK)
    {
        ret = false;
    }

    return ret;
}

bool CSQLite::RowExec()
{
    bool ret = false;
    if(SQLITE_ROW == sqlite3_step(m_pStmt))
    {
        ret = true;
    }
    return ret;
}

int CSQLite::GetColumnInt(int column)
{
    return sqlite3_column_int(m_pStmt,column);
}

const char* CSQLite::GetColumnStr(int column)
{
    return (const char*)sqlite3_column_text(m_pStmt, column);
}

int CSQLite::Finalize(void)
{
    return sqlite3_finalize(m_pStmt);
}
