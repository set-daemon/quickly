/*
 * file name: thread_resource.h
 * purpose  : 线程局部资源
 * author   : set_daemon@126.com
 * date     :
 * history  :
 */
#ifndef __THREAD_RESOURCE_H__
#define __THREAD_RESOURCE_H__

#include <map>
#include <string>
using namespace std;

class ThreadResource {
public:
	ThreadResource() {
	}
	~ThreadResource() {
	}

	typedef void* Resource;
	typedef map<string, Resource> ResourceMap;

	static void* get(const string& resource_name);

	static bool set(const string& resource_name, void *resource);

public:
	// resource name, resource
	ResourceMap resources;
};

#endif // __THREAD_RESOURCE_H__
