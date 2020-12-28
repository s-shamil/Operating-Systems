#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sock.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"

//
// TODO: Create a structure to maintain a list of sockets
// Should it have locking?
//
struct {
  struct spinlock lock;
  struct sock sock[NSOCK];
} stable;

//test func

void printSocket(struct sock * s){
  if(s->state == CLOSED){
    cprintf("CLOSED ");
  }
  if(s->state == CONNECTED){
    cprintf("CONNECTED ");
  }
  if(s->state == LISTENING){
    cprintf("LISTENING ");
  }
  cprintf("%d %d : %s\n", s->localPort, s->remotePort, s->ownerPID);
}

void printAllSocket(){
  for(int i = 0; i<NSOCK; i++) printSocket(&stable.sock[i]);
}


//utility
int isUsed(int port){
  struct sock *s;
  for(int i = 0; i<NSOCK; i++){
    s = &stable.sock[i];
    if( s->localPort == port && s->state != CLOSED ){
      return 1;
    }
  }
  return 0;
}

int getUnusedPort(void){
  for(int p = 1; p<=NPORT; p++){
    for(int i = 0; i<NSOCK; i++){
      //check if p is used
      if(!isUsed(p)){
        return p;
      }
    }
  }
  //all port engaged - so no socket can be opened
  return E_FAIL;
}

void
sinit(void)
{
  //
  // TODO: Write any initialization code for socket API
  // initialization.
  //
  initlock(&stable.lock, "stable");
}

int
listen(int lport) {

  //
  // TODO: Put the actual implementation of listen here.
  //

  //lelmil hello
  /*
  plan is to take the port number search stable if already not Used
  if unused entry new socket
  */
  if(isUsed(lport)){
    return E_WRONG_STATE; //port used
  }
  //lock table - remember to unlock before return
  acquire(&stable.lock);
  struct sock *s;
  for(int i = 0; i<NSOCK; i++){
    s = &stable.sock[i];
    if(s->state == CLOSED){
      //slot for new socket found
      s->localPort = lport;
      s->remotePort = 0; // not fixed yet
      s->state = LISTENING;
      s->ownerPID = myproc()->pid;
      s->dataPresent = 0;
      break;
    }
  }
  release(&stable.lock);
  return 0;
}

int
connect(int rport, const char* host) {
  //
  // TODO: Put the actual implementation of connect here.
  //
  //lelmil hello
  /*
  plan is to check the stable for rport and state is LISTENING
  */

  // rport should be as localport in one of the stable entries and in LISTENING mode
  struct sock *s;
  for(int i = 0; i < NSOCK; i++){
    //lock yo table
    //remember to unlock before return
    acquire(&stable.lock);

    s = &stable.sock[i];
    if(s->localPort == rport && s->state != CLOSED){
      //valid request
      if(s->state != LISTENING){
        //not ready yet
        release(&stable.lock);
        return E_WRONG_STATE;
      }
      //create client side socket
      struct sock *clientSocket;
      for(int j = 0; j<NSOCK; j++){
        clientSocket = &stable.sock[j];
        if(clientSocket->state == CLOSED){
          //slot for new socket found
          //change server side socket
          int clientPort = getUnusedPort();
          s->remotePort = clientPort;
          s->state = CONNECTED;

          //setup client socket
          clientSocket->localPort = clientPort;
          clientSocket->remotePort = s->localPort; // server's local port
          clientSocket->state = CONNECTED;
          clientSocket->ownerPID = myproc()->pid;
          clientSocket->dataPresent = 0;

          //both side done - connection was successful
          release(&stable.lock);
          return clientPort;
        }
      }
      release(&stable.lock);
      return E_FAIL;
    }
  }
  release(&stable.lock);
  return E_NOTFOUND;
}

int
send(int lport, const char* data, int n) {
  //
  // TODO: Put the actual implementation of send here.
  //

  //owner check - stable process against this lport should match myproc()->pid
  //lock stable
  acquire(&stable.lock);
  struct sock * s;
  for(int i = 0; i<NSOCK; i++){
    s = &stable.sock[i];
    if(s->localPort == lport && s->state != CLOSED){
      if(s->state == LISTENING){
        //not ready yet
        release(&stable.lock);
        return E_WRONG_STATE;
      }
      //found the requested port - check owner
      if(s->ownerPID != myproc()->pid){
        //access denied
        //unlock stable
        release(&stable.lock);
        return E_ACCESS_DENIED;
      }
      //access granted
      int rport = s->remotePort;
      //find this rport as localPort in stable
      struct sock * r;
      for(int j = 0; j<NSOCK; j++){
        r = &stable.sock[j];
        if(r->localPort == rport && r->state != CLOSED){
          if(r->state == LISTENING){
            //not ready yet
            release(&stable.lock);
            return E_WRONG_STATE;
          }
          //found the receiver socket
          //try to write in this buffer
          if(r->dataPresent == 1){ //cant write - go to sleep
            //sleeping on remote socket
            sleep(r, &stable.lock);
          }
          //wake up called from recv
          //can write now
          strncpy(r->buffer, data, n);
          r->dataPresent = 1;
          //if anyone sleeping on this socket wake up call needed
          wakeup(s);
          //unlock stable
          release(&stable.lock);
          return 0; //success
        }
      }
    }
  }
  //we are here means lport/rport one/both of them not found
  //unlock stable
  release(&stable.lock);
  return E_NOTFOUND;
}


int
recv(int lport, char* data, int n) {
  //
  // TODO: Put the actual implementation of recv here.
  //

  //owner check - stable process against this lport should match myproc()->pid
  //lock table
  acquire(&stable.lock);
  struct sock *s;
  for(int i = 0; i<NSOCK; i++){
    s = &stable.sock[i];
    if(s->localPort == lport && s->state != CLOSED){
      if(s->state == LISTENING){
        //not ready yet
        release(&stable.lock);
        return E_WRONG_STATE;
      }
      //found the requested port - check owner
      if(s->ownerPID != myproc()->pid){
        //access denied
        //unlock stable
        release(&stable.lock);
        return E_ACCESS_DENIED;
      }
      //access granted
      //try to write from the buffer
      if(s->dataPresent == 0){ // cant read - go to sleep
        //sleep on the remote port
        struct  sock *r;
        int found_remote_skt = 0;
        for(int j = 0; j<NSOCK; j++){
          r = &stable.sock[j];
          if(r->localPort == s->remotePort && r->state != CLOSED){
            //remote socket found
            found_remote_skt = 1;

            if(r->state == LISTENING){
              //not ready yet
              release(&stable.lock);
              return E_WRONG_STATE;
            }
            //sleep on this socket
            sleep(r, &stable.lock);
            //wake up called so break the loop
            break;
          }
        }
        if(found_remote_skt == 0) {
          return E_NOTFOUND;
        }
      }
      //wake up called from send
      //can read now
      strncpy(data, s->buffer, n);
      s->dataPresent = 0;
      //if anyone sleeping on this socket - wake up needed
      wakeup(s);
      //unlock stable
      release(&stable.lock);
      return 0; //success
    }
  }
  //we are here means lport not found in stable
  //unlock stable
  release(&stable.lock);
  return E_NOTFOUND;
}

int
disconnect(int lport) {
  //
  // TODO: Put the actual implementation of disconnect here.
  //
  acquire(&stable.lock);
  struct sock *s;
  for(int i = 0; i<NSOCK; i++){
    s = &stable.sock[i];
    if(s->localPort == lport && s->state != CLOSED){
      if(s->state == LISTENING ){
        release(&stable.lock);
        return E_WRONG_STATE;
      }
      //CONNECTED

      //found the requested port - check owner
      if(s->ownerPID != myproc()->pid){
        //access denied
        //unlock stable
        release(&stable.lock);
        return E_ACCESS_DENIED;
      }

      //int rport = s->remotePort;

      s->localPort = 0;
      s->remotePort = 0;
      s->state = CLOSED;
      s->ownerPID = 0;
      s->dataPresent = 0;
      //following piece of code maybe of no use as both side calls disconnect
      //if there is any need to uncomment this - make sure to check if data present in buffer before closing socket
      /*
      //rport find and close
      struct sock *r;
      for(int j = 0; j<NSOCK; j++){
        r = &stable.sock[j];
        if(r->localPort == rport && r->state != CLOSED){
          if(r->state == LISTENING ){
            release(&stable.lock);
            return E_WRONG_STATE;
          }
          //CONNECTED
          r->localPort = 0;
          r->remotePort = 0;
          r->state = CLOSED;
          r->ownerPID = 0;
          r->dataPresent = 0;

          release(&stable.lock);
          return 0; //success
        }
      }
      */
      release(&stable.lock);
      return 0;
    }
  }
  //not found port
  return E_NOTFOUND;
}
