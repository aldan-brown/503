// -------------------------------------------Client.cpp-------------------------------------------
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
// Acknowledgements: Initial code provided by Prof. Robert Dimpsey as "client.cpp"
// ------------------------------------------------------------------------------------------------

#include <arpa/inet.h>
#include <chrono>
#include <climits>
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
   // Argument inputs
   char* serverName;
   char* port;
   int repetition, nbufs, bufsize, type;

   // Socket variables
   struct addrinfo hints;
   struct addrinfo *result, *rp;
   int clientSD = -1;

   /*
    * Argument validation
    */
   if (argc != 7) {
      cerr << "Usage: serverName, port, repetition, nbufs, bufsize, type " << endl;
      return -1;
   }

   serverName = argv[0];

   if (sizeof(argv[1]) != 6) {
      cerr << "Usage: " << argv[1] << " ivalid port" << endl;
      return -1;
   }
   port = argv[1];

   try {
      repetition = atoi(argv[2]);
   } catch (overflow_error& e) {
      cerr << "Usage: " << argv[2] << " too many repetitions" << endl;
      return -1;
   }

   try {
      nbufs = atoi(argv[3]);
   } catch (overflow_error& e) {
      cerr << "Usage: " << argv[3] << " too many buffers" << endl;
      return -1;
   }

   try {
      bufsize = atoi(argv[4]);
   } catch (overflow_error& e) {
      cerr << "Usage: " << argv[4] << " buffer size too large" << endl;
      return -1;
   }

   try {
      type = atoi(argv[5]);
      if (type < 1 || type > 3) {
         cerr << "Invalid type. Defaulting to Type 1";
         type = 1;
      }
   } catch (overflow_error& e) {
      cerr << "Usage: " << argv[5] << " invalid type" << endl;
      return -1;
   }

   // Validate inputs
   if (nbufs * bufsize != 1500) {
      cerr << "nbufs: " << nbufs << ", bufsize: " << bufsize << "when multiplied must equal 1500"
           << endl;
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

   /*
    * Iterate through addresses and connect
    */
   for (rp = result; rp != NULL; rp = rp->ai_next) {
      clientSD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (clientSD == -1) {
         continue;
      }
      /*
       * A socket has been successfully created
       */
      rc = connect(clientSD, rp->ai_addr, rp->ai_addrlen);
      if (rc < 0) {
         cerr << "Connection Failed" << endl;
         close(clientSD);
         return -1;
      } else // success
      {
         break;
      }
   }

   if (rp == NULL) {
      cerr << "No valid address" << endl;
      exit(EXIT_FAILURE);
   } else {
      cout << "Client Socket: " << clientSD << endl;
   }
   freeaddrinfo(result);

   /*
    *  Write and read data over network
    */

   // Send repetitions
   cout << "Sending repetitions" << endl;
   int32_t repetitions_net = htonl(repetition);
   ssize_t numSent = send(clientSD, &repetitions_net, sizeof(repetitions_net), 0);
   // Error check
   if (numSent < 0) {
      cerr << "Repetition did not send" << endl;
   } else if (numSent == 0) {
      cerr << "Connection to server ended abruptly" << endl;
   } else if (numSent != sizeof(repetitions_net)) {
      cerr << "Repetition partialy sent" << endl;
   } else {
      cout << "Repetitions sent successfully" << endl;
   }

   // Create data
   char databuf[nbufs][bufsize];
   for (int i = 0; i < nbufs; i++) {
      for (int j = 0; j < bufsize; j++)
         databuf[i][j] = 'z';
   }

   // Send data by type
   ssize_t bytesWritten;
   struct iovec vector[nbufs];
   auto start = chrono::high_resolution_clock::now(); // start clock
   for (int r = 0; r < repetition; r++){
      if (type == 1) {
         for (int j = 0; j < nbufs; j++) {
            bytesWritten = write(clientSD, databuf[j], bufsize);
         }
      } else if (type == 2) {
         for (int j = 0; j < nbufs; j++) {
            vector[j].iov_base = databuf[j];
            vector[j].iov_len = bufsize;
         }
         bytesWritten = writev(clientSD, vector, nbufs);
      } else {
         bytesWritten = write(clientSD, databuf, nbufs * bufsize);
      }
   }
   auto end = chrono::high_resolution_clock::now(); // End test
   // Calculate duration
   auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
   // Calculate Gbps
   double totalBits = repetition * nbufs * bufsize * 8.0;
   double durationSec = duration / 1e6; // convert usec to sec
   double throughputGbps = (totalBits / durationSec) / 1e9;

   // Write error checking
   if (bytesWritten < 0) {
      cerr << "Bytes not written" << endl;
   } else if (bytesWritten == 0) {
      cerr << "Connection to server abrubtly terminated" << endl;
   } else if (bytesWritten != 1500) {
      cerr << "Bytes not fully written" << endl;
   } else {
      cout << "Write operation successful" << endl;
   }

   // Get number of runs from server
   int32_t runs_net;
   ssize_t bytesReceived = recv(clientSD, &runs_net, sizeof(runs_net), MSG_WAITALL);
   // Error check
   if (bytesReceived < 0) {
      cerr << "Did not receive runs" << endl;
   } else if (bytesReceived == 0) {
      cerr << "Connection to server abrubtly terminated" << endl;
   } else if (bytesReceived != sizeof(runs_net)) {
      cerr << "Partial read of runs" << endl;
   }
   int32_t numRuns = ntohl(runs_net);

   // Successful operation output
   cout << "Test(" << type << ") : time = " << duration << " usec, #reads = " << numRuns
        << ", throughput = " << throughputGbps << " Gbps " << endl;

   close(clientSD);
   return 0;
}
