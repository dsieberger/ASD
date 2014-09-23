// the lock server implementation

#include "lock_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

lock_obj::lock_obj()
{
  times_aq = 0;
  current_clt = -1;
  pthread_cond_init (&count_threshold, NULL);
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

int
lock_obj::getCurrentClt()
{
  return current_clt;
}

void
lock_obj::setCurrentClt(int clt)
{
  current_clt = clt;
}

pthread_cond_t
lock_obj::getCountThreshold()
{
  return count_threshold;
}


lock_server::lock_server()
{
    pthread_mutex_init(&mutexsum, NULL);
}

lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  r = 0;

  pthread_mutex_lock (&mutexsum);

  if(map.find(lid) != map.end()) {

    r = map[lid].getTimesAq();

  } else {
    ret = lock_protocol::NOENT;
  }

  pthread_mutex_unlock (&mutexsum);

  printf("stat request from clt %d :: status %d :: lockID %llu :: retVal %d\n", clt, ret, lid, r);
  return ret;
}



lock_protocol::status
lock_server::acquire(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  r = 0;

  pthread_mutex_lock (&mutexsum);

  if(map.find(lid) != map.end()) {

    pthread_cond_t ct = map[lid].getCountThreshold();
    while((map[lid].getCurrentClt() != -1) /*&& (obj.getCurrentClt() != clt)*/) {
      pthread_cond_wait(&ct, &mutexsum);
      printf("IT'S ALIVE!!!");
      //ret = lock_protocol::RETRY;
    }

    map[lid].setCurrentClt(clt);
    map[lid].incTimesAq();

  } else {

    lock_obj *obj = new lock_obj();
    obj->setCurrentClt(clt);
    obj->incTimesAq();
    map[lid] = *obj;  

  }

  r = map[lid].getCurrentClt();

  pthread_mutex_unlock (&mutexsum);

  printf("acquire request from clt %d :: status %d :: lockID %llu :: retVal %d\n", clt, ret, lid, r);
  return ret;
}

lock_protocol::status
lock_server::release(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  r = 0;

  pthread_mutex_lock (&mutexsum);

  if(map.find(lid) != map.end()) {

    map[lid].setCurrentClt(-1);

    pthread_cond_t ct = map[lid].getCountThreshold();

    pthread_cond_signal(&ct);

  } else {
    ret = lock_protocol::NOENT;
  }

  r = map[lid].getCurrentClt();

  pthread_mutex_unlock (&mutexsum);

  printf("release request from clt %d :: status %d :: lockID %llu :: retVal %d\n", clt, ret, lid, r);
  return ret;
}
