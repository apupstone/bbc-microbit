#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdint.h>

static const uint16_t CIRCULAR_BUFFER_MAX_CAPACITY = 256;

template <class T>
class CircularBuffer
{
	public:
		CircularBuffer(uint16_t capacity);
		void reset();
		bool put(T item, bool overwrite=false);
		bool get(T* item);
		bool is_empty();
		bool is_full();
		uint16_t get_capacity();
		uint16_t get_size();
	
	private:
		void advance_pointer();
		void retreat_pointer();
	
		T m_buffer[CIRCULAR_BUFFER_MAX_CAPACITY];
		uint16_t m_head;
		uint16_t m_tail;
		uint16_t m_capacity;
		bool m_full;
};


template<class T>
CircularBuffer<T>::CircularBuffer(uint16_t capacity)
: m_head(0), m_tail(0), m_capacity(capacity), m_full(false)
{

}


template<class T>
void CircularBuffer<T>::reset()
{
	m_head = 0;
	m_tail = 0;
	m_full = false;
}


template<class T>
bool CircularBuffer<T>::is_full()
{
	return m_full;
}


template<class T>
bool CircularBuffer<T>::is_empty()
{
	return (!m_full && (m_head == m_tail));
}


template<class T>
uint16_t CircularBuffer<T>::get_capacity()
{
	return m_capacity;
}


template<class T>
uint16_t CircularBuffer<T>::get_size()
{
	uint16_t size = m_capacity;
	
	if (!m_full)
	{
		if (m_head >= m_tail)
		{
			size = m_head - m_tail;
		}
		else
		{
			size = m_capacity + m_head - m_tail;
		}
	}
	
	return size;
}


template<class T>
void CircularBuffer<T>::advance_pointer()
{
	if (m_full)
	{
		m_tail = (m_tail + 1) % m_capacity;
	}
	
	m_head = (m_head + 1) % m_capacity;
	m_full = (m_head == m_tail);
}


template<class T>
void CircularBuffer<T>::retreat_pointer()
{
	m_full = false;
	m_tail = (m_tail + 1) % m_capacity;
}


template<class T>
bool CircularBuffer<T>::put(T item, bool overwrite)
{
	if (!overwrite && m_full)
	{
		return false;
	}
	
	m_buffer[m_head] = item;
	advance_pointer();
	return true;
}


template<class T>
bool CircularBuffer<T>::get(T* item)
{
	if (is_empty())
	{
		return false;
	}
	
	*item = m_buffer[m_tail];
	retreat_pointer();
	return true;
}

#endif