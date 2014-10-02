// the extent server implementation

#include "extent_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extent_server::extent_server() {

	file_obj *obj = new file_obj();

	time_t now;
	time(&now);

	obj->file = "";
	obj->attr.atime = now;
	obj->attr.ctime = now;
	obj->attr.mtime = now;
	obj->attr.size = 0;
	obj->valid = true;

	map[0x00000001] = *obj;
}

int extent_server::put(extent_protocol::extentid_t id, std::string buf, int &)
{

	int retval;

	if(map.find(id) != map.end()) {

		if(map[id].valid) {

			time_t now;
			time(&now);

			map[id].file = buf;
			map[id].attr.mtime = now;

		} else {

			file_obj *obj = new file_obj();

			time_t now;
			time(&now);

			obj->file = buf;
			obj->attr.atime = now;
			obj->attr.ctime = now;
			obj->attr.mtime = now;
			obj->attr.size = buf.length();
			obj->valid = true;

			map[id] = *obj;

		}

	} else {

		file_obj *obj = new file_obj();

		time_t now;
		time(&now);

		obj->file = buf;
		obj->attr.atime = now;
		obj->attr.ctime = now;
		obj->attr.mtime = now;
		obj->attr.size = buf.length();
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
			map[id].attr.atime = now;

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

			a.size = map[id].attr.size;
	  		a.atime = map[id].attr.atime;
	  		a.mtime = map[id].attr.mtime;
	  		a.ctime = map[id].attr.ctime;

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

