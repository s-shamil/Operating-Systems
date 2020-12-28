#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"

int isValidPort(int port){
  if(port<1 || port>NPORT){
    return 0;
  }
  return 1;
}

int isValidIp(char * p){
  char *x = "127.0.0.1";
  char *y = "localhost";
  int len = strlen(p);
  if(strncmp(x, p, len)==0 || strncmp(y, p, len)==0){
    //cprintf("IP VALID\n");
    return 1;
  }
  return 0;
}


int
sys_listen(void)
{
  int port = 0;
  //
  // TODO: Write your code to get and validate port no.
  //
  if(argint(0, &port)<0){
    return -1;
  }
  if(!isValidPort(port)){
    return E_INVALID_ARG;
  }
  return listen(port);
}



int
sys_connect(void)
{
  int port = 0;
  char *host = 0;

  //
  // TODO: Write your code to get and validate port no., host.
  // Allow connection to "localhost" or "127.0.0.1" host only
  //
  if( argint(0, &port)<0 || argstr(1, &host)<0 ){
    return -1;
  }
  if(!isValidPort(port) || !isValidIp(host)){
    return E_INVALID_ARG;
  }
  return connect(port, host);
}

int
sys_send(void)
{
  int port = 0;
  char* buf = 0;
  int n = 0;

  //
  // TODO: Write your code to get and validate port no., buffer and buffer size
  //
  if(argint(0, &port)<0 || argstr(1, &buf)<0 || argint(2, &n)<0){
    return -1;
  }
  if(!isValidPort(port) || n<0 || n > BUFFER_SIZE){
    return E_INVALID_ARG;
  }
  return send(port, buf, n);
}

int
sys_recv(void)
{
  int port = 0;
  char* buf = 0;
  int n = 0;

  //
  // TODO: Write your code to get and validate port no, buffer and buffer size
  //
  if(argint(0, &port)<0 || argstr(1, &buf)<0 || argint(2, &n)<0){
    return -1;
  }
  if(!isValidPort(port) || n<0 || n > BUFFER_SIZE){
    return E_INVALID_ARG;
  }
  return recv(port, buf, n);
}

int
sys_disconnect(void)
{
  int port = 0;

  //
  // TODO: Write your code to get and validate port no.
  //

  if(argint(0, &port)<0){
    return -1;
  }
  if(!isValidPort(port)){
    return E_INVALID_ARG;
  }
  return disconnect(port);
}
