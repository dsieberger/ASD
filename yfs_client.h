#ifndef yfs_client_h
#define yfs_client_h

#include <string>
//#include "yfs_protocol.h"
#include "extent_client.h"
#include <vector>

#include "lock_protocol.h"
#include "lock_client.h"

class yfs_client {
  extent_client *ec;
  lock_client *lc;
 public:

  typedef unsigned long long inum;
  enum xxstatus { OK, RPCERR, NOENT, IOERR, FBIG };
  typedef int status;

  struct fileinfo {
    unsigned long long size;
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
  };
  struct dirinfo {
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
  };
  struct dirent {
    std::string name;
    unsigned long long inum;
  };

 private:
  static std::string filename(inum);
  static inum n2i(std::string);
 public:

  yfs_client(std::string, std::string);

  bool isfile(inum);
  bool isdir(inum);
  inum ilockup(inum, std::string);
  inum ilookup(inum, std::string);

  int getfile(inum, fileinfo &);
  int getdir(inum, dirinfo &);

  //<david>
  int file_content(inum, std::string &);
  int modify_file(inum, std::string);
  //</david>
  
  int newfile(inum, inum, std::string);
  int newdir(inum, inum, std::string);
  int locked_remove(inum, std::string);
  int remove(inum, std::string);
  std::map<inum, std::string> listdir(inum inum);

  int setSize(inum, int);

};

#endif 
