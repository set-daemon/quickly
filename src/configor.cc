#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>
#include <pthread.h>

#include "debug.h"
#include "ini_file.h"
#include "configor.h"
#include "log.h"

Configor::Configor() : is_running(true) {
}

Configor::~Configor() {
}

static bool is_update_needed(Record& record, unsigned int &cur_file_time) {
	struct stat f_st;

	if (stat(record.file_name.c_str(), &f_st) != 0) {
		return false;
	}

	if (!S_ISREG(f_st.st_mode)) {
		return false;
	}

	cur_file_time = f_st.st_mtime;

	if (cur_file_time <= record.last_updated_time) {
		return false;
	}

	return true;
}

void* configor_cb(void *arg) {
	Configor &configor = *(Configor*)arg;

	while (configor.running()) {
		ConfigorData& cdata = configor.config_data.get();

		if (true != configor.locker.get_ab_write_permission()) {
			sleep(cdata.check_interval);
			continue;
		}
		RecordVec::iterator iter = configor.records.begin();
		for (; iter != configor.records.end(); ++iter) {
			unsigned int file_mtime = 0;
			if (!is_update_needed(*iter, file_mtime)) {
				continue;
			}

			// try to reload the file.
			int retried_times = cdata.retry_times;
			do {
				// note that: if the reloader works successfully, it is responsible to
				// update current usable buffer as double-buffer used.
				bool update_result = iter->reloader(iter->file_name, iter->context);
				if (update_result) {
					break;
				}
				usleep(cdata.retry_interval);
			} while ((--retried_times) > 0);
			if (retried_times <= 0) { // that file failed to be loaded.
				Log::inst().error("%s,%d %s can not be loaded successfully\n",
						__FUNCTION__, __LINE__, iter->file_name.c_str());
			} else {
				iter->last_updated_time = file_mtime;
				Log::inst().error("%s,%d %s has been loaded successfully\n",
						__FUNCTION__, __LINE__, iter->file_name.c_str());
			}
		}
		configor.locker.release_permission();

		// TODO: it should be more precisely for the consumation of file loading.
		sleep(cdata.check_interval);
	}

	return arg;
}

bool Configor::reload(const string &file_name, void *context) {
	DoubleBuffer<ConfigorData>& cdata = *(DoubleBuffer<ConfigorData> *)context;

	ConfigorData& next_data = cdata.get_next();
	next_data.clear();

	IniFile ini_processor;
	if (true != ini_processor.init(file_name)) {
		Log::inst().error("%s,%d can not process file[%s] successfully!\n",
				__FUNCTION__, __LINE__, file_name.c_str());
		return false;
	}

	next_data.check_interval = ini_processor.get("configor", "check-interval", 10);
	Log::inst().debug("check-interval = %d\n", next_data.check_interval);
	next_data.retry_times = ini_processor.get("configor", "retry-times", 10);
	Log::inst().debug("retry-times = %d\n", next_data.retry_times);
	next_data.retry_interval = ini_processor.get("configor", "retry-times", 100000);
	Log::inst().debug("retry-interval = %d\n", next_data.retry_interval);

	cdata.alter(); // alter next buffer index to current buffer index
	
	return true;
}

Result_enums_t Configor::init(const string &file_name) {

	// register itself.
	if (true != regist(file_name, &config_data, Configor::reload, true)) {
		return E_FAIL;
	}

	pthread_t th;
	if (0 != pthread_create(&th, NULL, configor_cb, this)) {
		Log::inst().error("%s,%d it can not create thread[%s,%d]\n",
				__FUNCTION__, __LINE__, strerror(errno), errno);
		stop();
		return E_FAIL;
	}

	return E_OK;
}

// notify: this function is not thread-safety.
bool Configor::regist(const string &file_name, void *context,
					  load_func reloader, bool run_imediately) {
	if (true != locker.get_ab_write_permission()) {
		return false;
	}	
	RecordVec::iterator iter = records.begin();
	for (; iter != records.end(); iter++) {
		if (file_name.compare(iter->file_name) == 0) {
			locker.release_permission();
			Log::inst().error("%s,%d sorry, system does not support " 
					"duplicated file name[%s] now, it will be ok later!\n",
					 __FUNCTION__, __LINE__, file_name.c_str());
			return false;
		}
	}
	// TODO: check the file is ok?
	struct stat f_st = { };
	if (0 != stat(file_name.c_str(), &f_st)) {
		Log::inst().error("%s,%d can not get stat of file [%s]\n",
				__FUNCTION__, __LINE__, file_name.c_str());
		locker.release_permission();
		return false;
	}
	if (!S_ISREG(f_st.st_mode)) {
		Log::inst().error("%s,%d file [%s] is not a regular file\n",
				__FUNCTION__, __LINE__, file_name.c_str());
		locker.release_permission();
		return false;
	}

	Record rec;
	rec.file_name = file_name;
	rec.last_updated_time = f_st.st_mtime;
	rec.reloader = reloader;
	rec.context = context;

	records.push_back(rec);

	if (run_imediately) {
		reloader(file_name, context);
	}

	locker.release_permission();
	return true;
}

