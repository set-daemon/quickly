/*
 * file name: msg_que.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2014-09-13
 * history  :
 */
#ifndef __MSG_QUE_H__
#define __MSG_QUE_H__

#include <list>
#include <string>
using namespace std;

#include "rw_locker.h"

template <typename ElemType>
class MsgQue {
public:
	MsgQue(int size_limit) : que_max_limit(size_limit) {
	}
	~MsgQue() {
	}

	int get_msg_num() {
		return que.size();
	}

	ElemType* get() {
		ElemType* elem = NULL;
		if (locker.get_ab_read_permission()) {
			if (que.size() > 1) {
				elem = &que.front();
			}
			locker.release_permission();
		} else {
		}

		return elem;
	}

	bool del(ElemType* elem) {
		if (locker.get_ab_write_permission()) {
			if (NULL == elem) {
				que.pop_front();
			} else {
				typename list<ElemType>::iterator iter = que.begin();
				while (iter != que.end()) {
					if (&(*iter) == elem) {
						que.erase(iter);
						break;
					}
				}
			}

			locker.release_permission();
			return true;
		}

		return false;
	}

	bool add(ElemType &data) {
		if (locker.get_ab_write_permission()) {
			if (que.size() >= que_max_limit) {
				locker.release_permission();
				return false;
			}

			que.push_back(data);
			locker.release_permission();
		} else {
			return false;
		}

		return true;
	}

private:
	int					que_max_limit;
	RwLocker			locker;
	list<ElemType>		que;
};

#endif // __MSG_QUE_H__
