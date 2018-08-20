#include<stdlib.h>
#include "ThreadedUDPSocket.h"
//#include <Core.h>
#include <iostream>
#include <unistd.h>

using namespace std;
ThreadedUDPSocket::ThreadedUDPSocket() {
  isConnected=false;
}

ThreadedUDPSocket::~ThreadedUDPSocket() {
  delete buffer;
  closeSocket();
}

void ThreadedUDPSocket::start(char* name_, int port_, int type, int buffersize_, void* (*method)( void* ), void * core) {
  name=name_;  
  port=port_;
  bufferSize=buffersize_;
  buffer=new char[bufferSize];

  sock=socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    cout << "Core: Error Opening " << name << " UDP socket" << endl << flush ;
    return;
  }
  length = sizeof(server);
  bzero(&server,length);

  if (type==SO_BROADCAST) {
    int broadcast=1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&broadcast, sizeof(broadcast));
    type=INADDR_BROADCAST;
  }

  server.sin_family=AF_INET;
  server.sin_addr.s_addr=type; //type;
  //int portno = 7890;
  server.sin_port=htons(port);
  if (type!=-1000) {
    if (bind(sock,(struct sockaddr *)&server,length)<0) {
      cout << "Core: Error Binding " << name << " UDP Socket on port " << port << endl << flush;
      return;
    }
  }
  
  cout << "Core: " << name << " UDP Socket binded on port " << port << endl << flush;
  fromlen = sizeof(struct sockaddr_in);
  isConnected=true;

  pthread_create( &thread, NULL, method, core);
}

int ThreadedUDPSocket::recvFrom() {
  return recvfrom(sock,buffer,bufferSize,0,(struct sockaddr *)&(from),&(fromlen));
}

void ThreadedUDPSocket::closeSocket() {
  cout << "Core: Closing socket " << name << endl << flush;
  close(sock);
};

