//
// Lock demo
//

#include "lock_protocol.h"
#include "lock_client.h"
#include "rpc.h"
#include <arpa/inet.h>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

std::string dst;
lock_client *lc;

int
main(int argc, char *argv[])
{
  int r;

  if(argc != 2){
    fprintf(stderr, "Usage: %s [host:]port\n", argv[0]);
    exit(1);
  }

  dst = argv[1];
  lc = new lock_client(dst);

  //Just some tests to check well functioning of the map

  r = lc->acquire(1);
  printf ("acquire 1 returned %d\n", r);
  r = lc->release(1);
  printf ("release 1 returned %d\n", r);
  r = lc->acquire(1);
  printf ("acquire 1 returned %d\n", r);
  r = lc->release(1);
  printf ("release 1 returned %d\n", r);
  r = lc->stat(1);
  printf ("stat 1 returned %d\n", r);

  r = lc->acquire(2);
  printf ("acquire 2 returned %d\n", r);
  r = lc->release(2);
  printf ("release 2 returned %d\n", r);
  r = lc->stat(2);
  printf ("stat 2 returned %d\n", r);

}
