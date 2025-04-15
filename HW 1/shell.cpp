// --------------------------------------------shell.cpp-------------------------------------------
// Aldan Brown CSS 503
// Date Created: 4/9/2025
// Date Modified: 4/14/2025
// ------------------------------------------------------------------------------------------------
// Description: This program implements a simple shell that can execute commands and handle
// input/output redirection. It uses fork and exec to create child processes for executing
// commands. The shell continues to run until the user enters "exit" or "quit". The shell also
// supports background execution of commands using the '&' symbol, repeating commands using '!!',
// redirection of commands using '<' and '>' and finally piping commands together using '|'.
// ------------------------------------------------------------------------------------------------
// Assumptions:
// 1) The !! command will save every command regardless of its success or failure.
// 2) Commands will contain only one pipe character and will not be combined with any redirection
// operators.
// ------------------------------------------------------------------------------------------------

#include <fcntl.h>
#include <iostream>
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
               hasPipe = true; // Pipes to other process
            }
            tokens.push_back(token);
         }
      }

      // ------------------------------------------------------------------------------------------
      // If no command, continue loop
      if (tokens.empty()) {
         continue;
      }

      // ------------------------------------------------------------------------------------------
      // Exit command
      if (tokens[0] == "exit") {
         should_run = false;
         continue;
      }

      // ------------------------------------------------------------------------------------------
      // Check for History command (!!)
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

      // Save previous command
      history = input;

      // ------------------------------------------------------------------------------------------
      // Pipe handling (|)
      if (hasPipe) {
         // Enters command into left until pipe string found, then switches to right
         vector<string> leftCmd, rightCmd;
         bool pipeSeparator = false;

         for (string& tok : tokens) {
            if (tok == "|") {
               pipeSeparator = true;
            } else if (!pipeSeparator) {
               leftCmd.push_back(tok);
            } else {
               rightCmd.push_back(tok);
            }
         }

         // Pipe error check
         if (leftCmd.empty() || rightCmd.empty()) {
            cerr << "Invalid pipe usage" << endl;
            continue;
         }

         // Create pipe
         int pipefd[2];
         if (pipe(pipefd) == -1) {
            cerr << "Pipe creation failed" << endl;
            continue;
         }

         pid_t pid1 = fork();
         // Error check
         if (pid1 < 0) {
            cerr << "Fork failed for first command" << endl;
            continue;
         }

         // Child process
         if (pid1 == 0) {
            // Write to pipe
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);

            // Convert command to c-string
            vector<char*> args;
            for (string& s : leftCmd)
               args.push_back(&s[0]);
            args.push_back(nullptr);

            execvp(args[0], args.data());
            cerr << "Command not found or failed to execute: '" << args[0] << "'" << endl;
            exit(1);
         }

         pid_t pid2 = fork();

         // Error check
         if (pid2 < 0) {
            cerr << "Fork failed for second command" << endl;
            continue;
         }

         // Child process
         if (pid2 == 0) {
            // Read from pipe
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);

            // Convert command to c-string
            vector<char*> args;
            for (string& s : rightCmd)
               args.push_back(&s[0]);
            args.push_back(nullptr);

            execvp(args[0], args.data());
            cerr << "Command not found or failed to execute: '" << args[0] << "'" << endl;
            exit(1);
         }

         // Parent process
         close(pipefd[0]);
         close(pipefd[1]);

         // Wait unless running in background
         if (!background) {
            waitpid(pid1, nullptr, 0);
            waitpid(pid2, nullptr, 0);
         }

         continue;
      }

      // ------------------------------------------------------------------------------------------
      // Handle redirection inputs (<) and outputs (>)
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

      // ------------------------------------------------------------------------------------------
      // Standard command execution
      // Convert command to c-string
      vector<char*> args;
      for (string& s : tokens) {
         args.push_back(&s[0]);
      }
      args.push_back(nullptr);

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
            // Open file in read-only
            int fd = open(iFile.c_str(), O_RDONLY);
            if (fd < 0) { // Error check
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
         if (!background) {
            waitpid(pid, nullptr, 0); // Wait for child unless background
         }
      }
   }

   return 0; // Shell exits
};