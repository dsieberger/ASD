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

	obj->file = "name\nroot";
	obj->attr.atime = now;
	obj->attr.ctime = now;
	obj->attr.mtime = now;
	obj->attr.size = now;
	obj->valid = true;

	map[0x00000001] = *obj;
	
}

int extent_server::put(extent_protocol::extentid_t id, std::string buf, int &)
{

	printf("put request for file %lld\n", id);

	int retval;

	if(map.find(id) != map.end()) {

		printf("-- found\n");

		if(map[id].valid) {

			printf("-- actualized:\n %s\n", buf.c_str());

			time_t now;
			time(&now);

			map[id].file = buf;
			map[id].attr.mtime = now;

			//<david>
			//também se muda o tamanho
			//map[id].attr.size = buf.size();
			//</david>

		} else {

			printf("-- not valid\n");
			printf("-- created new:\n %s\n", buf.c_str());

			file_obj *obj = new file_obj();

			time_t now;
			time(&now);

			obj->file = buf;
			obj->attr.atime = now;
			obj->attr.ctime = now;
			obj->attr.mtime = now;
			obj->attr.size = now;
			obj->valid = true;

			map[id] = *obj;

		}

	} else {

		printf("-- not found\n");

		printf("-- created new:\n %s\n", buf.c_str());

		file_obj *obj = new file_obj();

		time_t now;
		time(&now);

		obj->file = buf;
		obj->attr.atime = now;
		obj->attr.ctime = now;
		obj->attr.mtime = now;
		obj->attr.size = 0;
		obj->valid = true;

		map[id] = *obj;

	}

	retval = extent_protocol::OK;

	return retval;

}

int extent_server::get(extent_protocol::extentid_t id, std::string &buf)
{

	printf("get request for file %lld\n", id);

	int retval;

	if(map.find(id) != map.end()) {

		printf("-- found\n");

		if(map[id].valid) {

			printf("-- returned:\n %s\n", map[id].file.c_str());

			time_t now;
			time(&now);

			buf = map[id].file.c_str();
			map[id].attr.atime = now;

			retval = extent_protocol::OK;

		} else {
			printf("-- not valid\n");

			retval = extent_protocol::NOENT;
		}

	} else {

		printf("-- not found\n");

		retval = extent_protocol::NOENT;
	}

  	return retval;

}

int extent_server::getattr(extent_protocol::extentid_t id, extent_protocol::attr &a)
{

	printf("getattr request for file %lld\n", id);

	int retval;

	a.size = 0;
	a.atime = 0;
	a.mtime = 0;
	a.ctime = 0;

	if(map.find(id) != map.end()) {

		printf("-- found\n");

		if(map[id].valid) {

			printf("-- valid\n");

			a.size = map[id].attr.size;
	  		a.atime = map[id].attr.atime;
	  		a.mtime = map[id].attr.mtime;
	  		a.ctime = map[id].attr.ctime;

	  		retval = extent_protocol::OK;

		} else {
			printf("-- not valid\n");
			retval = extent_protocol::NOENT;
		}

	} else {
		printf("-- not found\n");
		retval = extent_protocol::NOENT;
	}

  	return retval;

}

int extent_server::remove(extent_protocol::extentid_t id, int &)
{

	printf("remove request for file %lld\n", id);

	int retval;

	if(map.find(id) != map.end()) {

		printf("-- found\n");

		if(map[id].valid) {

			printf("-- valid\n");

			map[id].valid = false;

			retval = extent_protocol::OK;

		} else {
			printf("-- not valid\n");
			retval = extent_protocol::NOENT;
		}

	} else {
		printf("-- not found\n");
		retval = extent_protocol::NOENT;
	}

  	return retval;

}

int extent_server::setSize(extent_protocol::extentid_t id, int newSize, int &)
{

	printf("changeSize request for file %lld\n", id);

	int retval;

	if(map.find(id) != map.end()) {

		printf("-- found\n");

		if(map[id].valid) {

			printf("-- valid\n");

			map[id].attr.size = newSize;

			retval = extent_protocol::OK;

		} else {
			printf("-- not valid\n");
			retval = extent_protocol::NOENT;
		}

	} else {
		printf("-- not found\n");
		retval = extent_protocol::NOENT;
	}

  	return retval;

}

