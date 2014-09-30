// the extent server implementation

#include "extent_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extent_server::extent_server() {}

int extent_server::put(extent_protocol::extentid_t id, std::string buf, int &)
{

	int retval;

	if(map.find(id) != map.end()) {

		if(map[id].valid) {

			time_t now;
			time(&now);

			map[id].file = buf;
			map[id].file_mtime = now;

		} else {

			file_obj *obj = new file_obj();

			time_t now;
			time(&now);

			obj->file = buf;
			obj->file_atime = now;
			obj->file_ctime = now;
			obj->file_mtime = now;
			obj->file_size = buf.length();
			obj->valid = true;

			map[id] = *obj;

		}

	} else {

		file_obj *obj = new file_obj();

		time_t now;
		time(&now);

		obj->file = buf;
		obj->file_atime = now;
		obj->file_ctime = now;
		obj->file_mtime = now;
		obj->file_size = buf.length();
		obj->valid = true;

		map[id] = *obj;

	}

	retval = extent_protocol::OK;

	return retval;

}

int extent_server::get(extent_protocol::extentid_t id, std::string &buf)
{

	int retval;

	if(map.find(id) != map.end()) {

		if(map[id].valid) {

			time_t now;
			time(&now);

			buf = map[id].file;
			map[id].file_atime = now;

			retval = extent_protocol::OK;

		} else {
			retval = extent_protocol::NOENT;
		}

	} else {
		retval = extent_protocol::NOENT;
	}

  	return retval;

}

int extent_server::getattr(extent_protocol::extentid_t id, extent_protocol::attr &a)
{

	int retval;

	a.size = 0;
	a.atime = 0;
	a.mtime = 0;
	a.ctime = 0;

	if(map.find(id) != map.end()) {

		if(map[id].valid) {

			a.size = map[id].file_size;
	  		a.atime = map[id].file_atime;
	  		a.mtime = map[id].file_mtime;
	  		a.ctime = map[id].file_ctime;

		}

	}

	retval = extent_protocol::OK;

  	return retval;

}

int extent_server::remove(extent_protocol::extentid_t id, int &)
{
  
	int retval;

	if(map.find(id) != map.end()) {

		if(map[id].valid) {

			map[id].valid = false;

			retval = extent_protocol::OK;

		} else {
			retval = extent_protocol::NOENT;
		}

	} else {
		retval = extent_protocol::NOENT;
	}

  	return retval;

}

