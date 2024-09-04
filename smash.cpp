#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"



int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler) == SIG_ERR) {
         perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler) == SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    // struct sigaction act_sigint =  { 0 };
    // act_sigint.sa_handler = &ctrlCHandler;
    // sigaction(2, &act_sigint, NULL); //2 SIGINT

    // struct sigaction act_sigstp =  { 0 };
    // act_sigstp.sa_handler = &ctrlZHandler;
    // sigaction(20, &act_sigstp, NULL); // 20 SIGTSTP

    //TODO: setup sig alarm handler
    
    SmallShell& smash = SmallShell::getInstance();
    while(smash.isRunning() == true) {
        
        std::cout << smash.getName() << "> ";
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}