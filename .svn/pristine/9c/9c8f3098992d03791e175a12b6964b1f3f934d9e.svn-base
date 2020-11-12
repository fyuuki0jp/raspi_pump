
//#include "sqlite3ext.h"
#include "sqlite3.h"
#include <string>

class CSQLite {
private:
    sqlite3* m_pDB;
    sqlite3_stmt* m_pStmt;

    public:
        CSQLite();
        ~CSQLite();
        bool Open(const char* filename);
        void Close();
        bool Exec(const char* sql);
        bool CreateStmt(const char* sql);
        bool RowExec(void);
        int GetColumnInt(int column);
        const char* GetColumnStr(int column);
        int Finalize(void);
};