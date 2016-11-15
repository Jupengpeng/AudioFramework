#include "ulu_thread.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#elif defined _LINUX
#include <sys/mman.h>
#include <pthread.h>
#include <sys/prctl.h>
#elif defined _IOS
#include <pthread.h>
#elif defined _MAC_OS || defined __APPLE__
#include <pthread.h>
#endif

#define ULU_DEFAULT_STACKSIZE (128 * 1024)

void set_threadname(const char * ptr_name )
{
#ifdef _LINUX_ANDROID
		prctl(PR_SET_NAME, (unsigned long)ptr_name , 0 , 0 , 0);
#endif
}

int get_current_threadid (void)
{
#ifdef WIN32
	return (int) GetCurrentThreadId ();
#elif defined __LINUX
	return (int) pthread_self ();
#elif defined _MAC_OS || defined _IOS || defined __APPLE__
	return (signed long) pthread_self ();
#endif // WIN32
}

unsigned int create_thread( THREAD_HANDLE * ptr_handle, unsigned int * ptr_id, ptr_thread_func ptr_func, void * ptr_param, unsigned int uFlag, unsigned int stack_size)
{
	if (ptr_handle == NULL || ptr_id == NULL)
		return 1;

	*ptr_handle = NULL;
	*ptr_id = 0;
#ifdef WIN32
	*ptr_handle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) ptr_func, ptr_param, 0, (LPDWORD)ptr_id);
	if (*ptr_handle == NULL)
		return -1;
#elif defined _LINUX
	pthread_t 		tt;

	pthread_attr_t	attr;
#if !defined NNJ
	attr.flags = 0;
	attr.stack_base = NULL;
	attr.stack_size = stack_size; //DEFAULT_STACKSIZE;
	attr.guard_size = PAGE_SIZE;

	if (uFlag == 0)
	{
		attr.sched_policy = SCHED_NORMAL;
		attr.sched_priority = 0;
	}
	else
	{
		attr.sched_policy = SCHED_RR;
		attr.sched_priority = uFlag;
	}
#else 

#define PAGE_SIZE   (1UL << 12)
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, stack_size);
	pthread_attr_setguardsize(&attr, PAGE_SIZE);

	struct sched_param param;
	if (uFlag == 0) {
		pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
		param.sched_priority = 0;
	} else {
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
		param.sched_priority = uFlag;
	}
	pthread_attr_setschedparam(&attr, &param);
#endif


	int rs = pthread_create(&tt, &attr, (void*(*)(void*))ptr_func ,ptr_param);
#if defined NNJ
	pthread_attr_destroy(&attr);
#endif
	if (rs != 0)
		return -1;

	*ptr_handle = (THREAD_HANDLE)tt;
	*ptr_id = (unsigned int)tt;
	
#elif defined __APPLE__
	pthread_attr_t  attr;
    pthread_t       posixThreadID;
    int             returnVal;
	
/*	attr.flags = 0;
	attr.stack_base = NULL;
	attr.stack_size = VO_DEFAULT_STACKSIZE; //DEFAULT_STACKSIZE;
	attr.guard_size = PAGE_SIZE;
	
	if (uFlag == 0)
	{
		attr.sched_policy = SCHED_NORMAL;
		attr.sched_priority = 0;
	}
	else
	{
		attr.sched_policy = SCHED_RR;
		attr.sched_priority = uFlag;
	}
 */
	
    returnVal = pthread_attr_init(&attr);
    //returnVal = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&posixThreadID, &attr, (void*(*)(void*))ptr_func, ptr_param);
	*ptr_handle	= (THREAD_HANDLE)posixThreadID;
	//*ptr_id		= (unsigned int)posixThreadID;
#elif defined _MAC_OS
	pthread_attr_t  attr;
    pthread_t       posixThreadID;
    int             returnVal;
    returnVal = pthread_attr_init(&attr);
    pthread_create(&posixThreadID, &attr, (void*(*)(void*))ptr_func, ptr_param);
	*ptr_handle	= (THREAD_HANDLE)posixThreadID;
	*ptr_id		= (unsigned int)posixThreadID;	
#endif

	return 0;
}

void exit_thread()
{
#ifdef _LINUX
	int ret;
	pthread_exit( &ret );
#elif defined _IOS
	int ret;
	pthread_exit( &ret );
#endif
}

void wait_thread_exit( THREAD_HANDLE thread_handle )
{
	if( thread_handle == 0 )
		return;

#ifdef WIN32
	WaitForSingleObject( thread_handle , INFINITE );
#elif defined _LINUX
	pthread_join( (pthread_t)thread_handle , 0 );
#elif defined _IOS	
	// need test this function on iOS
	pthread_join( (pthread_t)thread_handle , 0 );
#elif defined _MAC_OS
	// need test this function on Mac OS X
	pthread_join( (pthread_t)thread_handle , 0 );	
#endif
}


// End define
#ifdef _ULUNAMESPACE
}
#endif