// -------------------------------------------stdio.cpp--------------------------------------------
// Aldan Brown CSS 503
// Date Created: 5/20/2025
// Date Modified: 5/20/2025
// ------------------------------------------------------------------------------------------------
// Description: Implementation of "stdio.h"
// ------------------------------------------------------------------------------------------------
// Acknowledgements: Initial code provided by Prof. Robert Dimpsey
// ------------------------------------------------------------------------------------------------

#include "stdio.h"
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
using namespace std;

char decimal[100];

//------------------------------------Constructor/Destructor------------------------------------
// Constructor
FILE::FILE() {
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

//-----------------------------------------Read Functions------------------------------------------
// fopen(char*, char*)
FILE* FILE::fopen(const char* path, const char* mode) {
   FILE* stream = new FILE();
   setvbuf(stream, (char*)0, _IOFBF, BUFSIZ);

   // fopen( ) mode
   // r or rb = O_RDONLY
   // w or wb = O_WRONLY | O_CREAT | O_TRUNC
   // a or ab = O_WRONLY | O_CREAT | O_APPEND
   // r+ or rb+ or r+b = O_RDWR
   // w+ or wb+ or w+b = O_RDWR | O_CREAT | O_TRUNC
   // a+ or ab+ or a+b = O_RDWR | O_CREAT | O_APPEND

   switch (mode[0]) {
   case 'r':
      if (mode[1] == '\0') // r
      {
         stream->flag = O_RDONLY;
      } else if (mode[1] == 'b') {
         if (mode[2] == '\0') // rb
         {
            stream->flag = O_RDONLY;
         } else if (mode[2] == '+') // rb+
         {
            stream->flag = O_RDWR;
         }
      } else if (mode[1] == '+') // r+  r+b
      {
         stream->flag = O_RDWR;
      }
      break;
   case 'w':
      if (mode[1] == '\0') // w
      {
         stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
      } else if (mode[1] == 'b') {
         if (mode[2] == '\0') // wb
         {
            stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
         } else if (mode[2] == '+') // wb+
         {
            stream->flag = O_RDWR | O_CREAT | O_TRUNC;
         }
      } else if (mode[1] == '+') // w+  w+b
      {
         stream->flag = O_RDWR | O_CREAT | O_TRUNC;
      }
      break;
   case 'a':
      if (mode[1] == '\0') // a
      {
         stream->flag = O_WRONLY | O_CREAT | O_APPEND;
      } else if (mode[1] == 'b') {
         if (mode[2] == '\0') // ab
         {
            stream->flag = O_WRONLY | O_CREAT | O_APPEND;
         } else if (mode[2] == '+') // ab+
         {
            stream->flag = O_RDWR | O_CREAT | O_APPEND;
         }
      } else if (mode[1] == '+') // a+  a+b
      {
         stream->flag = O_RDWR | O_CREAT | O_APPEND;
      }
      break;
   }

   mode_t open_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

   // Error check
   if ((stream->fd = open(path, stream->flag, open_mode)) == -1) {
      delete stream;
      printf("fopen failed\n");
      stream = NULL;
   }

   return stream;
}

// fgetc(FILE*)
int fgetc(FILE* stream) {
   // complete it
   return 0;
}

// fgets(char*)
char* fgets(char* str, int size, FILE* stream) {
   // complete it
   return NULL;
}

// fread(void*, size_t, size_t, FILE*)
size_t FILE::fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
   // complete it
   return 0;
}

// fseek(FILE*, long, int)
int FILE::fseek(FILE* stream, long offset, int whence) {
   // complete it
   return 0;
}

// fclose(FILE*)
int FILE::fclose(FILE* stream) {
   // complete it
   return 0;
}

//----------------------------------------Write Functions---------------------------------------
// fwrite(void*, size_t, size_t, FILE*)
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream) {
   // comlete it
   return 0;
}

// fputc(int, FILE*)
int fputc(int c, FILE* stream) {
   // complete it
   return 0;
}

// fputs(char*, FILE*)
int fputs(const char* str, FILE* stream) {
   // complete it
   return 0;
}

// --------------------------------------Auxillary Functions---------------------------------------
// feof(FILE*)
int feof(FILE* stream) { return stream->eof == true; }

// fflush(FILE*)
int FILE::fflush(FILE* stream) {
   // complete it
   return 0;
}

// fpurge(FILE*)
int FILE::fpurge(FILE* stream) {
   // complete it
   return 0;
}

// setvbuf (FILE*, char*, int, size_t)
int FILE::setvbuf(FILE* stream, char* buf, int mode, size_t size) {
   if (mode != _IONBF && mode != _IOLBF && mode != _IOFBF) {
      return -1;
   }
   stream->mode = mode;
   stream->pos = 0;
   if (stream->buffer != (char*)0 && stream->bufown == true) {
      delete stream->buffer;
   }

   switch (mode) {
   case _IONBF:
      stream->buffer = (char*)0;
      stream->size = 0;
      stream->bufown = false;
      break;
   case _IOLBF:
   case _IOFBF:
      if (buf != (char*)0) {
         stream->buffer = buf;
         stream->size = size;
         stream->bufown = false;
      } else {
         stream->buffer = new char[BUFSIZ];
         stream->size = BUFSIZ;
         stream->bufown = true;
      }
      break;
   }
   return 0;
}

// setbuf (FILE*, char*)
void FILE::setbuf(FILE* stream, char* buf) {
   setvbuf(stream, buf, (buf != (char*)0) ? _IOFBF : _IONBF, BUFSIZ);
}

// printf(void*, ...)
int FILE::printf(const void* format, ...) {
   va_list list; // variable argument list type
   va_start(list, format);

   char* msg = (char*)format;
   char buf[1024];
   int nWritten = 0;

   int i = 0, j = 0, k = 0;
   while (msg[i] != '\0') {
      if (msg[i] == '%' && msg[i + 1] == 'd') {
         buf[j] = '\0';
         nWritten += write(1, buf, j);
         j = 0;
         i += 2;

         int int_val = va_arg(list, int);
         char* dec = itoa(abs(int_val));
         if (int_val < 0) {
            nWritten += write(1, "-", 1);
         }
         nWritten += write(1, dec, strlen(dec));
         delete dec;
      } else {
         buf[j++] = msg[i++];
      }
   }
   if (j > 0) {
      nWritten += write(1, buf, j);
   }
   va_end(list);
   return nWritten;
}

// itoa(int)
char* FILE::itoa(const int arg) {
   bzero(decimal, 100);
   int order = recursive_itoa(arg);
   char* new_decimal = new char[order + 1];
   bcopy(decimal, new_decimal, order + 1);
   return new_decimal;
}

// recursive_itoa(int)
int FILE::recursive_itoa(int arg) {
   int div = arg / 10;
   int mod = arg % 10;
   int index = 0;
   if (div > 0) {
      index = recursive_itoa(div);
   }
   decimal[index] = mod + '0';
   return ++index;
}
