/*
 * file name: double_buffer.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2014-08-28
 * history  :
 */
#ifndef __DOUBLE_BUFFER_H__
#define __DOUBLE_BUFFER_H__

template <typename T>
class DoubleBuffer {
public:
	DoubleBuffer() : current_buffer_index(0) {
	}

	~DoubleBuffer() {
	}

public:
	typedef T DataType;

	DataType& get() {
		return buffer[current_buffer_index];
	}

	DataType& get_next() {
		return buffer[1 - current_buffer_index];
	}

	void alter() {
		current_buffer_index = 1 - current_buffer_index;
	}

private:
	int			current_buffer_index;
	DataType	buffer[2];
};


#endif // __DOUBLE_BUFFER_H__
