// yfs client.  implements FS operations using extent and lock server
#include "yfs_client.h"
#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>


yfs_client::yfs_client(std::string extent_dst, std::string lock_dst)
{
  ec = new extent_client(extent_dst);

}

yfs_client::inum
yfs_client::n2i(std::string n)
{
  std::istringstream ist(n);
  unsigned long long finum;
  ist >> finum;
  return finum;
}

std::string
yfs_client::filename(inum inum)
{
  std::ostringstream ost;
  ost << inum;
  return ost.str();
}

bool
yfs_client::isfile(inum inum)
{
  if(inum & 0x80000000)
    return true;
  return false;
}

bool
yfs_client::isdir(inum inum)
{
  return ! isfile(inum);
}

int
yfs_client::getfile(inum inum, fileinfo &fin)
{
  int r = OK;


  printf("getfile %016llx\n", inum);
  extent_protocol::attr a;
  if (ec->getattr(inum, a) != extent_protocol::OK) {
    r = IOERR; printf("getfile IOERR\n");
    goto release;
  }

  fin.atime = a.atime;
  fin.mtime = a.mtime;
  fin.ctime = a.ctime;
  fin.size = a.size;
  printf("getfile %016llx -> sz %llu\n", inum, fin.size);

 release:

  return r;
}

int
yfs_client::getdir(inum inum, dirinfo &din)
{
  int r = OK;


  printf("getdir %016llx\n", inum);
  extent_protocol::attr a;
  if (ec->getattr(inum, a) != extent_protocol::OK) {
    r = IOERR; printf("getdir IOERR\n");
    goto release;
  }
  din.atime = a.atime;
  din.mtime = a.mtime;
  din.ctime = a.ctime;

 release:

  return r;
}

int
yfs_client::setSize(inum id, int newSize)
{

  int r = OK;

  if(ec->setSize(id, newSize) != extent_protocol::OK) {
    r = IOERR; printf("changeSize ERROR");
  }

  return r;

}

int
yfs_client::newfile(inum parent_id, inum inum, std::string name) 
{
  int r = OK;

  printf("newfile %016llx\n", inum);
  std::string s = "name\n" + name;

  s += "\nparent\n" + filename(parent_id);

  std::string res;
  if(ec->get(parent_id, res) == extent_protocol::OK) {
    if(ec->put(parent_id, res + "\nchild\n" + filename(inum)) != extent_protocol::OK) {
    	r = IOERR; printf("newfile child put IOERR\n");
    	goto release;
    }
  } else {
  	r = IOERR; printf("newfile parent get IOERR\n");
    goto release;
  }

  if(ec->put(inum, s) != extent_protocol::OK) {
     r = IOERR; printf("newfile child put IOERR\n");
     goto release;
  }
  

  printf("newfile %016llx  OK\n", inum);

  release:

  return r;
}

yfs_client::inum
yfs_client::ilookup(inum parent_id, std::string name) {

	printf("ilookup called searching for %s in %lld\n", name.c_str(), parent_id);

	std::list<yfs_client::inum> list;
  std::string res;
  ec->get(parent_id, res);

  char array [res.length()];
  for(int i = 0; i < res.length(); i++) {
    array[i] = res[i];
  }

  char * tokens;
  bool next = false;

  tokens = strtok (array,"\n");
  while (tokens != NULL) {

    std::string tok;
    tok = tokens;

    if(tok.compare("child") == 0) {
      next = true;
    }

    else if(next) {
      next = false;
      list.push_front(n2i(tok));

    }

    tokens = strtok (NULL,"\n");

  }


	for(std::list<yfs_client::inum>::iterator iter = list.begin(); iter != list.end(); iter++) {

		  	std::string comeback;
		      ec->get(*iter, comeback);

		      char a [comeback.length()];
		      for(int b = 0; b < comeback.length(); b++) {
		        a[b] = comeback[b];
		      }

		      char * tokensc;
		      bool nextc = false;

		      tokensc = strtok (a,"\n");
		      while (tokensc != NULL) {     

		        std::string tkn;
		        tkn = tokensc;

		        if(tkn.compare("name") == 0) {
		          nextc = true;
		        }

		        else if(nextc) {
		          nextc = false;
		          if(tkn.compare(name) == 0) {
		          	printf("ilookup FOUND!\n");
		            return *iter;
		          }
		        }

		        tokensc = strtok (NULL,"\n");

		      }

   } 


  printf("ilookup NOT FOUND!\n");

  return -1;

}


std::map<yfs_client::inum, std::string>
yfs_client::listdir(inum num) {

	printf("listdir called to list dir %lld\n", num);

  std::map<yfs_client::inum, std::string> map;
  std::list<yfs_client::inum> list;

  std::string res;
  ec->get(num, res);

  char array [res.length()];
  for(int i = 0; i < res.length(); i++) {
    array[i] = res[i];
  }

  char * tokens;
  bool next = false;

  tokens = strtok (array,"\n");
  while (tokens != NULL) {

    std::string tok;
    tok = tokens;

    if(tok.compare("child") == 0) {
      next = true;
    }

    else if(next) {
      next = false;
      list.push_front(n2i(tok));
    }

    tokens = strtok (NULL,"\n");

  }

  ////////////////////////
  printf("----------------------\n");

  	for(std::list<yfs_client::inum>::iterator iter = list.begin(); iter != list.end(); iter++) {

	  	  std::string comeback;
	      ec->get(*iter, comeback);

	      char a [comeback.length()];
	      for(int b = 0; b < comeback.length(); b++) {
	        a[b] = comeback[b];
	      }

	      char * tokensc;
	      bool nextc = false;

	      tokensc = strtok (a,"\n");
	      while (tokensc != NULL) {     

	        std::string tkn;
	        tkn = tokensc;

	        if(tkn.compare("name") == 0) {
	          nextc = true;
	        }

	        else if(nextc) {
	          nextc = false;
	          map[*iter] = tkn;
	        }

	        tokensc = strtok (NULL,"\n");

	      }

    }

  printf("listdir res:\n");
  for (std::map<yfs_client::inum, std::string>::iterator it = map.begin(); it!=map.end(); it++) {
      std::string name = it->second;
      printf("%lld - %s\n", it->first, name.c_str());
  }
  printf("----------------------\n");

  return map;

}
