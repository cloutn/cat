////////////////////////////////////////////////////////////////////////////////
//	ʱ�����
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/assert.h"
#include "scl/pstring.h"
#include "scl/string.h"

#include "scl/thread.h"

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
#include <time.h>
#endif

namespace scl {

//��ȡϵͳ��ǰTick����λ����(ms)
uint64 	get_realtime_tick();	// get current real time with system API
time_t	get_realtime_time();

#if defined(SCL_ANDROID) || defined(SCL_APPLE) || defined(SCL_HTML5)
#define SCL_TICK scl::get_realtime_tick()
#define SCL_TIME scl::get_realtime_time() 
#endif

#if defined(SCL_WIN) || defined(SCL_LINUX)
#define SCL_TICK ((scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_TICK == 0) ? (scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_TICK = scl::_scl__inner__init_tick()) : scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_TICK)
#define SCL_TIME ((scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_TIME == 0) ? (scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_TIME = scl::_scl__inner__init_time()) : scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_TIME)
#endif


#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
#define SCL_MILLISECOND (int(SCL_TICK % 1000))
#endif
#ifdef SCL_WIN
#define SCL_MILLISECOND ((scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_MILLISECOND == 0) ? (scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_MILLISECOND = scl::_scl__inner__init_millisecond()) : scl::_DO_NOT_USE_THIS_SCL_INNER_THREAD_MILLISECOND)
#endif

//����
void sleep(uint32 milliseconds);	//��λ����(ms)
void usleep(uint32 useconds);		//��λ΢��(us)

//��ȡ�ַ�����ʽΪ 12:30 ���͵�ʱ��
bool string_to_time(const char* timeStr, int& hour, int& min);
//��ȡ�ַ�����ʽΪ 1984-03-21 ���͵�����
bool string_to_date(const char* dataStr, uint& year, uint& month, uint& day);

//ʱ��������
class time
{
public:
	time() : m_time(0), second(0), minute(0), hour(0), day(0), month(0), year(0), dayOfWeek(0), dayOfYear(0), daylightSavingTime(0), millisecond(0) {}
	time(uint64 t) : m_time(static_cast<time_t>(t)) { do_local(); }
	time(uint64 t, int _millisecond) : m_time(static_cast<time_t>(t)) { do_local(_millisecond);  }

	void			set(uint64 t) { m_time = static_cast<time_t>(t); do_local(); }
	uint64			time_stamp() const;
	const time& 	now();
	void			to_string	(char* output, const int outputCapacity, bool withYear = true, bool withMillisecond = true) const;
	void			to_string	(string32& output, bool withYear = true, bool withMillisecond = true) const { return to_string(output.c_str(), output.capacity(), withYear, withMillisecond); }
	void			to_filename	(char* output, const int outputCapacity, bool withYear = true, bool withMilliSecond = true) const;
	void			do_local	(int _millisecond = -1);
	
	::time_t m_time;

	uint second;				// seconds after the minute - [0,59] 
	uint minute;				// minutes after the hour - [0,59] 
	uint hour;					// hours since midnight - [0,23] 
	uint day;					// day of the month - [1,31]
	uint month;					// months since January - [1,12] 
	uint year;					// years since 0
	uint dayOfWeek;				// days since Sunday - [0,6] 
	uint dayOfYear;				// days since January 1 - [0,365] 
	uint daylightSavingTime;	// daylight savings time flag 
	uint millisecond;			// millisecond
};

////////////////////////////////////
//[��ʱ��1]
//ʵ�֣�
//		�ڲ������ˡ��´�tick��ʱ��m_nextTick����ÿ��ring��ʱ�򶼻����m_nextTick
//		����m_nextTickʱ���ڲ������scl::getTick()����
//�е㣺
//		����timer2������ring������Ҫһ������������������򵥷���һЩ
//ȱ�㣺
//		����������scl::getTick���������Ե�getTick�����ʱ��ᵼ�³�ʱ�䲻�ᴥ��ring
////////////////////////////////////
class timer
{
public:
	timer() : m_nextTick(0), m_interval(0), m_paused(false), m_lastPauseTick(0), m_update_to_now(false) {}

	void start				(int interval);		//������ʱ������λ������;
	void start				(); 				//������ʱ������λ������;
	bool ring				();					//�Ƿ�����
	void pause				();					//��ͣ��ʱ��
	void resume				();					//�ָ�����ͣ�ļ�ʱ��

	void set_interval		(const int interval) { m_interval = interval; }
	int	 get_interval		() { return m_interval; }
	bool is_start			();					//�Ƿ��Ѿ���ʼ
	bool is_paused			();					//�Ƿ��Ѿ���ͣ
	void set_update_to_now	(bool value) { m_update_to_now = value; } //ÿ��ring��ʱ�򣬸���ʱ���Ƿ�Ϊnow + interval������ֵΪtrue��ʱ��������μ��ring��ʱ�����2��interval����ֻ��ring1��

private:
	uint64	m_nextTick;
	int		m_interval;

	//pause�߼����
	bool	m_paused;
	uint64	m_lastPauseTick;
	bool	m_update_to_now;		//ÿ��ring��ʱ�򣬸���ʱ���Ƿ�Ϊnow + interval������ֵΪtrue��ʱ��������μ��ring��ʱ�����2��interval����ֻ��ring1��
};

////////////////////////////////////
//[��ʱ��2]
//ʵ�֣�
//		�ڲ������ˡ���һ��tick�ڣ���ǰ�Ѿ����ŵ�ʱ��m_currentTick��
//		������ʱ�䵽�˵�ʱ�򣬽�����ʱ���ȥһ��tick�ĳ��ȣ�Ȼ�����
//�ŵ㣺
//		��������scl::get_tick����
//ȱ�㣺
//		ring��Ҫ����һ������diff����ʾ�ϴε���ring����ε���ring��ʱ����
//		�������һ�����ͨ����֮֡���ʱ������ȷ��
////////////////////////////////////
class timer2
{
public:
	timer2() : m_current_tick(0), m_interval(0) {}

	void	start			(const uint64 interval); 								//������ʱ������λ������;
	void	start			(); 													//������ʱ������λ������;
	bool	ring			(const uint64 diff);									//�Ƿ�����;
	void	pause			()						{ m_paused = true;			}	//��ͣ��ʱ��
	void	resume			()						{ m_paused = false;			}	//�ָ�����ͣ�ļ�ʱ��

	void	set_interval	(const uint64 interval) { m_interval = interval;	}
	uint64	get_interval	() const				{ return m_interval;		}
	uint64	get_current_tick() const				{ return m_current_tick;	}
	bool	is_start		() const				{ return m_interval > 0;	}	//�Ƿ��Ѿ���ʼ
	bool	is_paused		() const				{ return m_paused;			}	//�Ƿ��Ѿ���ͣ

private:
	uint64	m_current_tick;
	uint64	m_interval;
	bool	m_paused;		//�Ƿ���ͣ
};

// �����࣬ʱ���
class time_span
{
public:
	time_span	();
	time_span	(uint64 begin, uint64 end);
	void	offset		(int sec);											// ʱ�������ƫ��sec��
	int		compare		(uint64 t) const;									// ����strcmp. ����t�Ƿ���ʱ�����,�ڵĻ�,����0,��û�ﵽ,����<0,�Ѿ���������>0
	bool	contains	(uint64 t) const { return compare(t) == 0; };		//�Ƿ����ʱ���
	uint64	begin		() const { return m_begin; }
	uint64	end			() const { return m_end; }

private:
	uint64 m_begin;
	uint64 m_end;
};

// get tick which has been written by time thread, if TICK == 0 then start time thread and wait until TICK > 0
uint64	_scl__inner__init_tick();				
time_t	_scl__inner__init_time();				
#ifdef SCL_WIN
int		_scl__inner__init_millisecond();				
#endif

// thread tick, don't use this directly, use SCL_TICK macro instead.
extern volatile uint64 	_DO_NOT_USE_THIS_SCL_INNER_THREAD_TICK;  
extern volatile time_t	_DO_NOT_USE_THIS_SCL_INNER_THREAD_TIME;  
#ifdef SCL_WIN
extern volatile int		_DO_NOT_USE_THIS_SCL_INNER_THREAD_MILLISECOND; 
#endif

int timezone(); //���ص�ǰʱ��-12~+12��ΪUTC���෴�������籱����UTC+8, �����ֵ��Ϊ-8


} //namespace scl


