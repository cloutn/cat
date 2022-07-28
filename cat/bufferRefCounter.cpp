//#include "cat/bufferRefCounter.h"
//
//namespace cat {
//
//void BufferRefCounter::AddRef(void* buffer)
//{
//	std::map<void*, int>::iterator it = m_counter.find(buffer);
//	if (it == m_counter.end())
//		m_counter.insert(std::make_pair(buffer, 1));
//	else
//		it->second++;
//}
//
//int BufferRefCounter::DecRef(void* buffer)
//{
//	std::map<void*, int>::iterator it = m_counter.find(buffer);
//	if (it != m_counter.end())
//	{
//		it->second--;
//		return it->second;
//	}
//	return 0;	
//}
//
//int BufferRefCounter::GetRefCounter(void* buffer)
//{
//	std::map<void*, int>::iterator it = m_counter.find(buffer);
//	if (it != m_counter.end())
//		return it->second;
//	return 0;
//}
//
//} // namepsace cat

