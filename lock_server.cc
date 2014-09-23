// the lock server implementation

#include "lock_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

lock_obj::lock_obj()
{
  is_locked = false;
  times_aq = 0;
  current_clt = -1;
}

int
lock_obj::getTimesAq()
{
  return times_aq;
}

void
lock_obj::incTimesAq()
{
  times_aq++;
}

bool
lock_obj::status()
{
  return is_locked;
}

void
lock_obj::setLock(bool lock)
{
  is_locked = lock;
}

int
lock_obj::getCurrentClt()
{
  return current_clt;
}

void
lock_obj::setCurrentClt(int clt){
  current_clt = clt;
}



lock_server::lock_server()
{
    pthread_mutex_init(&mutexsum, NULL);
    pthread_cond_init (&count_threshold, NULL);
}

lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  r = 0;

  if(map.find(lid) != map.end()) {

    lock_obj obj = map[lid];
    r = obj.getTimesAq();

  } else {
    ret = lock_protocol::NOENT;
  }

  printf("stat request from clt %d :: %d\n", clt, ret);
  return ret;
}



lock_protocol::status
lock_server::acquire(int clt, lock_protocol::lockid_t lid, int &r)
{

  lock_protocol::status ret = lock_protocol::OK;

  if(map.find(lid) != map.end()) {

    lock_obj obj = map[lid];

    if(obj.status() == true) {
      ret = lock_protocol::RETRY;
    } else {
      obj.setLock(true);
      obj.setCurrentClt(clt);
      obj.incTimesAq();
      map[lid] = obj;
    }

  } else {

    lock_obj *obj = new lock_obj();
    obj->setLock(true);
    obj->setCurrentClt(clt);
    obj->incTimesAq();
    map[lid] = *obj;  

  }

  printf("acquire request from clt %d :: %d\n", clt, ret);
  r = 0;
  return ret;
}

lock_protocol::status
lock_server::release(int clt, lock_protocol::lockid_t lid, int &r)
{

  lock_protocol::status ret = lock_protocol::OK;

  if(map.find(lid) != map.end()) {

    lock_obj obj = map[lid];
    obj.setLock(false);
    obj.setCurrentClt(-1);
    map[lid] = obj;

  } else {
    ret = lock_protocol::NOENT;
  }

  printf("release request from clt %d :: %d\n", clt, ret);
  r = 0;
  return ret;
}


