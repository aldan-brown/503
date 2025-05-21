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

// recursive_itoa(int)
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

// itoa(int)
char* itoa(const int arg) {
   bzero(decimal, 100);
   int order = recursive_itoa(arg);
   char* new_decimal = new char[order + 1];
   bcopy(decimal, new_decimal, order + 1);
   return new_decimal;
}

// printf(void*, ...)
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

// setvbuf (FILE*, char*, int, size_t)
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

// setbuf (FILE*, char*)
void setbuf(FILE* stream, char* buf) {
   setvbuf(stream, buf, (buf != (char*)0) ? _IOFBF : _IONBF, BUFSIZ);
}

// fflush(FILE*)
int fflush(FILE* stream) {
   // Error checking
   if (!stream || stream->fd < 0) {
      return EOF; // Return EOF to indicate an error
   }

   // If the stream is in write mode, flush the output buffer
   if (stream->lastop == 'w' || stream->lastop == 'a') {
      // Write data from the buffer to the file
      ssize_t bytesWritten = write(stream->fd, stream->buffer, stream->pos);

      // If writing fails, return EOF
      if (bytesWritten == -1) {
         return EOF;
      }

      // Reset buffer position after flush
      stream->pos = 0;
      stream->actual_size = 0; // Reset buffer content as it is now written
   }

   return 0;
}

// fpurge(FILE*)
int fpurge(FILE* stream) {
   // Error checking
   if (!stream || stream->fd < 0) {
      return EOF; // Return EOF to indicate an error
   }

   // Clear the input buffer (if any)
   if (stream->lastop == 'r') {
      // Simply reset buffer state, discarding the current contents
      stream->pos = 0;
      stream->actual_size = 0;
      stream->eof = false; // Ensure the end-of-file state is reset
   }

   return 0;
}

//-----------------------------------------Read Functions------------------------------------------
/** Opens a file given file name
    @param path A string representing the name of the file to be opened. This can include an
   absolute or relative path.
    @param mode A string representing the mode in which the file should be opened
    @return a FILE pointer if successfully opened or NULL if not */
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

   // Error check
   if ((stream->fd = open(path, stream->flag, open_mode)) == -1) {
      delete stream;
      printf("fopen failed\n");
      stream = NULL;
   }

   return stream;
}

/** Gets the next character from the specified file and advances the position indicator for the
  * stream.
 @param stream input file
 @return Returns the character read from the stream as an unsigned char cast to an int. If the
         end-of-file is encountered or an error occurs, the function returns -1 */
int fgetc(FILE* stream) {
   // Check if stream is null or file descriptor is invalid
   if (!stream || stream->fd < 0) {
      return -1;
   }

   // Check if in write mode and resets buffer. Sets read mode
   if (stream->lastop != 'r') {
      fpurge(stream);
      stream->pos = 0;
      stream->actual_size = 0;
   }
   stream->lastop = 'r';

   // Checks if buffer is empty and needs to be refilled
   if (stream->pos >= stream->actual_size) {
      ssize_t bytes_read = read(stream->fd, stream->buffer, stream->size);
      // EOF check
      if (bytes_read <= 0) {
         stream->eof = true;
         return -1;
      }
      // Notes the new filled buffer size and sets position to start of the buffer
      stream->actual_size = bytes_read;
      stream->pos = 0;
   }

   return static_cast<unsigned char>(stream->buffer[stream->pos++]);
}

/** Reads a line from the specified stream and stores it into the string pointed to by str. It
  * stops when either (size-1) characters are read, the newline character is read, or the
  * end-of-file is reached, whichever comes first.
 @param str char array where the string will be stored
 @param size number of characters to read, including termination ('\0')
 @param stream input file
 @return Pointer to the string of the line read in, or NULL if an error occurs */
char* fgets(char* str, int size, FILE* stream) {
   // Error checking
   if (!stream || stream->fd < 0 || !str || size <= 1) {
      if (str && size > 0){
         str[0] = '\0';
      }
      return NULL;
   }

   // Check if in write mode and resets buffer. Sets read mode
   if (stream->lastop != 'r') {
      fpurge(stream);
      stream->pos = 0;
      stream->actual_size = 0;
   }
   stream->lastop = 'r';

   // Loop until characters read or new line reached
   int currentPos = 0;
   while (currentPos < size - 1) {
      // Checks if buffer is empty and needs to be refilled
      if (stream->pos >= stream->actual_size) {
         ssize_t bytes_read = read(stream->fd, stream->buffer, stream->size);
         // EOF check
         if (bytes_read <= 0) {
            stream->eof = true;
            // No characters read because eof reached
            if(currentPos == 0){
               return NULL;
            }
            break;
         }
         // Notes the new filled buffer size and sets position to start of the buffer
         stream->actual_size = bytes_read;
         stream->pos = 0;
      }
      // Pulls in next 
      char currentChar = stream->buffer[stream->pos++];
      str[currentPos] = currentChar;

      // Stop on new line char
      if(currentChar == '\n'){
         currentPos++;
         break;
      }
      currentPos++;
   }

   // Set the end of the string
   str[currentPos] = '\0';
   return str;
}


/** Reads data from the given stream into the array pointed to by ptr.
 @param ptr A pointer to a block of memory where the read data will be stored.
 @param size The size, in bytes, of each element to be read.
 @param nmemb The number of elements, each of size bytes, to be read.
 @param stream input file
 @return The number of elements successfully read or zero if an error occurs */
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
   // Error check
   if (!stream || stream->fd < 0 || size < 1 || nmemb < 1) {
      return 0;
   }

   // Check if in write mode and resets buffer. Sets read mode
   if (stream->lastop != 'r') {
      fpurge(stream);
      stream->pos = 0; 
      stream->actual_size = 0;
   }
   stream->lastop = 'r';

   char* charArray = (char*)ptr;
   size_t totalBytes = size * nmemb;
   size_t bytesRead = 0;

   while(bytesRead < totalBytes ){
      if (stream->pos >= stream->actual_size) {
         ssize_t bytes_read = read(stream->fd, stream->buffer, stream->size);
         // EOF check
         if (bytes_read <= 0) {
            stream->eof = true;
            // No characters read because eof reached
            break;
         }
         // Notes the new filled buffer size and sets position to start of the buffer
         stream->actual_size = bytes_read;
         stream->pos = 0;
      }

      // Check available bytes in the buffer against the needed amount
      size_t bufferAvailable = stream->actual_size - stream->pos;
      size_t remaining = totalBytes - bytesRead;
      size_t bytesToCopy = (bufferAvailable < remaining) ? bufferAvailable : remaining;

      // Copy from internal buffer to output pointer
      memcpy(charArray + bytesRead, stream->buffer + stream->pos, bytesToCopy);

      stream->pos += bytesToCopy;
      bytesRead += bytesToCopy;
   }

   // Return number of full elements read
   return bytesRead / size;
}

/** Sets the file position to the given offset
 @param stream input file
 @param long the offset location from whence
 @param whence the position from where the offset is added
 @return Zero if successfully moved and non-zero if an error occurred */
int fseek(FILE* stream, long offset, int whence) {
   // Error check
   if (!stream || stream->fd < 0) {
      return 1;
   }

   // Flush any buffered data if necessary
   if (stream->lastop == 'w' || stream->lastop == 'a') {
      if (fflush(stream) != 0) {
         return 1; 
      }
   }

   // Move the file pointer
   if (lseek(stream->fd, offset, whence) == -1) {
      return 1; 
   }

   // Reset buffer 
   stream->pos = 0;
   stream->actual_size = 0;

   return 0; 
}

// fclose(FILE*)
int fclose(FILE* stream) {
   //Error check
   if (!stream || stream->fd < 0) {
      return -1; 
   }

   // Close the file descriptor
   if (close(stream->fd) == -1) {
      return -1; 
   }

   // Free any dynamically allocated resources
   delete[] stream->buffer; // Release buffer if it was dynamically allocated
   delete stream;           // Free the FILE object

   return 0; // Successfully closed the file
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