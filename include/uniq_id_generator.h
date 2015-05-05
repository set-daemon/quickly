/*
 * file name: uniq_id_generator.h
 * author   : set_daemon@126.com
 * date     : 2015-03-31
 * history  :
 */

#ifndef __UNIQ_ID_GENERATOR_H__
#define __UNIQ_ID_GENERATOR_H__

#include <sys/types.h>
#include <unistd.h>

#include <sys/time.h>
#include <openssl/md5.h>

#include <string>
#include <vector>
#include <map>
using namespace std;

#include "utils.h"
#include "ini_file.h"

typedef string UniqId;
typedef map<string, string> UniqIdKvData;
typedef vector<string> UniqIdKeyList;

class UniqIdGenerator {
public:
	~UniqIdGenerator() {
	}

	static UniqIdGenerator& inst() {
		static UniqIdGenerator gen;
		return gen;
	}

	// 在多线程环境下，需要在主线程中预先加载.
	int init(const string &config_file) {
		IniFile ini;
		if (ini.init(config_file) != true) {
			return -1;
		}

		UniqIdKeyList::iterator key_iter = essential_keys.begin();
		for (; key_iter != essential_keys.end(); ++key_iter) {
			string data = ini.get("uniq-id", *key_iter, "");
			if (data.size() <= 0) {
				return -5;
			}
			essential_kvdata[*key_iter] = data;
		}

		return 0;
	}

	UniqId gen_id() {
		// 目前设置最大长度为1024，暂时未测试出最佳长度。
		const int Buffer_Size = 1024;
		char buf[Buffer_Size];

		int data_num = 0;

		do {
			UniqIdKeyList::iterator key_iter = essential_keys.begin();
			for (; key_iter != essential_keys.end(); ++key_iter) {
				string& data = essential_kvdata[*key_iter];
				if (Buffer_Size - data_num < data.size()) {
					break;
				}
				int ret = snprintf(buf+data_num, Buffer_Size-data_num, "%s", data.c_str());
				if (ret < 0 || ret > Buffer_Size-data_num) {
					break;
				}
				data_num += ret;
			}

			pid_t my_pid = getpid();
			int ret = snprintf(buf+data_num, Buffer_Size-data_num, "%08x", my_pid);
			if (ret < 0 || ret > Buffer_Size-data_num) {
				break;
			}
			data_num += ret;

			pid_t my_parent_pid = getppid();
			ret = snprintf(buf+data_num, Buffer_Size-data_num, "%08x", my_parent_pid);
			if (ret < 0 || ret > Buffer_Size-data_num) {
				break;
			}
			data_num += ret;

			pthread_t tid = pthread_self();
			ret = snprintf(buf+data_num, Buffer_Size-data_num, "%08x", tid);
			if (ret < 0 || ret > Buffer_Size-data_num) {
				break;
			}
			data_num += ret;

			struct timeval tv;
			gettimeofday(&tv, NULL);
			ret = snprintf(buf+data_num, Buffer_Size-data_num, "%d%d", tv.tv_sec, tv.tv_usec);
			if (ret < 0 || ret > Buffer_Size-data_num) {
				break;
			}
			data_num += ret;

			srandom(tv.tv_sec * 1000000 + tv.tv_usec);
			ret = snprintf(buf+data_num, Buffer_Size-data_num, "%ld", random());
			if (ret < 0 || ret > Buffer_Size-data_num) {
				break;
			}
			data_num += ret;


		} while (0);

		char md5_data[16];
		MD5((const unsigned char*)buf, data_num, (unsigned char*)md5_data);
		char md5_str[33];
		const char *hex_map = "0123456789abcdef";
		for (int i = 0; i < 16; i++) {
			int pos = i << 1;
			md5_str[pos] = hex_map[(md5_data[i] >> 4) &0x0F];
			md5_str[pos+1] = hex_map[md5_data[i] & 0x0F];
		}
		md5_str[32] = '\0';

		return string(md5_str);
	}

private:
	UniqIdGenerator() {
		essential_keys.erase(essential_keys.begin(), essential_keys.end());
		essential_keys.push_back("country");
		essential_keys.push_back("city");
		essential_keys.push_back("carrier_hotel");
		essential_keys.push_back("host_name");
		essential_keys.push_back("server_ip");
		essential_keys.push_back("server_port");
	}

private:
	UniqIdKeyList	essential_keys;
	UniqIdKvData	essential_kvdata;
};

#endif // __UNIQ_ID_GENERATOR_H__
