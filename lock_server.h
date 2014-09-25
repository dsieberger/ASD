// this is the lock server
// the lock client has a similar interface

#ifndef lock_server_h
#define lock_server_h

#include <string>
#include "lock_protocol.h"
#include "lock_client.h"
#include "rpc.h"

//Class lock_obj, represents a lock
class lock_obj {

public:
	int client;  	//Current client holding the lock. If -1, lock is free, else is locked
	int nacquire; 	//Number of times the lock was given to a client
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;	//Condition variable of the lock, so threads can wait and wake up

public:
	//Constructor
	lock_obj()
	{
		client = -1;	//No clients, lock free
		nacquire = 0;	//No clients obtained the lock yet
	}

};

class lock_server {

 protected:
  int nacquire;					//Just a value of return
  pthread_mutex_t mutexsum;		//Mutex to control the threads
  std::map<lock_protocol::lockid_t,lock_obj> map;	//Map where each lockID has a lock_obj

 public:
  lock_server();
  ~lock_server() {};
  //RPC methods
  lock_protocol::status stat(int clt, lock_protocol::lockid_t lid, int &);
  lock_protocol::status acquire(int clt, lock_protocol::lockid_t lid, int &);
  lock_protocol::status release(int clt, lock_protocol::lockid_t lid, int &);
};

#endif 







