// -------------------------------------------Client.cpp-------------------------------------------
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
// Notes:
// 1)
// ------------------------------------------------------------------------------------------------
// Assumptions:
// 1)
// ------------------------------------------------------------------------------------------------
// Acknowledgements: Initial code provided by Prof. Robert Dimpsey as "client.cpp"
// ------------------------------------------------------------------------------------------------

#include <arpa/inet.h>
#include <chrono>
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

int main(int argc, char* argv[]) {
   // Socket variables
   struct addrinfo hints;
   struct addrinfo *result, *rp;
   int clientSD = -1;

   // Check input number
   if (argc != 7) {
      cerr << "Usage: serverName, port, repetition, nbufs, bufsize, type " << endl;
      return -1;
   }

   // Parse arguments
   char* serverName = argv[1];
   char* port = argv[2];
   int repetition = atoi(argv[3]);
   int nbufs = atoi(argv[4]);
   int bufsize = atoi(argv[5]);
   int type = atoi(argv[6]);

   // Validate inputs
   if (nbufs * bufsize != BUFFSIZE) {
      cerr << "Usage: nbufs * bufsize must equal " << BUFFSIZE << endl;
      return -1;
   }
   if (type < 1 || type > 3) {
      cerr << "Usage: Type must be 1, 2, or 3." << endl;
      return -1;
   }

   /*
    * Use getaddrinfo() to get addrinfo structure corresponding to serverName / Port
    * This addrinfo structure has internet address which can be used to create a socket too
    */
   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6*/
   hints.ai_socktype = SOCK_STREAM; /* TCP */
   hints.ai_flags = 0;              /* Optional Options*/
   hints.ai_protocol = 0;           /* Allow any protocol*/

   int rc = getaddrinfo(serverName, port, &hints, &result);
   if (rc != 0) {
      cerr << "ERROR: " << gai_strerror(rc) << endl;
      exit(EXIT_FAILURE);
   }

   // Iterate through addresses and connect
   clientSD = -1;
   for (rp = result; rp != NULL; rp = rp->ai_next) {
      int sd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sd == -1) {
         continue;
      }
      if (connect(sd, rp->ai_addr, rp->ai_addrlen) == 0) {
         clientSD = sd; // successful
         break;
      }
      close(sd); // failed connect, cleanup
   }
   // Error checks and outputs for successful connections
   if (rp == NULL) {
      cerr << "No valid address" << endl;
      exit(EXIT_FAILURE);
   } else {
      cout << "Client Socket: " << clientSD << endl;
   }
   freeaddrinfo(result);
   if (clientSD == -1) {
      cerr << "Could not connect to server" << endl;
      return -1;
   }
   cout << "Connected to server on socket: " << clientSD << endl;

   // Write and read over network
   // Send repetitions
   cout << "Sending repetitions" << endl;
   int32_t repetitions_net = htonl(repetition);
   ssize_t numSent = send(clientSD, &repetitions_net, sizeof(repetitions_net), 0);
   // Error check
   if (numSent != sizeof(repetitions_net)) {
      cerr << "Failed to send repetitions to server." << endl;
      close(clientSD);
      return -1;
   }

   // Create data buffers
   char databuf[nbufs][bufsize];
   for (int i = 0; i < nbufs; i++) {
      for (int j = 0; j < bufsize; j++)
         databuf[i][j] = 'z';
   }

   // Send data by type
   ssize_t bytesWritten;
   struct iovec vector[nbufs];
   auto start = chrono::high_resolution_clock::now(); // start clock
   /*
    * Repetition loop for reads
    * Type 1 (multiple writes): invokes the write() system call for each data buffer, thus
    * resulting in calling as many write()s as the number of data buffers, (i.e., nbufs).
    * Type 2 (writev): allocates an array of iovec data structures, each having its *iov_base field
    * point to a different data buffer as well as storing the buffer size in its iov_len field; and
    * thereafter calls writev() to send all data buffers at once
    * Type 3 (single write):  allocates an nbufs-sized array of data buffers, and thereafter calls
    * write() to send this array, (i.e., all data buffers) at once
    */
   for (int r = 0; r < repetition; r++) {
      if (type == 1) {
         for (int j = 0; j < nbufs; j++) {
            int totalSent = 0;
            while (totalSent < bufsize) {
               bytesWritten = write(clientSD, databuf[j] + totalSent, bufsize - totalSent);
               // Error check
               if (bytesWritten < 0) {
                  cerr << "Error writing to socket (type 1)" << endl;
                  close(clientSD);
                  return -1;
               }
               totalSent += bytesWritten;
            }
         }
      } else if (type == 2) {
         for (int j = 0; j < nbufs; j++) {
            vector[j].iov_base = databuf[j];
            vector[j].iov_len = bufsize;
         }
         bytesWritten = writev(clientSD, vector, nbufs);
         // Error check
         if (bytesWritten < nbufs * bufsize) {
            cerr << "Error writing to socket (type 2)" << endl;
            close(clientSD);
            return -1;
         }
      } else {
         int totalSent = 0;
         while (totalSent < BUFFSIZE) {
            bytesWritten = write(clientSD, databuf, BUFFSIZE - totalSent);
            // Error check
            if (bytesWritten < 0) {
               cerr << "Error writing to socket (type 3)" << endl;
               close(clientSD);
               return -1;
            }
            totalSent += bytesWritten;
         }
      }
   }

   // End test & calculate duration
   auto end = chrono::high_resolution_clock::now();
   auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();

   // Get number of runs from server
   int32_t runs_net;
   ssize_t bytesReceived = recv(clientSD, &runs_net, sizeof(runs_net), MSG_WAITALL);

   // Error check
   if (bytesReceived != sizeof(runs_net)) {
      cerr << "Failed to receive read count from server." << endl;
      close(clientSD);
      return -1;
   }

   // Convert runs_net
   int32_t numRuns = ntohl(runs_net);

   // Calculate Gbps
   double totalBits = repetition * nbufs * bufsize * 8.0;
   double durationSec = duration / 1e6; // convert usec to sec
   double throughputGbps = (totalBits / durationSec) / 1e9;

   // Successful operation output
   cout << "Test(" << type << ") : time = " << duration << " usec, #reads = " << numRuns
        << ", throughput = " << throughputGbps << " Gbps " << endl;

   close(clientSD);
   return 0;
}
