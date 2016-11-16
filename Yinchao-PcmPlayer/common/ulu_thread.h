#ifndef __ULU_THREAD_H__
#define __ULU_THREAD_H__

#include "string.h"

#ifdef __APPLE__

#ifndef _IOS
#define _IOS
#endif //_IOS

#else

#ifdef ANDROID
#ifndef _LINUX
#define _LINUX
#endif //_LINUX

#ifndef _LINUX_ANDROID
#define _LINUX_ANDROID
#endif //_LINUX_ANDROID
#endif //ANDROID

#endif //__APPLE__

typedef unsigned int (* ptr_thread_func) (void * pParam);
typedef void * THREAD_HANDLE;

unsigned int create_thread( THREAD_HANDLE * ptr_handle, unsigned int * ptr_id, ptr_thread_func ptr_func, void * pParam, unsigned int uFlag, unsigned int stack_size = 131072/*128k default*/);
void exit_thread();
void wait_thread_exit( THREAD_HANDLE thread_handle );
void set_threadname( const char * ptr_name );
int get_current_threadid (void);

class ulu_thread
{
public:
	ulu_thread()
		:m_thread_handle(0)
		,m_is_thread_running( false )
	{
		memset( m_thread_name , 0 , sizeof( m_thread_name ) );
	}

	virtual ~ulu_thread()
	{
		ut_stop();
	}

public:
	virtual bool ut_begin(unsigned int stack_size = 131072/*128k default*/, char * ptr_thread_name = 0)
	{
		unsigned int thread_id;

		ut_stop();

		if( ptr_thread_name )
			strcpy( m_thread_name , ptr_thread_name );
		else
			memset( m_thread_name , 0 , sizeof( m_thread_name ) );

		return (0 == create_thread( &m_thread_handle , &thread_id , ut_threadfunc , this , 0 ,stack_size));
	}

	virtual void ut_stop()
	{
		wait_thread_exit( m_thread_handle );
		m_thread_handle = 0;
	}

protected:
	static unsigned int ut_threadfunc( void * pParam )
	{
		ulu_thread * ptr_thread = (ulu_thread *)pParam;

		ptr_thread->m_is_thread_running = true;
		
		if( strlen( ptr_thread->m_thread_name ) > 0 )
			set_threadname( ptr_thread->m_thread_name );

		ptr_thread->ut_thread_function();

		ptr_thread->m_is_thread_running = false;

		exit_thread();

		return 0;
	}

	virtual void ut_thread_function() = 0;

	bool ut_is_thread_running()
	{
		return m_is_thread_running;
	}

protected:
	THREAD_HANDLE m_thread_handle;
	char		  m_thread_name[256];
	bool		  m_is_thread_running;
};

// End define
#ifdef _ULUNAMESPACE
}
#else
#endif /* _ULUNAMESPACE */

#endif