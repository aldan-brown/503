#ifndef _MY_STDIO_H_
#define _MY_STDIO_H_

#define BUFSIZ 8192 // default buffer size
#define _IONBF 0    // unbuffered
#define _IOLBF 1    // line buffered.  Do not need to implement.
#define _IOFBF 2    // fully buffered
#define EOF -1      // end of file

class FILE {
 public:
   FILE() {
      fd = 0;
      pos = 0;
      buffer = (char*)0;
      size = 0;
      actual_size = 0;
      mode = _IONBF;
      flag = 0;
      bufown = false;
      lastop = 0;
      eof = false;
   }

   int fd;          // a Unix file descriptor of an opened file
   int pos;         // the current file position in the buffer
   char* buffer;    // an input or output file stream buffer
   int size;        // the buffer size
   int actual_size; // the actual buffer size when read( ) returns # bytes read smaller than size
   int mode;        // _IONBF, _IOLBF, _IOFBF. Do not need to implement _IOLBF
   int flag;        // O_RDONLY
                    // O_RDWR
                    // O_WRONLY | O_CREAT | O_TRUNC
                    // O_WRONLY | O_CREAT | O_APPEND
                    // O_RDWR   | O_CREAT | O_TRUNC
                    // O_RDWR   | O_CREAT | O_APPEND
   bool bufown;     // true if allocated by stdio.h or false by a user
   char lastop;     // 'r' or 'w'
   bool eof;        // true if EOF is reached

};
#include "stdio.cpp"
#endif
