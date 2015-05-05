#include "ini_file.h"
#include "configor.h"

#include "qps_controller.h"

static bool reloader(const string &config_file, void *context) {
	IniFile ini;
	DbQpsConfigs *db_configs = (DbQpsConfigs*)context;
	QpsConfig& next = db_configs->get_next();

	if (true != ini.init(config_file)) {
		return false;
	}
	next.clear();

	next.on_effect = ini.get("sys", "on_effect", 0);
	if (next.on_effect != 1) {
		next.on_effect = 0;
	}
	fprintf(stdout, "sys on_effect %d\n", next.on_effect);

	if (next.on_effect == 0) {
		return true;
	}

	string data = ini.get("sys", "items", "");
	if (data.empty()) {
		return true;
	}
	fprintf(stdout, "sys items %s\n", data.c_str());

	set<string> item_set;
	if (true != Utils::parse_line(data, ",", item_set)) {
		return false;
	}

	set<string>::iterator iter = item_set.begin();
	for (; iter != item_set.end(); ++iter) {
		QpsElem elem;
		string item = *iter;
		elem.elem_name = item;
		elem.intv_seconds = ini.get(item.c_str(), "interval_seconds", 1);
		if (elem.intv_seconds <= 0) {
			return false;
		}
		elem.max_count = ini.get(item.c_str(), "max_count", 10);
		if (elem.max_count <= 0) {
			return false;
		}
		elem.on_effect = ini.get(item.c_str(), "on_effect", 0);
		if (elem.on_effect != 1) {
			elem.on_effect = 0;
		}
		next.elems[item] = elem;
		fprintf(stdout, "%s %d %d %d\n", item.c_str(), elem.on_effect, elem.max_count, elem.intv_seconds);
	}

	db_configs->alter();

	return true;
}

bool QpsController::init(const string &config_file) {
	if (true != Configor::get_inst().regist(config_file, (void*)&db_configs, reloader, true)) {
		return false;
	}

	return true;
}
