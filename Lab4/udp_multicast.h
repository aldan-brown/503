#ifndef _UDPMULTICAST_H_
#define _UDPMULTICAST_H_

#include <arpa/inet.h> // inet_addr
#include <cstring>
#include <iostream>     // cerr
#include <netinet/in.h> // inet_addr
#include <strings.h>    // bzero, strncpy
#include <sys/socket.h> // socket
#include <sys/types.h>  // socket
#include <unistd.h>

using namespace std;

#define NULL_SD -1
#define BUFSIZE 1024

class UdpMulticast {
 public:
   UdpMulticast(char group[], int port);
   ~UdpMulticast();
   int getClientSocket();
   bool multicast(char buf[]);
   int getServerSocket();
   bool recv(char buf[], int size);

 private:
   int clientSd;
   int serverSd;
   int port;
   char group[BUFSIZE];
};

#endif
