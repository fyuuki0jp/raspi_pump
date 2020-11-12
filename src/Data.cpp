//
// Data.cpp
//
//  CDataクラス
//     .ini ファイル管理
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <plog/Log.h>
#include "datatype.h"
#include "Data.h"

CData::CData()
	: fh(NULL)
{
	m_filename[0] = 0;
}

CData::CData(const char *filename)
	: fh(NULL)
{
	open(filename);
}

CData::~CData()
{
	close();
}

bool CData::open(const char *filename)
{
	if (fh)
		close();

//	fprintf(stderr, "open\n");
	LOG_DEBUG << "open : " << filename;

	fh = fopen(filename, "r+");

	if (fh)
	{
		char name[256];
		char data[256];
		std::string n;
		std::string d;

		while(fscanf(fh, "%s = %s", name, data) != EOF)
		{
			LOG_DEBUG << name << " = " << data;
			//printf("%s = %s\n",name,data);
			n = name;
			d = data;
			m_data.insert(std::pair<std::string, std::string>(n, d));
		}
		return true;
	}

	return false;
}

bool CData::close(void)
{
//	if (!m_data.empty())
//		m_data.clear();

	fclose(fh);
	return true;
}

bool CData::getbooldata(const char *key, bool &data, bool def)
{
	std::string n = key;
	std::map<std::string, std::string>::iterator itr;
	itr = m_data.find(n);

	if (itr != m_data.end())
	{
		// キーを取得します。
		std::string key0 = itr->first;
		// 値も取得します。
		std::string value = itr->second;
		LOG_INFO << key0.c_str() << " = " << value.c_str();

		if ( value == "ON" )
			data = true;
		else
			data = false;
	}
	else
	{
		LOG_DEBUG << key << " = " << def << " (none)";
		data = def;
	}

	return true;
}

bool setbooldata(const char *key, bool data)
{
	return true;
}

bool CData::getintdata(const char *key, int &data, int def)
{
	std::string n = key;
	std::map<std::string, std::string>::iterator itr;
	itr = m_data.find(n);

	if (itr != m_data.end())
	{
		// キーを取得します。
		std::string key0 = itr->first;
		// 値も取得します。
		std::string value = itr->second;
		LOG_INFO << key0.c_str() << " = " << value.c_str();

		data = atoi(value.c_str());
	}
	else
	{
		LOG_DEBUG << key << " = " << def << " (none)";
		data = def;
	}

	return true;
}

bool setintdata(const char *key, int data)
{
	return true;
}

bool CData::getstrdata(const char *key, char *data, const char *def)
{
	std::string n = key;
	std::map<std::string, std::string>::iterator itr;
	itr = m_data.find(n);

	if (itr != m_data.end())
	{
		// キーを取得します。
		std::string key0 = itr->first;
		// 値も取得します。
		std::string value = itr->second;

		LOG_INFO << key0.c_str() << " = " << value.c_str();
		value.copy(data, 255);
	}
	else
	{
		LOG_DEBUG << key << " = " << def << " (none)";
		std::string value = def;
		value.copy(data, 255);
	}
	return true;
}

bool CData::setstrdata(const char *key, char *data)
{
	return true;
}