/*
 * file name: configor.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2014-08-20
 * history  :
 *
 */
#ifndef __CONFIGOR_H__
#define __CONFIGOR_H__


#include <vector>
#include <string>
using namespace std;

#include "common_defs.h"
#include "double_buffer.h"
#include "rw_locker.h"

typedef bool (*load_func)(const string &file_name, void *context);

class Record {
public:
	Record() {}
	~Record() {}

	string 			file_name;
	unsigned int 	last_updated_time;
	load_func		reloader;
	void 			*context;
};
typedef vector<Record> RecordVec;

class ConfigorData {
public:
	ConfigorData() : check_interval(10), retry_times(3),
					 retry_interval(100000) {
	}

	~ConfigorData() { }

	void clear() {
		check_interval = 10;
		retry_times = 3;
		retry_interval = 100000;
	}

public:
	int				check_interval;	// unit with second.
	// if the file is loaded unsuccesfully, it need some reload retrying.
	int				retry_times;
	unsigned int	retry_interval;
};

class Configor {
public:
	~Configor();

	static Configor& get_inst() {
		static Configor configor;

		return configor;
	}

	void stop() {
		is_running = false;
	}

	Result_enums_t init(const string &file_name);

	static bool reload(const string &file_name, void *context);

	bool regist(const string &file_name, void *context,
				load_func reloader, bool run_imediately);

	friend void* configor_cb(void *arg);

private:
	Configor();

	bool running() {
		return is_running;
	}

private:
	RecordVec 					records;
	DoubleBuffer<ConfigorData>	config_data;
	bool						is_running;
	RwLocker					locker;
};

#endif // __CONFIGOR_H__
