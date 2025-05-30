// -------------------------------------------Server.cpp-------------------------------------------
// Aldan Brown CSS 503
// Date Created: 5/30/2025
// Date Modified: 5/30/2025
// ------------------------------------------------------------------------------------------------
// Description: A client-server model where a client process establishes a connection to a server,
// sends data or requests, and closes the connection.  The server will accept the connection and
// create a thread to service the request and then wait for another connection on the main thread.
// Servicing the request consists of (1) reading the number of iterations the client will perform,
// (2) reading the data sent by the client, and (3) sending the number of reads which the server
// performed.
// ------------------------------------------------------------------------------------------------
// Notes:
// 1)
// ------------------------------------------------------------------------------------------------
// Assumptions:
// 1) 
// ------------------------------------------------------------------------------------------------
// Acknowledgements: Initial code provided by Prof. Robert Dimpsey as "server.cpp"
// ------------------------------------------------------------------------------------------------

#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
using namespace std;

const int BUFFSIZE = 1500;
const int NUM_CONNECTIONS = 5;

int main(int argc, char* argv[]) {
   int serverPort;
   char* serverName;
   char databuf[BUFFSIZE];
   bzero(databuf, BUFFSIZE);

   /*
    * Build address
    */
   int port = 12345;
   sockaddr_in acceptSocketAddress;
   bzero((char*)&acceptSocketAddress, sizeof(acceptSocketAddress));
   acceptSocketAddress.sin_family = AF_INET;
   acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
   acceptSocketAddress.sin_port = htons(port);

   /*
    *  Open socket and bind
    */
   int serverSD = socket(AF_INET, SOCK_STREAM, 0);
   const int on = 1;
   setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(int));
   cout << "Socket #: " << serverSD << endl;

   int rc = bind(serverSD, (sockaddr*)&acceptSocketAddress, sizeof(acceptSocketAddress));
   if (rc < 0) {
      cerr << "Bind Failed" << endl;
   }

   /*
    *  listen and accept
    */
   listen(serverSD, NUM_CONNECTIONS); // setting number of pending connections
   sockaddr_in newSockAddr;
   socklen_t newSockAddrSize = sizeof(newSockAddr);
   int newSD = accept(serverSD, (sockaddr*)&newSockAddr, &newSockAddrSize);
   cout << "Accepted Socket #: " << newSD << endl;

   /*
    *  read from the socket
    */
   int bytesRead = read(newSD, databuf, BUFFSIZE);
   cout << "Bytes Read: " << bytesRead << endl;
   cout << databuf[0] << endl;

   /*
    *  write to socket
    */
   databuf[13] = 'R';
   int bytesWritten = write(newSD, databuf, BUFFSIZE);
   cout << "Bytes Written: " << bytesWritten << endl;

   close(newSD);
   close(serverSD);

   return 0;
}
