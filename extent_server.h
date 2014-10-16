// this is the extent server

#ifndef extent_server_h
#define extent_server_h

#include <string>
#include <map>
#include "extent_protocol.h"

class file_obj {

public:
	std::string file;
	extent_protocol::attr attr;
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
  int setSize(extent_protocol::extentid_t id, int newSize, int &);

};

#endif 







