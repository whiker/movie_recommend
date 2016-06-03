#ifndef CLASS_IdAtomic
#define CLASS_IdAtomic

#include <pthread.h>

class IdAtomic
{
private:
	pthread_mutex_t m_mtx;
	int m_id;

public:
	IdAtomic(int id)
	:m_id(id), m_mtx(PTHREAD_MUTEX_INITIALIZER) { }

	int get()
	{
		int id;
		pthread_mutex_lock(&m_mtx);
		id = m_id++;
		pthread_mutex_unlock(&m_mtx);
		return id;
	}

	void set(int id)
	{
		pthread_mutex_lock(&m_mtx);
		m_id = id;
		pthread_mutex_unlock(&m_mtx);
	}
};

#endif