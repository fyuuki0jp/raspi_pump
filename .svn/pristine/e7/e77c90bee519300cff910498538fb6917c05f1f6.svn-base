//
// Data.h
//
//  CDataクラス の header
//     .ini ファイル管理
//
//
#ifndef _DATA_H_
#define _DATA_H_

#include <map>

class CData
{
  public:
	CData();
	CData(const char *filename);
	~CData();

	bool open(const char *filename);
	bool close(void);
	bool getintdata(const char *key, int &data, int def);
	bool setintdata(const char *key, int data);
	bool getstrdata(const char *key, char *data, const char *def);
	bool setstrdata(const char *key, char *data);
	bool getbooldata(const char *key, bool &data, bool def);
	bool setbooldata(const char *key, bool data);

  private:
	char m_filename[256];
	FILE *fh;
	std::map<std::string, std::string> m_data;
};

#endif // _DATA_H_