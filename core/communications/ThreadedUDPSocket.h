#ifndef _COREUDPSOCKET_H_
#define _COREUDPSOCKET_H_


// Network stuff
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <strings.h>
#include <pthread.h>


/// @ingroup communications
class ThreadedUDPSocket {
public:
  ThreadedUDPSocket();
  ~ThreadedUDPSocket();

  char* name;
  int port;
  pthread_t thread;
  int sock, length;
  struct sockaddr_in server;
  struct sockaddr_in from;
  socklen_t fromlen;
  char* buffer;
  int bufferSize;
  void start(char* name_, int port_, int type, int buffersize_, void* (*method)( void *), void * core);
  int recvFrom();
  bool isConnected;
  void closeSocket();
};


#endif
