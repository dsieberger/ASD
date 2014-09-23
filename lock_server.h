// this is the lock server
// the lock client has a similar interface

#ifndef lock_server_h
#define lock_server_h

#include <string>
#include "lock_protocol.h"
#include "lock_client.h"
#include "rpc.h"

class lock_obj {

protected:
	int times_aq;
	int current_clt;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

public:
	lock_obj();
	~lock_obj() {};
	int getTimesAq();
	void incTimesAq();
	int getCurrentClt();
	void setCurrentClt(int clt);
	pthread_cond_t getCond();

};

class lock_server {

 protected:
  pthread_mutex_t mutexsum;
  std::map<lock_protocol::lockid_t,lock_obj> map;

 public:
  lock_server();
  ~lock_server() {};
  lock_protocol::status stat(int clt, lock_protocol::lockid_t lid, int &);
  lock_protocol::status acquire(int clt, lock_protocol::lockid_t lid, int &);
  lock_protocol::status release(int clt, lock_protocol::lockid_t lid, int &);
};

#endif 







