//--------------------------------------------lab3.cpp---------------------------------------------
// CSS 503 - Lab 3
// Created by: Aldan Brown
// Date:5/13/2025
//-------------------------------------------------------------------------------------------------

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <iostream>

using namespace std;

struct timeval start, end2; // maintain starting and finishing wall time.

void startTimer(){
   gettimeofday(&start, NULL);
}

void stopTimer(const char* str) {
   gettimeofday(&end2, NULL);
   cout << str << "'s elapsed time\t= " 
   << ((end2.tv_sec - start.tv_sec) * 1000000 + (end2.tv_usec - start.tv_usec)) << endl;
}

int main(int argc, char* argv[]) {

   if (argc != 3) {
      cerr << "usage: lab3 filename bytes" << endl;
      return -1;
   }

   int bytes = atoi(argv[2]);

   if (bytes < 1) {
      cerr << "usage: lab3 filename bytes" << endl;
      cerr << "where bytes > 0" << endl;
      return -1;
   }

   char* filename = argv[1];

   char* buf = new char[bytes];

   // linux i/o

   int fd = open(filename, O_RDONLY);

   if (fd == -1) {
      cerr << filename << " not found" << endl;
      return -1;
   }

   startTimer();

   while (read(fd, buf, bytes) > 0){
      ;
   }
   stopTimer("Unix read");
   close(fd);

   // standard i/o

   // write the same functionality as in linux i/o

   // but use fopen(), fgetc(), fread(), and fclose( )

   // use fgetc() if bytes == 1

   return 0;
}