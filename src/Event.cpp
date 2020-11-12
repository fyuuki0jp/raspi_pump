

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <chrono>
#include <errno.h>   
#include "datatype.h"
#include "Event.h"

CEvent::CEvent()
	: m_handleEvent(NULL)
{
}

CEvent::CEvent(HANDLEEV h)
{
	Attach(h);
}

CEvent::~CEvent()
{
	if (m_handleEvent)
	{
		if (m_handleEvent->count > 0)
			m_handleEvent->count--;

		DestroyEvent();
	}
}

bool CEvent::isEvent()
{
	if (m_handleEvent)
		return true;

	return false;
}

HANDLEEV CEvent::GetHandle()
{
	return m_handleEvent;
}

bool CEvent::Attach(HANDLEEV h)
{
	if (!isEvent())
	{
		h->count++;
		m_handleEvent = h;
		return true;
	}

	return false;
}

bool CEvent::Detach(HANDLEEV h)
{
	if (!isEvent())
	{
		if (m_handleEvent == h)
		{
			if (m_handleEvent->count > 0)
				m_handleEvent->count--;

			if (m_handleEvent->count == 0)
				return true;
		}
	}

	return false;
}

int CEvent::WaitForSingleObject(unsigned long dwTimeOut)
{
	if (!isEvent())
		return -1;

	timespec t;
	std::chrono::milliseconds msec(dwTimeOut);
	auto now = std::chrono::system_clock::now();
	std::chrono::nanoseconds nano = now.time_since_epoch() + msec;

	t.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(nano).count();
	std::chrono::seconds sec(t.tv_sec);
	t.tv_nsec = (nano - sec).count();
	pthread_mutex_lock(&m_handleEvent->mt);

	int ret = pthread_cond_timedwait(&m_handleEvent->cond, &m_handleEvent->mt, &t);

	if (ret != 0)
	{
//		fprintf(stderr, "timedwait : %d %d %d\n", ret, EAGAIN, ETIMEDOUT);

		switch (ret)
		{
		case EAGAIN:
		case ETIMEDOUT:
			ret = 1;
			break;

		default:
			ret = -1;
			fprintf(stderr, "CEvent error\n");
		}
	}

	pthread_mutex_unlock(&m_handleEvent->mt);

	return ret;
}

bool CEvent::CreateEvent(void)
{
	if (isEvent())
		return false;

	m_handleEvent = new EVHAND;

	pthread_mutex_init(&m_handleEvent->mt, NULL);
	pthread_cond_init(&m_handleEvent->cond, NULL);

	return true;
}

bool CEvent::SetEvent()
{
	if (!isEvent())
		return false;

	pthread_mutex_lock(&m_handleEvent->mt);

	pthread_cond_signal(&m_handleEvent->cond);

	pthread_mutex_unlock(&m_handleEvent->mt);

	return true;
}

void CEvent::DestroyEvent()
{
	if (!isEvent())
		return;

	if (m_handleEvent->count > 0)
		return;

	pthread_mutex_destroy(&(m_handleEvent->mt));
	pthread_cond_destroy(&(m_handleEvent->cond));

	delete m_handleEvent;

	m_handleEvent = NULL;
}
