#include "scl/thread.h"
#include "scl/ring_queue.h"


namespace scl {

////////////////////////////////////////////////////////////////////////
// class task_thread
//
// ʹ��һ��thread������һ��task����
// �ڲ�ʹ��һ���ȴ�����Ļ��ζ���������δ�����task���ⲿ����task���̣߳�����ǵ��̣߳���add���task��������У�task_thread��������Ĺ����߳��������������Щtask
// ʹ��һ��������task��ɵĻ��ζ��������洦������Ϣ��������紦������task���̣߳�����ǵ��̣߳�����process_task_thread_result����������
//
// ����ʹ��ʾ����
//
//		#include "scl/task_thread.h"
//
//		using scl::task_thread;
//		using scl::task;
//
//		struct task_param { int i; };
//		struct task_result { int r;};
//
//		bool task_func(void* param, void* result)
//		{
//			task_param* p	= static_cast<task_param*>(param);
//			task_result* r	= static_cast<task_result*>(result);
//			return true;  //���ﷵ��true�ᵼ��task����ӵ�result�����С�ע�⣡���۷���true����false��task���ᱻ�Ӵ�����������Ƴ���
//		}
//
//		bool task_result_func(void* result)
//		{
//			task_result* r = static_cast<task_result*>(result);
//			return true;  //���ﷵ��true�ᵼ��task����result�������Ƴ�������false�򲻻ᱻ�Ƴ����´ε���process_task��Ȼ�ᴦ��
//		}
//
//		int main()
//		{
//			task_thread* pool = new task_thread[8];
//			int freeIndex = scl::find_free_task_thread(pool, 8);
//			task t(task_func, new task_param, new task_result);
//			if (freeIndex >= 0 && freeIndex < 8) pool[freeIndex]->add(t);
//			while (scl::process_task_thread_result(pool, 8, task_result_func) == 0);
//			delete t.param; delete t.result; delete[] pool;
//			return 0;
//		}
////////////////////////////////////////////////////////////////////////

class task_thread;

class task
{
public:
	typedef void (*funcT)(void*, void*, task_thread*);

	funcT	func;
	void*	param;
	void*	result;

	task() : func(NULL), param(NULL), result(NULL) {}
	task(funcT f, void* p = NULL, void* r= NULL) : func(f), param(p), result(r) {}
};

class task_thread
{
public:
#ifdef SCL_WIN
	static const int						MAX_TASK_COUNT = 256;
#endif
#ifdef SCL_LINUX
	static const int						MAX_TASK_COUNT = 10000;
#endif
#ifdef SCL_APPLE
	static const int						MAX_TASK_COUNT = 1024;
#endif
#ifdef SCL_ANDROID
	static const int						MAX_TASK_COUNT = 1024;
#endif
#ifdef SCL_HTML5
	static const int						MAX_TASK_COUNT = 256;
#endif

public:
	task_thread() : m_resultThreadID(-1), m_userData(NULL), m_processedCount(0), m_processedTime(0), m_index(-1), m_ignoreResult(false) {}
	~task_thread();

	void			start				();
	bool			add					(const task& t); 
	int				free_count			() const			{ return m_tasks.free() + m_results.free(); }
	bool			is_running			() const			{ return m_thread.is_running(); }
	bool			process_result		(task::funcT func, bool check_thread = false); 
	int				process_all_results	(task::funcT func, int maxProcessCount = -1); //maxProcessCount�����ɴ�������������Ϊ-1����ʾҪ��������������
	void			set_ignore_result	(bool v) { m_ignoreResult = v; }
	void			stop				();
	bool			has_result			() const { return m_results.used() > 0; }
	bool			has_task			() const { return m_tasks.used() > 0; }
	void			set_log_buffer_size	(const int v) { m_logBufferSize = v ;}

	//����ͳ������
	int				index				() const		{ return m_index;			}
	int				processed_count		() const		{ return m_processedCount;	}
	uint64			processed_time		() const		{ return m_processedTime;	}
	void			set_user_data		(void* data)	{ m_userData = data;		}
	void			set_index			(const int i)	{ m_index = i;				}
	int				task_count			()	const		{ return m_tasks.used();	}
	int				result_count		()	const		{ return m_results.used();	}
	
private:
	static void*	thread_func			(void* param, int* signal);

private:
	scl::thread								m_thread;
	scl::ring_queue<task, MAX_TASK_COUNT>	m_tasks;			//δ�����������
	scl::ring_queue<task, MAX_TASK_COUNT*2>	m_results;			//�Ѵ����������Ķ���
	int										m_resultThreadID;	//��������̵߳�thread id�����ڼ���������Ƿ���ͬһ���߳���ִ��
	volatile	void*						m_userData;			//�û��Զ�������
	volatile	int							m_processedCount;	//�Ѵ�������
	volatile	uint64						m_processedTime;	//�����һ������ʱ��ʱ��
	int										m_index;			//�̵߳�index
	bool									m_ignoreResult;		//���Դ�����
	int										m_logBufferSize;	//�߳��õ�����־��������С
};

//��һ��task_thread�������ҵ����ж�������task_thread���±�����
int				find_free_task_thread		(const task_thread* task_threads, const int max_thread_count);

//���̴߳������ĺ���
int				process_task_thread_result	(task_thread* task_threads, const int max_thread_count, task::funcT func);
int				process_task_thread_all_results(task_thread* task_threads, const int max_thread_count, task::funcT func, int maxProcessCount);

} //namespace scl

