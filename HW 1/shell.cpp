// --------------------------------------------shell.cpp-------------------------------------------
// Aldan Brown CSS 503
// Date Created: 4/9/2025
// Date Modified: 4/9/2025
// ------------------------------------------------------------------------------------------------
// Description: This program implements a simple shell that can execute commands and handle
// input/output redirection. It uses fork and exec to create child processes for executing commands.
// The shell continues to run until the user enters "exit" or "quit". The shell also supports
// background execution of commands using the '&' symbol.
// ------------------------------------------------------------------------------------------------

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
   string input, history;
   bool should_run = true;

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

      // Add commands to vector
      while (iss >> token) {
         if (token == "&") {
            background = true; // Run program in background
         } else {
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
                  tokens.push_back(token);
               }
            }
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
         cerr << "Fork failed." << endl;
         exit(1);
      } else if (pid == 0) {
         // Child process
         execvp(args[0], args.data()); // Execute command
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