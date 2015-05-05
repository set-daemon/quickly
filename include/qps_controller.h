/*
 * file name: qps_controller.h
 * author   : set_daemon@126.com
 * date     :
 * history  :
 */
#ifndef __QPS_CONTROLLER_H__
#define __QPS_CONTROLLER_H__

#include "double_buffer.h"
#include "rw_locker.h"

class QpsElem {
public:
	QpsElem() {
	}
	~QpsElem() {
	}

	void clear() {
		on_effect = 0;
	}

public:
	string 	elem_name;
	int		intv_seconds;
	int		max_count;
	int		on_effect;
};

class QpsConfig {
public:
	QpsConfig() {
	}
	~QpsConfig() {
	}

	// 尽量保留以前的内存而只清理原内存中的数据，以防止内存的非法访问
	void clear() {
		map<string, QpsElem>::iterator iter = elems.begin();
		for (; iter != elems.end(); ++iter) {
			iter->second.clear();
		}
		on_effect = 0;
	}

	QpsElem* get(const string& elem_name) {
		map<string, QpsElem>::iterator find_iter = elems.find(elem_name);
		if (find_iter == elems.end()) {
			return NULL;
		}
		
		return &find_iter->second;
	}

public:
	map<string, QpsElem> elems;
	int					 on_effect;
};
typedef DoubleBuffer<QpsConfig> DbQpsConfigs;

class RunStat {
public:
	RunStat() {
		last_time = time(NULL);
		count = 0;
	}
	~RunStat() {
	}

	void lock() {
		locker.get_ab_write_permission();
	}
	void unlock() {
		locker.release_permission();
	}

public:
	int			last_time;	// 上次重置时间
	int 		count;
	RwLocker 	locker; 	// 采用加锁的方式同步数据，暂时未考虑队列+更新线程方式的同步方案，过于复杂，如果后续压力大的时候，可以将该方案实现以减少多个线程抢占一个锁，同时注意：更新线程的扫描时间应该为500毫秒左右。
};
typedef map<string, RunStat> RunStatMap;

class QpsController {
public:
	~QpsController() {
	}

	static QpsController& inst() {
		static QpsController controller;
		return controller;
	}

	bool init(const string &config_file);

	bool under_qps(const string &qps_point) {
		QpsConfig& config = db_configs.get();
		if (0 == config.on_effect) {
			fprintf(stdout, "%s no effect globally %d\n", __FUNCTION__, config.on_effect);
			return true;
		}
		QpsElem* elem_info = config.get(qps_point);
		if (NULL == elem_info || 0 == elem_info->on_effect) {
			fprintf(stdout, "%s no effect\n", __FUNCTION__);
			return true;
		}

		RunStatMap::iterator iter = run_stats.find(qps_point);
		if (iter == run_stats.end()) {
			fprintf(stdout, "%s can not find %s\n", __FUNCTION__, qps_point.c_str());
			return true;
		}
		if (iter->second.count > elem_info->max_count) {
			time_t now = time(NULL);
			if ((now - iter->second.last_time) >= elem_info->intv_seconds) {
				iter->second.lock();
				iter->second.count = 0;
				iter->second.last_time = now;
				iter->second.unlock();
				return true;
			}
		
			fprintf(stdout, "%s %s is out of qps %d %d\n", __FUNCTION__, qps_point.c_str(), iter->second.count, elem_info->max_count);
			return false;
		}

		return true;
	}

	bool update(const string &qps_point) {
		QpsConfig& config = db_configs.get();
		if (0 == config.on_effect) {
			return true;
		}

		QpsElem* elem_info = config.get(qps_point);
		if (NULL == elem_info || 0 == elem_info->on_effect) {
			return true;
		}

		RunStatMap::iterator iter = run_stats.find(qps_point);
		if (iter == run_stats.end()) {
			if (run_stats_locker.get_ab_write_permission()) {
				RunStat stat;
				stat.count++;
				run_stats.insert(pair<string,RunStat>(qps_point, stat));
				run_stats_locker.release_permission();
			}
		} else {
			iter->second.lock();
			time_t now = time(NULL);
			if ((now - iter->second.last_time) >= elem_info->intv_seconds) {
				iter->second.count = 0;
				iter->second.last_time = now;
			}
			iter->second.count++;
			iter->second.unlock();
		}

		return true;
	}

private:
	QpsController() {
	}

private:
	DbQpsConfigs db_configs;
	RunStatMap   run_stats;
	RwLocker	 run_stats_locker;
};

#endif // __QPS_CONTROLLER_H__
