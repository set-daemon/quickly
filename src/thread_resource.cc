#include "thread_resource.h"

__thread ThreadResource* g_thread_resource = NULL;

void* ThreadResource::get(const string& resource_name) {
	if (!g_thread_resource) {
		g_thread_resource = new ThreadResource();
	}

	ResourceMap::iterator iter_find = g_thread_resource->resources.find(resource_name); 
	if (iter_find != g_thread_resource->resources.end()) {
		return iter_find->second;
	}

	return NULL;
}

bool ThreadResource::set(const string& resource_name, void *resource) {
	if (!g_thread_resource) {
		g_thread_resource = new ThreadResource();
	}
	
	g_thread_resource->resources[resource_name] = resource;
}
