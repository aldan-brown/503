// --------------------------------------------shell.cpp-------------------------------------------
// Aldan Brown CSS 503
// Date Created: 4/9/2025
// Date Modified: 4/9/2025
// ------------------------------------------------------------------------------------------------
// Description: This program implements a simple shell that can execute commands and handle
// input/output redirection. It uses fork and exec to create child processes for executing
// commands. The shell continues to run until the user enters "exit" or "quit". The shell also
// supports background execution of commands using the '&' symbol, repeating commands using "!!",
// and connection of commands together using "<" and ">".
// ------------------------------------------------------------------------------------------------

#include <fcntl.h>
#include <iostream>
#include <queue>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using namespace std;

/** Determines maximum character count on the console */
const int MAX_LINE = 80;

int main() {
   string input, history;  // Command line inputs and a copy for history
   bool should_run = true; // Exit command receipt

   while (should_run) {
      cout << "osh> " << flush; // Prompt for user

      getline(cin, input); // Pull in line upon hitting enter

      if (input.empty()) {
         continue; // If no value, reprompt
      }

      // Pull in line from console into a stream, read each command, and enter into string vector
      istringstream iss(input);
      string token;
      vector<string> tokens;
      bool background = false;
      bool hasPipe = false;

      // Add commands to vector
      while (iss >> token) {
         if (token == "&") {
            background = true; // Run process in background
         } else {
            if (token == "|") {
               hasPipe == true; // Pipes to other process
            }
            tokens.push_back(token);
         }
      }

      // If no command, continue loop
      if (tokens.empty()) {
         continue;
      }

      // Exit command
      if (tokens[0] == "exit") {
         should_run = false;
         continue;
      }

      // Check for repeat command
      if (tokens[0] == "!!") {
         if (history.empty()) {
            cout << "No command in history" << endl;
            continue;
         } else {
            cout << history << endl;
            input = history;
            // Re-tokenize and execute the last command
            iss.clear();
            iss.str(history);
            tokens.clear();
            while (iss >> token) {
               if (token == "&") {
                  background = true;
               } else {
                  if (token == "|") {
                     hasPipe = true;
                  }
                  tokens.push_back(token);
               }
            }
         }
      }

      // Handle file inputs (<) and outputs (>)
      bool fileInput = false;
      bool fileOutput = false;
      string iFile, oFile;

      for (int i = 0; i < tokens.size(); i++) {
         if (tokens[i] == "<") {
            fileInput = true;
            if (tokens.size() > (i + 1)) { // Check file name exists
               iFile = tokens[i + 1];

               // Delete < and file name before parsing to args
               tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
            }
            break;
         } else if (tokens[i] == ">") {
            fileOutput = true;
            if (tokens.size() > (i + 1)) { // Check file name exists
               oFile = tokens[i + 1];

               // Delete > and file name before parsing to args
               tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
            }
            break;
         }
      }

      // Convert vector<string> to char* array for execvp
      vector<char*> args;
      for (string& s : tokens) {
         args.push_back(&s[0]); // Get pointer to string's internal buffer
      }
      args.push_back(nullptr); // Null-terminate for execvp

      // Fork and exec
      pid_t pid = fork();

      if (pid < 0) {
         // Process error
         cerr << "Fork failed." << endl;
         exit(1);
      } else if (pid == 0) {
         // Child process
         // Check input
         if (fileInput) {
            int fd = open(iFile.c_str(), O_RDONLY); // open file in read-only
            if (fd < 0) {                           // Error check
               cerr << "Input file open failed: " << iFile << endl;
               exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
         }
         // Check output
         if (fileOutput) {
            // Open file write only, create/truncate if needed, and set permissions
            int fd = open(oFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { // Error check
               cerr << "Output file open failed: " << oFile << endl;
               exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
         }

         // Execute command
         execvp(args[0], args.data());
         cerr << "Command not found: " << args[0] << endl;
         exit(1);
      } else {
         // Parent process
         history = input; // Save last command (success)
         if (!background) {
            waitpid(pid, nullptr, 0); // Wait for child unless background
         }
      }
   }

   return 0; // Shell exits
};