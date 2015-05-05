#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

#include <unistd.h>
#include <signal.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sstream>
using namespace std;

#include "common_defs.h"
#include "debug.h"
#include "utils.h"

namespace Utils {

int signal_process() {
	// TODO:

    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    umask(0);
	return E_OK;
}

int set_daemon(bool set_flag) {
	// TODO:
	if (!set_flag) {
        return 0;
    }

    pid_t pid;

    switch (pid=fork()) {
        case 0:
            break;
        case -1:
            exit(-1);
        default:
            exit(0); // parent process.
    }
    if (setsid() == -1) {
		return -1;
	}

	umask(0);

	int fd = open("/dev/null", O_RDWR);
	if (fd == -1) {
		return -1;
	}
	
	if (dup2(fd, STDIN_FILENO) == -1) {
		return -1;
	}
	if (dup2(fd, STDOUT_FILENO) == -1) {
		return -1;
	}

#if 0
	if (dup2(fd, STDERR_FILENO) == -1) {
		return -1;
	}
#endif

#if 0
    int i = 0;
    for (i = 0; i < NOFILE; ++i) {
        close(i);
    }
#endif

	if (fd > STDERR_FILENO) {
		close(fd);
	}

	signal_process();

	return 0;
}

int check_path_is_valid(const char* path) {
	// TODO:

	return 0;
}

int change_working_dir(const char *working_path) {
	// TODO:
	if (0 != chdir(working_path)) {
		return -1;
	}

	return 0;
}

int append_pid_to_file(pid_t pid, const char *file_name) {
	int fd = open(file_name, O_RDWR|O_CREAT|O_APPEND, S_IRWXU);
	if (fd == -1) {
		fprintf(stderr, "can not open {%s} <%d,%s>\n", file_name, errno, strerror(errno));
		return -1;
	}
	char buf[64];
	int len = snprintf(buf, sizeof(buf), "%d\n", getpid());
	if (len < 0 || len >= sizeof(buf)) {
		return -1;
	}
	write(fd, buf, len);
	close(fd);

	return 0;
}

int remove_pid_file(const char *file_name) {
	unlink(file_name);

	return 0;
}

bool parse_line(const string &line, const char* spliter, vector<string> &storage) {
	if (line.size() <= 0) {
		return true;
	}

	size_t pos = 0;
	int quit = 0;
	while (!quit && pos != line.size()) {
		size_t next_pos = line.find_first_of(*spliter, pos);
		if (next_pos == string::npos) {
			next_pos = line.size();
			quit = 1;
		}
		size_t space_pos = line.find_first_not_of(" ", pos);
		size_t last_space_pos = line.find_last_not_of(" ", next_pos-1);
		string sec = line.substr(space_pos, last_space_pos-space_pos+1);
		//INFO(",,,, size=%d, sec = {%s}, pos=%d,next_pos=%d\n", line.size(), sec.c_str(), pos, next_pos);
		storage.push_back(sec);
		pos = next_pos + 1;
	}

	return true;
}

bool parse_line(const string &line, const char* spliter, set<string> &storage) {
	vector<string> vec;

	if (true != parse_line(line, spliter, vec)) {
		return false;
	}

	for (int i = 0; i < vec.size(); i++) {
		storage.insert(vec[i]);
	}

	return true;
}

bool parse_line(const string &line, const char* spliter, vector<int> &storage) {
	vector<string> vec;

	if (true != parse_line(line, spliter, vec)) {
		return false;
	}

	for (int i = 0; i < vec.size(); i++) {
		storage.push_back(strtoul(vec[i].c_str(), NULL, 10));
	}

	return true;
}

bool parse_line(const string &line, const char* spliter, set<int> &storage) {
	vector<string> vec;

	if (true != parse_line(line, spliter, vec)) {
		return false;
	}

	for (int i = 0; i < vec.size(); i++) {
		storage.insert(strtoul(vec[i].c_str(), NULL, 10));
	}


	return true;
}

bool parse_line(const string &line, const char* spliter, map<string, string> &kvs) {
	vector<string> segs;

	if (true != parse_line(line, spliter, segs)) {
		return false;
	}

	for (int i = 0; i < segs.size(); i++) {
		vector<string> kv;
		if (true != parse_line(segs[i], "=", kv)) {
			return false;
		}

		if (kv.size() < 2) {
			kvs[kv[0]] = "";
		} else {
			kvs[kv[0]] = kv[1];
		}
	}

	return true;
}

string to_string(int v) {
	char buf[24] = { 0 };
	sprintf(buf, "%d", v);

	return string(buf);
}

string to_string(long v) {
	char buf[24] = { 0 };
	sprintf(buf, "%ld", v);

	return string(buf);
}

string to_string(long long v) {
	char buf[24] = { 0 };
	sprintf(buf, "%lld", v);

	return string(buf);
}

string to_string(float v) {
	char buf[24] = { 0 };
	sprintf(buf, "%f", v);

	return string(buf);
}

string to_string(double v) {
	char buf[24] = { 0 };
	sprintf(buf, "%lf", v);

	return string(buf);
}

string to_string(long double v) {
	char buf[24] = { 0 };
	sprintf(buf, "%llf", v);

	return string(buf);
}

string to_upper(string &str) {
	for (int i = 0; i < str.size(); i++) {
		str[i] = toupper(str[i]);
	}

	return str;
}

string str_ltrim(const string &str, const string &trim_chars) {
	size_t pos = str.find_first_not_of(trim_chars);
	if (pos == string::npos) {
		return str;
	} else {
		return str.substr(pos);
	}
}

string str_rtrim(const string &str, const string &trim_chars) {
	size_t pos = str.find_last_not_of(trim_chars, str.size()-1);
	if (pos == string::npos) {
		return "";
	} else {
		return str.substr(0, pos+1);
	}
}

string str_trim(const string &str, const string &trim_chars) {
	string new_str = str_ltrim(str, trim_chars);
	new_str = str_rtrim(new_str, trim_chars);

	return new_str;
}

bool intersection_is_empty(const vector<int> &v1, const set<int> &s1) {
	// here can not make sort to v1 and s1, as they are defined as const type.
	
	for (int i = 0; i < v1.size(); i++) {
		set<int>::const_iterator iter = s1.find(v1[i]);
		if (iter != s1.end()) {
			return false;
		}
	}

	return true;
}

bool intersection_is_empty(const vector<int> &v1, const vector<int> &v2) {
	// here can not make sort to v1 and s1, as they are defined as const type.

	for (int i = 0; i < v1.size(); i++) {
		for (int j = 0; j < v2.size(); j++) {
			if (v1[i] == v2[j]) {
				return false;
			}
		}
	}

	return true;
}

bool intersection_is_empty(const set<int> &s1, const set<int> &s2) {
	// here can not make sort to v1 and s1, as they are defined as const type.
	set<int>::const_iterator s1_iter = s1.begin();
	for (; s1_iter != s1.end(); ++s1_iter) {
		set<int>::const_iterator s2_iter = s2.begin();
		for (; s2_iter != s2.end(); ++s2_iter) {
			if (*s1_iter == *s2_iter) {
				return false;
			}
		}
	}

	return true;
}

bool intersection_is_empty(const set<string> &s1, const set<string> &s2) {
	// here can not make sort to v1 and s1, as they are defined as const type.
	set<string>::const_iterator s1_iter = s1.begin();
	for (; s1_iter != s1.end(); ++s1_iter) {
		set<string>::const_iterator s2_iter = s2.begin();
		for (; s2_iter != s2.end(); ++s2_iter) {
			if (*s1_iter == *s2_iter) {
				return false;
			}
		}
	}

	return true;
}

bool intersection_is_empty(const set<string> &s1, const vector<string> &s2) {
	// here can not make sort to v1 and s1, as they are defined as const type.
	set<string>::const_iterator s1_iter = s1.begin();
	for (; s1_iter != s1.end(); ++s1_iter) {
		vector<string>::const_iterator s2_iter = s2.begin();
		for (; s2_iter != s2.end(); ++s2_iter) {
			if (*s1_iter == *s2_iter) {
				return false;
			}
		}
	}

	return true;
}


string get_host_from_url(const string &url) {
	size_t start = 0, end = 0;

	start = url.find_first_of("://", 0);
	if (start == string::npos) {
		start = 0;
	} else {
		start += sizeof("://") - 1;
	}

	end = url.find_first_of("/", start);
	if (end == string::npos) {
		end = url.size();
	}

	return url.substr(start, end-start);
}

bool host_matching(set<string> &hosts, string &host) {
	set<string>::iterator iter = hosts.find(host);
	if (iter != hosts.end()) {
		return true;
	}
	vector<string> segs;
	if (true != Utils::parse_line(host, ".", segs)) {
		return false;
	}
	for (int i = segs.size() - 2; i >= 0; --i) {
		ostringstream os;
		os << "*.";

		for (int j = i; j < segs.size(); ++j) {
			os << segs[j];
			if (j < segs.size()-1) {
				os << ".";
			}
		}

		string pattern_host = os.str();
		if (hosts.end() != hosts.find(pattern_host)) {
			return true;
		} else {
		}
	}

	return false;
}

}
