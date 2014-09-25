// the lock server implementation

#include "lock_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

lock_server::lock_server():
  nacquire (0)
{
  //Initialize the mutex
	pthread_mutex_init(&mutexsum, NULL);
}

lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  r = nacquire;

  pthread_mutex_lock (&mutexsum);

  //Search the map for the lockID requested
  if(map.find(lid) != map.end()) {
  	r = map[lid].nacquire;
  } else {
  	ret = lock_protocol::NOENT;
  }

  pthread_mutex_unlock (&mutexsum);

  printf("stat request from clt %d\n", clt);
  return ret;
}

lock_protocol::status
lock_server::acquire(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  r = nacquire;

  pthread_mutex_lock (&mutexsum);

  //Search the map for the lockID requested
  if(map.find(lid) != map.end()){

    //While lock is locked, thread waits
  	while(map[lid].client != -1) {
  		pthread_cond_wait(&map[lid].cond, &mutexsum);
  	}

  	map[lid].client = clt;
  	map[lid].nacquire++;

  } else { //Create a new lock_obj for the new lockID and store it in the map

  	lock_obj *obj = new lock_obj();
  	obj->nacquire = 1;
  	obj->client = clt;
  	map[lid] = *obj;

  }

  pthread_mutex_unlock (&mutexsum);

  printf("acquire request from clt %d\n", clt);
  return ret;
}

lock_protocol::status
lock_server::release(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  r = nacquire;

  pthread_mutex_lock (&mutexsum);

  //Search the map for the lockID requested
  if(map.find(lid) != map.end()){
  	map[lid].client = -1;
  	pthread_cond_signal(&map[lid].cond);   //Wake up threads waiting on this lock
  } else {
  	ret = lock_protocol::NOENT;
  }

  pthread_mutex_unlock (&mutexsum);

  printf("release request from clt %d\n", clt);
  return ret;
}


