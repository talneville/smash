# smash
a small shell

includes the following files:
- Commands.h/Commands.cpp: The supported commands of smash, each command is represented by
  a class that inherits from either BuiltInCommand or ExternalCommand.
  Each command has a virtual method called execute, that executes
  the command.
- signals.h/signals.cpp: Declares and implements requires signal handlers: 
  SIGINT handler to handle Ctr+C and SIGTSTP to handle Ctrl+Z.
- smash.cpp: Contains the smash main, which runs an infinite loop that receives the next typed
  command and sends it to SmallShell::executeCommand to handle it.
- Makefile: builds and tests using a basic test your smash.
  You can use "make zip" to prepare a zip file for submission; this is recommended, which makes
  sure you follow our submission's structure. 
- test_input1.txt / test_expected_output1.txt: basic test files that being used by the given 
  Makefile to run a basic test on your smash implementation. 

We use a few known design patterns for making the code
modular and readable, mainly the Singleton and Factory Method.

