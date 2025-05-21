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
#include <algorithm>
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

/** Recursive function for int to character (ASCII) function. Helper for *itoa
 @param arg digit (integer)
 @return Populates the decimal char array that represents a given int */
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

/** Integer to character array (c string)
 @param arg integer to covert
 @return string of characters */
char* itoa(const int arg) {
   bzero(decimal, 100);
   int order = recursive_itoa(arg);
   char* new_decimal = new char[order + 1];
   bcopy(decimal, new_decimal, order + 1);
   return new_decimal;
}

/** Implementation of the STL printf function. Takes a format string and a variable number of
 *  arguments and prints the formatted output to the standard output stream.
 @param format the string to print
 @return number of bytes written */
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

/** Sets the buffering mode and buffer for the specified output stream. Configures buffering
 *  behavior, optionally assigning a user-provided or system-allocated buffer.
 @param stream pointer to the FILE stream to configure
 @param buf optional buffer to use
 @param mode buffering mode to set
 @param size size of the buffer
 @return 0 on success, -1 otherwise */
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

/** Wrapper around setvbuf to simplify buffer assignment with default size.
 @param stream pointer to the FILE stream to configure
 @param buf buffer to use; if NULL, disables buffering */
void setbuf(FILE* stream, char* buf) {
   setvbuf(stream, buf, (buf != (char*)0) ? _IOFBF : _IONBF, BUFSIZ);
}

/** Synchronizes the output stream with the buffer
 @param stream pointer to the FILE stream to configure
 @return 0 if flush was successful, -1 otherwise  */
int fflush(FILE* stream) {
   // Error checking
   if (!stream || stream->fd < 0) {
      return -1;
   }

   // Check for output stream
   if (stream->lastop == 'w') {
      if (stream->pos > 0 && stream->buffer) {
         // Writer remainder of the buffer to the stream
         ssize_t bytesWritten = write(stream->fd, stream->buffer, stream->pos);
         if (bytesWritten == -1) {
            return -1;
         }

         // Reset stream position and size
         stream->pos = 0;
         stream->actual_size = 0;
      }
   }

   stream->lastop = '\0';
   return 0;
}

/** Clears the input/output stream buffer
 @param stream pointer to the FILE stream to configure
 @return 0 if purge was successful, -1 otherwise  */
int fpurge(FILE* stream) {
   // Error checking
   if (!stream || stream->fd < 0) {
      return -1;
   }

   // Clear the buffer
   if (stream->lastop == 'r' || stream->lastop == 'w') {
      stream->pos = 0;
      stream->actual_size = 0;
      stream->eof = false;
   }

   // Reset operation
   stream->lastop = '\0';
   return 0;
}

/** Checks if the end of file has been reached
 @param stream pointer to the FILE stream to configure
 @return 1 if eof , 0 if otherwise  */
int feof(FILE* stream) { return stream->eof == true; }

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
 @param stream pointer to the FILE stream to configure
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
   // Set last operation to read
   stream->lastop = 'r';

   // Check if unbuffered reading is required and read char
   if (stream->mode == _IONBF) {
      char c;
      ssize_t bytes_read = read(stream->fd, &c, 1);
      if (bytes_read <= 0) {
         stream->eof = true;
         return EOF;
      }
      return static_cast<unsigned char>(c);
   }

   // Buffered reading: checks if buffer is empty and needs to be refilled
   if (stream->pos >= stream->actual_size) {
      ssize_t bytes_read = read(stream->fd, stream->buffer, stream->size);
      // EOF check
      if (bytes_read <= 0) {
         stream->eof = true;
         return EOF;
      }
      // Notes the new filled buffer size and sets position to start of the buffer
      stream->actual_size = bytes_read;
      stream->pos = 0;
   }

   // Read from buffer and return
   return static_cast<unsigned char>(stream->buffer[stream->pos++]);
}

/** Reads a line from the specified stream and stores it into the string pointed to by str. It
  * stops when either (size-1) characters are read, the newline character is read, or the
  * end-of-file is reached, whichever comes first.
 @param str char array where the string will be stored
 @param size number of characters to read, including termination ('\0')
 @param stream pointer to the FILE stream to configure
 @return Pointer to the string of the line read in, or NULL if an error occurs */
char* fgets(char* str, int size, FILE* stream) {
   // Error checking
   if (!stream || stream->fd < 0 || !str || size <= 1) {
      if (str && size > 0) {
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
   // Set last operation to read
   stream->lastop = 'r';


   int currentPos = 0;
   int ch;
   // Loop through all characters using fgetc
   while (currentPos < size - 1) {
      ch = fgetc(stream);
      // EOF check
      if (ch == EOF) {
         if (currentPos == 0) {
            return NULL; 
         }
         break; 
      }

      // Advance to next char
      str[currentPos++] = (char)ch;

      // Break on new line
      if (ch == '\n') {
         break;
      }
   }
   return str;
}

/** Reads data from the given stream into the array pointed to by ptr.
 @param ptr A pointer to a block of memory where the read data will be stored.
 @param size The size, in bytes, of each element to be read.
 @param nmemb The number of elements, each of size bytes, to be read.
 @param stream pointer to the FILE stream to configure
 @return The number of elements successfully read  */
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
   // Error check
   if (!stream || stream->fd < 0 || !ptr || size < 1 || nmemb < 1) {
      return 0;
   }

   // Check if in write mode and resets buffer. Sets read mode
   if (stream->lastop != 'r') {
      fpurge(stream);
      stream->pos = 0;
      stream->actual_size = 0;
   }
   // Set last operation to read
   stream->lastop = 'r';

   char* charArray = (char*)ptr;
   size_t totalBytes = size * nmemb;
   size_t bytesRead = 0;

   // Loop to read in specified amount of bytes or return after reaching eof
   while (bytesRead < totalBytes) {
      // Unbuffered read
      if (stream->mode == _IONBF) {
         ssize_t n = read(stream->fd, charArray + bytesRead, totalBytes - bytesRead);
         // EOF check
         if (n <= 0) {
            stream->eof = true;
            break;
         }
         bytesRead += n;
      } else {
         // Buffered read
         if (stream->pos >= stream->actual_size) {
            ssize_t n = read(stream->fd, stream->buffer, stream->size);
            if (n <= 0) {
               stream->eof = true;
               break;
            }
            stream->actual_size = n;
            stream->pos = 0;
         }
         // Check whether the bytes available are the same as what was requested and performs a copy
         size_t available = stream->actual_size - stream->pos;
         size_t toCopy = min(available, totalBytes - bytesRead);
         memcpy(charArray + bytesRead, stream->buffer + stream->pos, toCopy);
         stream->pos += toCopy;
         bytesRead += toCopy;
      }
   }
   // Return number of full elements read
   return (bytesRead / size);
}

/** Sets the file position to the given offset
 @param stream pointer to the FILE stream to configure
 @param long the offset location from whence
 @param whence the position from where the offset is added
 @return 0 if successfully moved, -1 otherwise */
int fseek(FILE* stream, long offset, int whence) {
   // Error check
   if (!stream || stream->fd < 0) {
      return -1;
   }

   // Flush any buffered data if necessary
   if (stream->lastop == 'w') {
      if (fflush(stream) != 0) {
         return -1;
      }
   }

   // Move the file pointer
   if (lseek(stream->fd, offset, whence) == -1) {
      return -1;
   }

   // Reset buffer
   stream->pos = 0;
   stream->actual_size = 0;

   return 0;
}

/** Close the file
 @param stream pointer to the FILE stream to configure
 @return 0 if successfully closed, -1 otherwise */
int fclose(FILE* stream) {
   // Error check
   if (!stream || stream->fd < 0) {
      return -1;
   }

   // Flush the buffer
   fflush(stream);

   // Close the file
   if (close(stream->fd) == -1) {
      return -1;
   }

   // Ensure no memory leaks
   delete[] stream->buffer;
   delete stream;

   return 0;
}

//----------------------------------------Write Functions---------------------------------------
/** Write a character to a stream 
 @param c the integer representation of the character
 @param stream pointer to the FILE stream to configure
 @return character written as an unsigned char cast to an int, -1 otherwise */
int fputc(int c, FILE* stream) {
   // Error check
   if (!stream || stream->fd < 0) {
      return -1;
   }

   // Check if last mode was read and purge
   if (stream->lastop != 'w') {
      fpurge(stream);
   }
   // Set last operation
   stream->lastop = 'w';

   // Unbuffered write
   if (stream->mode == _IONBF) {
      char ch = (char)c;
      if (write(stream->fd, &ch, 1) != 1) {
         return -1;
      }
      return (unsigned char)c;
   }

   // Flush the buffer
   if (!stream->buffer || stream->pos >= stream->size) {
      if (fflush(stream) != 0) {
         return -1;
      }
   }

   // Write char to the buffer
   stream->buffer[stream->pos++] = (char)c;
   return (unsigned char)c;
}

/** Write a string of characters to a stream
 @param str the pointer to a string of characters
 @param stream pointer to the FILE stream to configure
 @return the number of characters written, -1 otherwise */
int fputs(const char* str, FILE* stream) {
   // Error check
   if (!stream || stream->fd < 0 || !str) {
      return -1;
   }

   int totalWritten = 0;
   // Loop through each character using fputc
   while (*str) {
      if (fputc(*str, stream) == -1) {
         return -1;
      }
      // Advance to next character
      str++;
      totalWritten++;
   }
   return totalWritten;
}

/** Write characters from an array to the stream
 @param ptr the pointer to an array
 @param size the number of bytes in an element
 @param nmemb the number of elements to be written
 @param stream pointer to the FILE stream to configure
 @return the number of elements successfully written*/
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream) {
   // Error check
   if (!stream || stream->fd < 0 || !ptr || size < 1 || nmemb < 1) {
      return 0;
   }

   // Check if read mode and purge
   if (stream->lastop != 'w') {
      fpurge(stream);
   }

   // Set last operation
   stream->lastop = 'w';

   const char* data = (const char*)ptr;
   size_t totalBytes = size * nmemb;
   size_t bytesWritten = 0;

   // Loop until all elements written or eof
   while (bytesWritten < totalBytes) {
      // Unbuffered
      if (stream->mode == _IONBF) {
         ssize_t n = write(stream->fd, data + bytesWritten, totalBytes - bytesWritten);
         if (n <= 0)
            break;
         bytesWritten += n;
      } else {
         // Buffered write
         size_t spaceLeft = stream->size - stream->pos;
         size_t toBuffer = min(spaceLeft, totalBytes - bytesWritten);
         memcpy(stream->buffer + stream->pos, data + bytesWritten, toBuffer);
         stream->pos += toBuffer;
         bytesWritten += toBuffer;

         // Check full buffer
         if (stream->pos >= stream->size) {
            if (fflush(stream) != 0) {
               break;
            }
         }
      }
   }
   return bytesWritten / size;
}