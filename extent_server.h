// this is the extent server

#ifndef extent_server_h
#define extent_server_h

#include <string>
#include <map>
#include "extent_protocol.h"

class file_obj {

public:
	std::string file;
	int file_size;
    time_t file_atime;
    time_t file_mtime;
    time_t file_ctime;
    bool valid;

public:
	file_obj(){
		valid = true;
	}

};

class extent_server {

protected:
	std::map<extent_protocol::extentid_t, file_obj> map;

 public:
  extent_server();

  int put(extent_protocol::extentid_t id, std::string, int &);
  int get(extent_protocol::extentid_t id, std::string &);
  int getattr(extent_protocol::extentid_t id, extent_protocol::attr &);
  int remove(extent_protocol::extentid_t id, int &);
};

#endif 







