////////////////////////////////////////////////////////////////////////////////
//	thread.h
//	Thread��
//	2010.09.11 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/ptr.h"

namespace scl {

const int INVALID_THREAD_ID = -1;

#ifdef SCL_WIN
const int MAX_THREAD_COUNT = 64; // ����߳�����
#endif

#ifdef SCL_LINUX
const int MAX_THREAD_COUNT = 256; // ����߳�����
#endif

#ifdef SCL_APPLE
const int MAX_THREAD_COUNT = 64; // ����߳�����
#endif

#ifdef SCL_ANDROID
const int MAX_THREAD_COUNT = 64; // ����߳�����
#endif

#ifdef SCL_HTML5
const int MAX_THREAD_COUNT = 64; // ����߳�����
#endif


class thread_info;

////////////////////////////////////////////////////////////////////////////////
//	class thread
////////////////////////////////////////////////////////////////////////////////
class thread
{
public:
	typedef void* (*ThreadFunction)(void*, int*);

public:
	thread();
	~thread();

	int				start				(ThreadFunction function, void* param = NULL, bool start_at_once = true, bool auto_detach = false);
	void			resume				();
	void			set_affinity_mask	(int cpuid = 0);
	bool			is_running			() const;
	bool			is_main_thread		() const;

	void			send_stop_signal	();
	bool			wait				(const int timeout = -1, bool check_thread = true);  // wait for thread close, timeout is millionseconds, if timeout == -1 wait until thread exits.
	bool			force_kill			(); //ǿ��ɱ���̣߳��ǳ�����ȫ

	//static members
	static	int		self				();			//�߳�id
	static	int		self_process		(); //����id
	static	int		self_index			();	//�߳���g_thread_infos�����е�index
	static	bool	exists				(const int thread_id);

	static	int		main_thread_id		()	{ return m_main_thread_id; }


public:
	enum SIGNAL
	{
		SIGNAL_NORMAL	= 0, //��������
		SIGNAL_STOP		= 1, //ֹͣ
		SIGNAL_HUNG		= 2, //����
	};

	class info
	{
	public:
		void*					handle;				//�߳��ڲ���ϵͳ�еľ��

		uint					process_id;			//����id
		int						id;					//�߳�id
		int						index;				//�߳���g_threads�е�index

		uint64					pthread_id;			//pthread�߳�id  ����linux����Ч

		int						parent_thread_id;	//���̵߳�id

		thread::ThreadFunction	function;			//�̵߳�ִ�к���
		void*					param;				//�߳�ִ�к����Ĳ���
		int						signal;				//�߳��źţ������߳�ͨ����������źź��̺߳���ͨ��

		bool					auto_detach;		//�̺߳����Ƴ����߳���Դ�Ƿ��Զ��ͷţ�ע�⣬�����ֵΪfalse����ô�����̣߳�thread::start�����ͷ��߳�(thread::start)���̱߳�����ͬһ���߳�
		void*					exit_code;			//�̺߳����˳�����
		volatile bool			is_running;			//�̺߳����Ƿ���������

		void clear()
		{
			handle		= NULL;

			process_id	= -1;
			id			= -1;
			index		= -1;
			pthread_id	= -1;

			function	= NULL; 
			param		= NULL; 
			signal		= 0; 

			exit_code	= NULL;
			auto_detach = false;
			is_running	= false;
		}
	};

private:
	static int __init_main_thread_info();

private:
	info		m_info;
	static int	m_main_thread_id;
};

template <typename T>
class class_thread : public thread
{
public:
	struct class_thread_info
	{
		class_function	function;
		void*			ptr;

		class_thread_info() :
			function	(NULL), 
			ptr			(NULL)	{}
	};

	typedef void* (T::*functionT)(int*);
	int start(functionT func, T* param = NULL, bool startAtOnce = true)
	{
		m_class_info.function	= reinterpret_cast<class_function>(func);
		m_class_info.ptr		= static_cast<void*>(param);
		return thread::start(_class_thread_function_dispatcher, this, startAtOnce);
	}

private:
	static void* _class_thread_function_dispatcher(void* param, int* singal);
	class_thread_info	m_class_info;
};


template <typename T>
void* scl::class_thread<T>::_class_thread_function_dispatcher(void* param, int* signal)
{
	class_thread*	t		= static_cast<class_thread*>(param);
	T*				c		= static_cast<T*>(t->m_class_info.ptr);
	functionT		func	= reinterpret_cast<functionT>(t->m_class_info.function);

	return (c->*func)(signal);
}

class mutex
{
public:
	mutex		();
	~mutex		();
	void create	();
	void lock	();
	void unlock	();
private:
	void*		m_handle;
};

class mutex_lock
{
public:
	mutex_lock	(mutex* m);
	~mutex_lock	();
	void unlock	();
private:
	mutex*		m_mutex;
};


//�ź���
class semaphore
{
public:
	semaphore();
	~semaphore();
	bool create(const int count = 0);
	void post();
	bool wait();

private:
	void* m_handle;
};



bool compare_and_swap(volatile uint* i,		uint oldval,	uint newval);
bool compare_and_swap(volatile int* i,		int oldval,		int newval);
bool compare_and_swap(volatile ptr_t* i,	ptr_t oldval,	ptr_t newval);

int atomic_inc(volatile int* i);
int atomic_dec(volatile int* i);

extern mutex __inner_error_log_mutex;

} //namespace scl

