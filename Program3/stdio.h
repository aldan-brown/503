// --------------------------------------------stdio.h---------------------------------------------
// Aldan Brown CSS 503
// Date Created: 5/20/2025
// Date Modified: 5/20/2025
// ------------------------------------------------------------------------------------------------
// Description: This program is a self-implementation of core input and output functions in the
// C/C++ standard I/O library : <stdio.h>.
// ------------------------------------------------------------------------------------------------
// Notes:
// 1) Many files were already provided: stdio.h, stdio.cpp, eval.cpp, driver.cpp, compile.h,
// eval_tests.sh, and a number of test .txt files.
// 2) The stdio.h and stdio.cpp files have been reformatted for clarity including a reorder of
// functions to better match project spec (program_3.docx), comment formatting, function
// declarations in stdio.h, and more.
// ------------------------------------------------------------------------------------------------
// Assumptions:
// ------------------------------------------------------------------------------------------------
// Acknowledgements: Initial code provided by Prof. Robert Dimpsey
// ------------------------------------------------------------------------------------------------

#ifndef _MY_STDIO_H_
#define _MY_STDIO_H_

#define BUFSIZ 8192 // default buffer size
#define _IONBF 0    // unbuffered
#define _IOLBF 1    // line buffered.  Do not need to implement.
#define _IOFBF 2    // fully buffered
#define EOF -1      // end of file

class FILE {
 public:
   //------------------------------------Constructor/Destructor------------------------------------
   /** Default constructor */
   FILE();

   //----------------------------------------Read Functions----------------------------------------
   /** Opens a file given file name
    @param path A string representing the name of the file to be opened. This can include an absolute
                  or relative path.
    @param mode A string representing the mode in which the file should be opened
    @return a FILE pointer if successfully opened or NULL if not */
   FILE* fopen(const char* path, const char* mode);

   /** Gets the next character from the specified stream and advances the position indicator for the 
     * stream.
    @param stream input stream
    @return Returns the character read from the stream as an unsigned char cast to an int. If the 
            end-of-file is encountered or an error occurs, the function returns -1 */
   int fgetc(FILE* stream);

   /** Reads a line from the specified stream and stores it into the string pointed to by str. It
     * stops when either (size-1) characters are read, the newline character is read, or the 
     * end-of-file is reached, whichever comes first.
    @param str char array where the string will be stored
    @param size number of characters to read, including termination ('\0')
    @param stream input stream 
    @return Pointer to the string of the line read in, or NULL if an error occurs */
   char* fgets(char* str, int size, FILE* stream);


   size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
   int fseek(FILE* stream, long offset, int whence);
   int fclose(FILE* stream);

   //----------------------------------------Write Functions---------------------------------------
   size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);
   int fputc(int c, FILE* stream);
   int fputs(const char* str, FILE* stream);

   //--------------------------------------Auxillary Functions-------------------------------------
   int feof(FILE* stream);
   int fflush(FILE* stream);
   int fpurge(FILE* stream);
   int setvbuf(FILE* stream, char* buf, int mode, size_t size);
   void setbuf(FILE* stream, char* buf);
   int printf(const void* format, ...);
   char* itoa(const int arg);
   int recursive_itoa(int arg);

   //------------------------------------------Variables-------------------------------------------
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
