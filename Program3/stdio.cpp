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

// recursive_itoa
// Recursive Integer to Char (ASCII) conversion.  Helper for *itoa
// Populates the decimal char array that represents a given int
int recursive_itoa(int arg) {
   int div = arg / 10;
   int mod = arg % 10;
   int index = 0;
   if (div > 0) {
      index = recursive_itoa(div);
   }
   decimal[index] = mod + '0';
   return ++index;
}

// *itoa
// Integer to character array (c string)
char* itoa(const int arg) {
   bzero(decimal, 100);
   int order = recursive_itoa(arg);
   char* new_decimal = new char[order + 1];
   bcopy(decimal, new_decimal, order + 1);
   return new_decimal;
}

// printf
// Implementation of the STL printf function
// Takes a format string and a variable number of arguments
//  and prints the formatted output to the standard output stream
int printf(const void* format, ...) {
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

// setvbuf
// Sets the buffering mode and the size of the buffer for a stream
// Returns 0 if successful, EOF/-1 if using an unsupported mode
int setvbuf(FILE* stream, char* buf, int mode, size_t size) {
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

// setbuf
// Sets the buffering mode and the size of the buffer for a stream
// Will use the defualt buffer size of 8192 if a buffer is null
void setbuf(FILE* stream, char* buf) {
   setvbuf(stream, buf, (buf != (char*)0) ? _IOFBF : _IONBF, BUFSIZ);
}

// *fopen
// Opens a file with the specified mode and returns a file pointer.
// It uses the open system call to open the file and sets the buffering mode to fully
//  buffered with a buffer size of 8192 using setvbuf.
FILE* fopen(const char* path, const char* mode) {
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

   if ((stream->fd = open(path, stream->flag, open_mode)) == -1) {
      delete stream;
      printf("fopen failed\n");
      stream = NULL;
   }

   return stream;
}

int fpurge(FILE* stream) {
   // complete it
   return 0;
}

int fflush(FILE* stream) {
   // comlete it
   return 0;
}

// ** Replace with your own comments **
// fread (Sample input/return parameter comments - For full behavior, consult C documentation for
// stdio functions) Reads data from a given stream into a ptr buffer Input parameters: ptr = pointer
// to the buffer where the data read from the file will be stored
//					 size = the size(in bytes) of each element to be read,
//					 nmemb(count) = the number of elements that will be read from the file, each one with
//"size" bytes 					 stream = pointer to the file to read from
// Returns: 		 total number of elements read.  (Note: can be less than requested items)
//					 (Note: size_t is an unsigned integer type that is often used as the return type to
// represent 					   returned sizes of objects.  By using size_t vs int, you can guarantee
// a non-neg value that can represent 					   the sizes of the largest objects possible in
// memory)
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
   // complete it
   return 0;
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream) {
   // comlete it
   return 0;
}

int fgetc(FILE* stream) {
   // complete it
   return 0;
}

int fputc(int c, FILE* stream) {
   // complete it
   return 0;
}

char* fgets(char* str, int size, FILE* stream) {
   // complete it
   return NULL;
}

int fputs(const char* str, FILE* stream) {
   // complete it
   return 0;
}

int feof(FILE* stream) { return stream->eof == true; }

int fseek(FILE* stream, long offset, int whence) {
   // complete it
   return 0;
}

int fclose(FILE* stream) {
   // complete it
   return 0;
}
