/*
 * file name: utils.h
 * purpose  : 
 * author   : set_daemon@126.com
 * date     : 2014-08-20
 * history  :
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>
#include <vector>
#include <set>
#include <map>
using namespace std;

namespace Utils {

int signal_process();

int set_daemon(bool flag);

int check_path_is_valid(const char *path);

int change_working_dir(const char *working_path);

int append_pid_to_file(pid_t pid, const char *file_name);
int remove_pid_file(const char *file_name);

bool parse_line(const string &line, const char* spliter, vector<string> &storage);
bool parse_line(const string &line, const char* spliter, set<string> &storage);
bool parse_line(const string &line, const char* spliter, vector<int> &storage);
bool parse_line(const string &line, const char* spliter, set<int> &storage);
// it works for such line: k=1;m=3
bool parse_line(const string &line, const char* spliter, map<string, string> &kvs);

bool intersection_is_empty(const vector<int> &v1, const set<int> &s1);
bool intersection_is_empty(const vector<int> &v1, const vector<int> &v2);
bool intersection_is_empty(const set<int> &s1, const set<int> &s2);
bool intersection_is_empty(const set<string> &s1, const set<string> &s2);
bool intersection_is_empty(const set<string> &s1, const vector<string> &s2);

string str_ltrim(const string &str, const string &trim_chars);
string str_rtrim(const string &str, const string &trim_chars);
string str_trim(const string &str, const string &trim_chars);

string to_string(int v);
string to_string(long v);
string to_string(long long v);
string to_string(float v);
string to_string(double v);
string to_string(long double v);
string to_upper(string &str);

string get_host_from_url(const string &url);
bool host_matching(set<string> &hosts, string &host);
}

#endif // __UTILS_H__
