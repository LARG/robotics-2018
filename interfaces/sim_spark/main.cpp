#include <errno.h>
#include <signal.h>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <netinet/in.h>

#include <rcssnet/tcpsocket.hpp>
#include <rcssnet/exception.hpp>

#include "Behavior.h"
#include "RobotBehavior.h"

using namespace rcss::net;
using namespace std;

TCPSocket gSocket;
//UDPSocket gSocket;
string gHost = "127.0.0.1";
int gPort = 3100;

// bool to indicate whether to continue the agent mainloop
static bool gLoop = true;

string teamName;
int uNum;
string outputFile; // For optimization
string agentType("naoagent");

// SIGINT handler prototype
extern "C" void handler(int sig)
{
    if (sig == SIGINT)
        gLoop = false;
}

void PrintHelp()
{
    cout << "\nusage: agentspark [options]" << endl;
    cout << "\noptions:" << endl;
    cout << " --help      prints this message." << endl;
    cout << " --host=IP   IP of the server." << endl;
    cout << " --port port port of the server." << endl;
    cout << " --team TeamName Name of Team." << endl;
    cout << " --unum UNum Uniform Number of Player." << endl;
    cout << " --paramsfile name of a parameters file to be loaded" << endl;
    cout << " --pkgoalie goalie for penalty kick shootout" << endl;
    cout << " --pkshooter shooter for penalty kick shootout" << endl;
    cout << " --experimentout filename The file to output the results of an experimental run" << endl;
    cout << " --optimize <agent-type> optimization agent type: optWalkFront, and so on" << endl;
    cout << " --recordstats have agent write out stats about game" << endl;
    cout << "\n";
}


void ReadOptions(int argc, char* argv[]) {

    teamName = "UTAustinVilla";
    uNum = 2;

    for( int i = 0; i < argc; i++) {
      if ( strcmp( argv[i], "--help" ) == 0 ) {
        PrintHelp();
        exit(0);
      }
      else if ( strncmp( argv[i], "--host", 6 ) == 0 ) {
        string tmp=argv[i];
       
        if ( tmp.length() <= 7 ) // minimal sanity check
        {
          PrintHelp();
          exit(0);
        }
        gHost = tmp.substr(7);
      }
      else if ( strncmp( argv[i], "--port", 6) == 0 ) { 
        if (i == argc - 1) {
          PrintHelp();
          exit(0);
        }
        gPort = atoi(argv[i+1]);
      }
      else if(strcmp(argv[i], "--team") == 0){
        if(i == argc - 1){
          PrintHelp();
          exit(0);
        }
        
        teamName = argv[i + 1];
      }
      else if(strcmp(argv[i], "--unum") == 0){
        if(i == argc - 1){
          PrintHelp();
          exit(0);
        }
        uNum = atoi(argv[i + 1]);
        if (uNum < 1 || uNum > 4){
          cout << "Invalid player num (1-4): " << uNum << endl;
          exit(0);
        }
      }
    }
}

bool Init() {
  cout << "connecting to TCP " << gHost << ":" << gPort << "\n";
  //cout << "connecting to UDP " << gHost << ":" << gPort << "\n";
  
  try {
    Addr local(INADDR_ANY,INADDR_ANY);
    gSocket.bind(local);
  }
  
  catch (BindErr error) {
    cerr << "failed to bind socket with '"
         << error.what() << "'" << endl;
    
    gSocket.close();
    return false;
  }
  
  try {
    Addr server(gPort,gHost);
    gSocket.connect(server);
  } catch (ConnectErr error) {
    cerr << "connection failed with: '"
         << error.what() << "'" << endl;
    gSocket.close();
    return false;
  }
  
  return true;
}

void Done() {
  gSocket.close();
  cout << "closed connection to " << gHost << ":" << gPort << "\n";
}

bool SelectInput() {
  fd_set readfds;
  struct timeval tv = {60,0};
  FD_ZERO(&readfds);
  FD_SET(gSocket.getFD(),&readfds);
  
  while(1) {
    switch(select(gSocket.getFD()+1,&readfds, 0, 0, &tv)) {
    case 1:
            return 1;
    case 0:
      cerr << "(SelectInput) select failed " << strerror(errno) << endl;
      abort();
      return 0;
    default:
      if(errno == EINTR)
        continue;
      cerr << "(SelectInput) select failed " << strerror(errno) << endl;
      abort();
      return 0;
    }
  }
}

void PutMessage(const string& msg) {
  if (msg.empty()) {
    return;
  }
  // prefix the message with it's payload length
  unsigned int len = htonl(msg.size());
  string prefix((const char*)&len,sizeof(unsigned int));
  string str = prefix + msg;
  write(gSocket.getFD(), str.data(), str.size());
}

bool GetMessage(string& msg) {
  static char buffer[16 * 82024]; // Increased to support images from SPL version of sim
  
  unsigned int bytesRead = 0;
  while(bytesRead < sizeof(unsigned int)) {
    SelectInput();
    int readResult = read(gSocket.getFD(), buffer + bytesRead, sizeof(unsigned int) - bytesRead);
    if(readResult < 0)
      continue;
    if (readResult == 0) {
      // [patmac] Kill ourselves if we disconnect from the server
      // for instance when the server is killed.  This helps to
      // prevent runaway agents.
      cerr << "Lost connection to server" << endl;
      Done();
      exit(1);
    }
    bytesRead += readResult;
  }
  
  // msg is prefixed with it's total length
  unsigned int msgLen = ntohl(*(unsigned int*)buffer);
  // cerr << "GM 6 / " << msgLen << " (bytesRead " << bytesRead << ")\n";
  if(sizeof(unsigned int) + msgLen > sizeof(buffer)) {
    cerr << "too long message; aborting" << endl;
    abort();
  }
  
  // read remaining message segments
  unsigned int msgRead = bytesRead - sizeof(unsigned int);
  
  //cerr << "msgRead = |" << msgRead << "|\n";
  
  char *offset = buffer + bytesRead;
  
  while (msgRead < msgLen) {
    if (! SelectInput()) {
      return false;
    }
    
    int readLen = sizeof(buffer) - msgRead;
    if(readLen > msgLen - msgRead)
      readLen = msgLen - msgRead;
    
    int readResult = read(gSocket.getFD(), offset, readLen);
    if(readResult < 0)
      continue;
    msgRead += readResult;
    offset += readResult;
    //cerr << "msgRead = |" << msgRead << "|\n";
  }
  
  // zero terminate received data
  (*offset) = 0;
  
  msg = string(buffer+sizeof(unsigned int));
  
  // DEBUG
  //cout << msg << endl;
  
  return true;
}

void Run() {    
  Behavior *behavior;
  if( agentType == "naoagent" ) {
    behavior = new RobotBehavior(teamName, uNum);
  }
   
  PutMessage(behavior->Init());

  string msg;
  while (gLoop) {  
    GetMessage(msg);
    PutMessage(behavior->Think(msg));
  }
}

int main(int argc, char* argv[]) {
    // registering the handler, catching SIGINT signals 
    signal(SIGINT, handler);

    ReadOptions(argc,argv);

    if (! Init())  {
      return 1;
    }

    Run();
    Done();
}
