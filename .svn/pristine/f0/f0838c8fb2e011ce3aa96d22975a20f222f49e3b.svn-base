#pragma once

typedef struct {
	int count;
	pthread_mutex_t mt;
//	pthread_mutexattr_t mtp;
	pthread_cond_t cond;
//	pthread_condattr_t condp;
}EVHAND;
typedef EVHAND* HANDLEEV;

class CEvent
{
public:
	CEvent();
	CEvent(HANDLEEV h);
	~CEvent();

	bool isEvent();
	HANDLEEV GetHandle();
	bool Attach(HANDLEEV h);
	bool Detach(HANDLEEV h);

	int WaitForSingleObject(unsigned long dwTimeOut);
	bool SetEvent();

	bool CreateEvent(void);
	void DestroyEvent();
private:
	HANDLEEV m_handleEvent;
	int m_nAttachCount;
};