#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

// TO DO: make sure working for bg job after writing kill func
// change jolist private - update func 


//add job from foreground to jobs list

void ctrlZHandler(int sig_num) {
  cout << "smash: got ctrl-Z" << endl;
  SmallShell& smash = SmallShell::getInstance();
  
  if((smash.getCurrentCommand() == nullptr) || (smash.getCurrentCommand()->isBackroundCommand() == true)) {
      return;
  }
  int pid = smash.getCurrentCommand()->getPid();
  kill(pid, SIGSTOP);
  smash.changeStatusByPid(pid, JobsList::JobEntry::STOPPED); 
  cout << "smash: process " << pid << " was stopped" << endl;
}

void ctrlCHandler(int sig_num) {
  cout << "smash: got ctrl-C" << endl;
  SmallShell& smash = SmallShell::getInstance();
  if((smash.getCurrentCommand() == nullptr) || (smash.getCurrentCommand()->isBackroundCommand() == true)) {
      return;
  }
  int pid = smash.getCurrentCommand()->getPid();
  kill(pid, 9);
  cout << "smash: process " << pid << " was killed" << endl;
  //smash.changeStatusByPid(smash.getCurrentForegroundPid(), JobsList::JobEntry::DEAD);
  // to do - remove from job list ?
  //smash.killJobByPid(getCurrentForegroundPid())
}



// void catch_ctrl_c(int sig_num) {
//   printf("YES!!!\n");
//   //   if (getpid() == jobs->smash_pid){
// 	// fflush(stdout);	
//   //       printf("caught ctrl-C\n");
//   //       if ( jobs->foreground_job != NULL ){
//   //           Job* for_job_to_kill = jobs->foreground_job;
//   //           pid_t job_pid = for_job_to_kill->pid;
//   //           int err = kill(job_pid,SIGKILL);// SIGNAL9
//   //           if (err ==-1){
//   //               perror("smash error: kill failed");
//   //               return;
//   //           }
//   //           jobs->foreground_job= NULL;
//   //           printf("process %d was killed\n",job_pid);
//   //       }
//   //   }
//   //   else{
// 	// int err = kill(getpid(),SIGINT);
//   //       if (err ==-1){
//   //           perror("smash error: kill failed");
//   //           return;
//   //       }
//   //   }
// }

// void catch_ctrl_z(int sig_num) {
//   //   if (getpid() == jobs->smash_pid){
//   //       printf("caught ctrl-Z\n");
//   //       fflush(stdout);
//   //       if ( jobs->foreground_job != NULL ){
//   //           Job* for_job_to_stop = jobs->foreground_job;
//   //           pid_t job_pid = for_job_to_stop->pid;
//   //           int err = kill(job_pid,SIGSTOP);// SIGNAL 19
//   //           if (err ==-1){
//   //               perror("smash error: kill failed");
//   //               return ;
//   //           }
// 	//     for_job_to_stop->stopped = true;
//   //           jobs->foreground_job= NULL;
//   //           printf("process %d was stopped\n",job_pid);
// 	// }
//   //   }
//   //   else{
// 	// int err = kill(getpid(),SIGSTOP);
//   //       if (err ==-1){
//   //           perror("smash error: kill failed");
//   //           return;
//   //       }
//   //   }
// }



// void ctrlZHandler(int sig_num) {
//   //print
//   cout << "smash: got ctrl-Z" << endl;
//   //add job from foreground to jobs list
//   SmallShell& smash = SmallShell::getInstance();
//   // check if in fg
//   //int current_pid = smash.getCurrentForegroundPid();
//   if(smash.getCurrentCommand() == nullptr || smash.getCurrentCommand()->isBackroundCommand() == true) {
//       return;
//   }
  
//   // if(current_pid == 0) {
//   //     return;
//   // }
//   //DO_SYS(kill(smash.getCurrentForegroundPid(),SIGSTOP));
//   kill(smash.getCurrentForegroundPid(),SIGSTOP);
//   //change status of pid 
//   cout << "WHY WHY" << endl;
//   smash.changeStatusByPid(current_pid, JobsList::JobEntry::STOPPED); 
//   cout << "smash: process " << current_pid << " was stopped" << endl;
//   smash.changeCurrentForegroundPid(0);
//   //smash.changeCurrentCommand(nullptr);
// }

// void ctrlCHandler(int sig_num) {
//   //print
//   cout << "smash: got ctrl-C" << endl;
//   //get pid of running process
//   SmallShell& smash = SmallShell::getInstance();
//   //kill the process
//   if(smash.getCurrentForegroundPid() == 0) {
//     return;
//   }
//   //int current_pid = smash.getCurrentForegroundPid()
//   kill(smash.getCurrentForegroundPid(), SIGKILL);
//   cout << "smash: process " << smash.getCurrentForegroundPid() << " was killed" << endl;
//   //smash.changeStatusByPid(smash.getCurrentForegroundPid(), JobsList::JobEntry::DEAD);
//   // to do - remove from job list
//   //smash.killJobByPid(getCurrentForegroundPid())
//   smash.changeCurrentForegroundPid(0);
//   //smash.changeCurrentCommand(nullptr);
// }

  
void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}







// void ctrlZHandler(int sig_num) {
// 	// TODO: Add your implementation
//   //print
//   cout << "smash: got ctrl-Z" << endl;
//   //add job to jobs list
//   SmallShell& smash = SmallShell::getInstance();
//   //check it was in jobs list
//   int current_pid = smash.getCurrentForegroundPid();
// void ctrlZHandler(int sig_num) {
// 	// TODO: Add your implementation
//   //print
//   cout << "smash: got ctrl-Z" << endl;
//   //add job to jobs list
//   SmallShell& smash = SmallShell::getInstance();
//   //check it was in jobs list
//   int current_pid = smash.getCurrentForegroundPid();
//   int current_id = smash.jobslist.pidExists(current_pid);
//   if(current_id > 0)
//     // if it was already then only need to change the status
//       JobEntry * entry = smash.jobslist.getJobEntryById(current_id);
//       entry->changeStatus(BACKGROUND);
//   else if(current_pid == 0) {
//       return;
//   }
//   else { 
//       smash.jobslist.addjob(smash.current_command->cmd_line,true);
//   }
 
//   //stop the process
//   kill(smash.getCurrentForegroundPid(),SIGSTOP);
//   cout << "smash: process " << current_pid << " was stopped" << endl;
// }

// void ctrlCHandler(int sig_num) {
//   // TODO: Add your implementation
//   //print
//   cout << "smash: got ctrl-C" << endl;
//   //get pid of running process
//   SmallShell& smash = SmallShell::getInstance();
//   //kill the process
//   if(smash.getCurrentForegroundPid() == 0) {
//     return;
//   }
//   int current_pid = smash.getCurrentForegroundPid()
//   kill(smash.getCurrentForegroundPid(),SIGKILL);
//   cout << "smash: process " << current_pid << " was killed" << endl;
//   }

// void alarmHandler(int sig_num) {
//   // TODO: Add your implementation
// }
//   int current_id = smash.jobslist.pidExists(current_pid);
//   if(current_id > 0)
//     // if it was already then only need to change the status
//       JobEntry * entry = smash.jobslist.getJobEntryById(current_id);
//       entry->changeStatus(BACKGROUND);
//   else if(current_pid == 0) {
//       return;
//   }
//   else { 
//       smash.jobslist.addjob(smash.current_command->cmd_line,true);
//   }
 
//   //stop the process
//   kill(smash.getCurrentForegroundPid(),SIGTSTP);
//   cout << "smash: process " << current_pid << " was stopped" << endl;
// }

// void ctrlCHandler(int sig_num) {
//   // TODO: Add your implementation
//   //print
//   cout << "smash: got ctrl-C" << endl;
//   //get pid of running process
//   SmallShell& smash = SmallShell::getInstance();
//   //kill the process
//   if(smash.getCurrentForegroundPid() == 0) {
//     return;
//   }
//   int current_pid = smash.getCurrentForegroundPid()
//   kill(smash.getCurrentForegroundPid(),SIGKILL);
//   cout << "smash: process " << current_pid << " was killed" << endl;
//   }

// void alarmHandler(int sig_num) {
//   // TODO: Add your implementation
// }

