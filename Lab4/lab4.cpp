#include "udp_multicast.h"

#include <iostream>

using namespace std;

#define SIZE 1024

int main(int argc, char* argv[]) {
   // validate arguments
   if (argc < 3) {
      cerr << "usage: lab4 group port [message]" << endl;
      return -1;
   }

   char* group = argv[1];
   int port = atoi(argv[2]);
   if (port < 5001) {
      cerr << "usage: lab4 group port [message]" << endl;
      return -1;
   }

   char* message = (argc == 4) ? argv[3] : NULL;

   // if message is null, the program should behave as a server,
   // otherwise as a client.
   UdpMulticast udp(group, port);
   if (message != NULL) {
      // client
      // implement yourself

   } else {
      udp.getServerSocket();
      while (true) {
         // server
         // implement yourself
      }
   }

   return 0;
}