// -------------------------------------------Server.cpp-------------------------------------------
// Aldan Brown CSS 503
// Date Created: 5/30/2025
// Date Modified: 6/2/2025
// ------------------------------------------------------------------------------------------------
// Description: A client-server model where a client process establishes a connection to a server,
// sends data or requests, and closes the connection.  The server will accept the connection and
// create a thread to service the request and then wait for another connection on the main thread.
// Servicing the request consists of (1) reading the number of iterations the client will perform,
// (2) reading the data sent by the client, and (3) sending the number of reads which the server
// performed.
// ------------------------------------------------------------------------------------------------
// Acknowledgements: Initial code provided by Prof. Robert Dimpsey as "server.cpp"
// ------------------------------------------------------------------------------------------------

#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
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

/** Creates a new thread and executes each client request
 @param arg pointer to the client server descriptor */
void* handleClient(void* arg) {
   // Convert pointer to int and free memory
   int clientSD = *((int*)arg);
   delete (int*)arg;

   char* databuf = new char[BUFFSIZE];
   bzero(databuf, BUFFSIZE);

   // Get number of repetitions
   int32_t repetitions;
   ssize_t bytesReceived = recv(clientSD, &repetitions, sizeof(repetitions), MSG_WAITALL);
   // Error check
   if (bytesReceived <= 0 || bytesReceived != sizeof(repetitions)) {
      cerr << "Failed to receive repetitions" << endl;
      close(clientSD);
      delete[] databuf;
      pthread_exit(nullptr);
   }
   int32_t numRep = ntohl(repetitions);

   // Read in buffers
   int totalReads = 0;
   for (int r = 0; r < numRep; r++) {
      int bytesRead = 0;
      int totalRead = 0;
      while (totalRead < BUFFSIZE) {
         bytesRead = read(clientSD, databuf + totalRead, BUFFSIZE - totalRead);
         // Error check
         if (bytesRead <= 0) {
            cerr << "Connection error or client closed connection during read" << endl;
            close(clientSD);
            delete[] databuf;
            pthread_exit(nullptr);
         }
         totalRead += bytesRead;
         totalReads++;
      }
   }

   // Send number of reads back to client
   int32_t reads_net = htonl(totalReads);
   send(clientSD, &reads_net, sizeof(reads_net), 0);

   // Close connection, free memory, and exit thread
   close(clientSD);
   delete[] databuf;
   pthread_exit(nullptr);
}

int main(int argc, char* argv[]) {
   // Validate input
   if (argc < 2) {
      cerr << "Usage: port" << endl;
      return -1;
   }
   int port = atoi(argv[1]);

   // Build address
   sockaddr_in acceptSocketAddress;
   bzero((char*)&acceptSocketAddress, sizeof(acceptSocketAddress));
   acceptSocketAddress.sin_family = AF_INET;
   acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
   acceptSocketAddress.sin_port = htons(port);

   // Open socket and bind
   int serverSD = socket(AF_INET, SOCK_STREAM, 0);
   const int on = 1;
   setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(int));
   cout << "Socket #: " << serverSD << endl;

   int rc = bind(serverSD, (sockaddr*)&acceptSocketAddress, sizeof(acceptSocketAddress));
   if (rc < 0) {
      cerr << "Bind Failed" << endl;
      return -1;
   }

   // Listen and accept
   listen(serverSD, NUM_CONNECTIONS); // setting number of pending connections

   while (true) { // Loop to accept new connections
      sockaddr_in newSockAddr;
      socklen_t newSockAddrSize = sizeof(newSockAddr);
      int* newSD = new int;
      *newSD = accept(serverSD, (sockaddr*)&newSockAddr, &newSockAddrSize);
      if (*newSD < 0) {
         cerr << "Accept failed" << endl;
         delete newSD;
         continue;
      }
      cout << "Accepted connection on socket: " << *newSD << endl;

      pthread_t threadID;
      if (pthread_create(&threadID, nullptr, handleClient, newSD) != 0) {
         cerr << "Failed to create thread" << endl;
         close(*newSD);
         delete newSD;
      }
      pthread_detach(threadID);
   }

   close(serverSD);

   return 0;
}
