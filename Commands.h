#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
/*
* Macro providing a “safe” way to invoke system calls
*/
#define DO_SYS( syscall ) do { \
/* safely invoke a system call */ \
if( (syscall) == -1 ) { \
perror( #syscall ); \
exit(1); \
} \
} while( 0 ) 


/*______________COMMANDS_____________*/

class Command {
// TODO: Add your data members
  protected:
    const std::string cmd_line;
    int num_of_args;
    std::vector<std::string> args;
    bool is_background_command;

  public:
    Command(const char* cmd_line);
    virtual ~Command() = default;
    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() = default;
};

class ExternalCommand : public Command {
  const char* prog = "/bin/bash"; 

 public:
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand() = default;
  void execute() override;
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() = default;
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() = default;
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};


/*_______BUILT_IN_COMMANDS_______*/

class ChpromptCommand : public BuiltInCommand {
  std::string new_name;
  public:
    ChpromptCommand(const char* cmd_line);
    virtual ~ChpromptCommand() = default;
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
    ShowPidCommand(const char* cmd_line);
    virtual ~ShowPidCommand() = default;
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
    GetCurrDirCommand(const char* cmd_line);
    virtual ~GetCurrDirCommand() = default;
    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members 
  const int MAX_NUM_OF_ARGS = 2; 
  const int MIN_NUM_OF_ARGS = 2;
  std::string new_dir; 

public:
    //orig: ChangeDirCommand(const char* cmd_line, char** p_last_pwd);
    ChangeDirCommand(const char* cmd_line);
    virtual ~ChangeDirCommand() = default;
    void execute() override;
};


/*_______EXTERNAL_COMMANDS_______*/



/*__________________COMMANDS_EXCEPTIONS___________________*/

  class Exception : public std::exception {
    protected:
      std::string error_message;
    
    public:
      explicit Exception(std::string error_message);
      virtual ~Exception() = default;
      const char* what() const noexcept override;
  };

/*___________BUILTIN_COMMANDS_EXCEPTIONS_________*/

   class OldPwdNotSet : public Exception {
     public:
      OldPwdNotSet();
      ~OldPwdNotSet() = default;
   };

   class TooManyArgs : public Exception {
     public:
      TooManyArgs();
      ~TooManyArgs() = default;
   };

    class WhyDoYouMakeProblems : public Exception {
     public:
      WhyDoYouMakeProblems();
      ~WhyDoYouMakeProblems() = default;
   };

/*___________EXTERNAL_COMMANDS_EXCEPTIONS_________*/

   


/*
class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
    QuitCommand(const char* cmd_line, JobsList* jobs);
    virtual ~QuitCommand() = default;
    void execute() override;
};




class JobsList {
 public:
    class JobEntry {
    // TODO: Add your data members
    // state - foreground/backround/stopped\
    // to add - if the job needs to be deleted
    };
 // TODO: Add your data members
 // vector(?) of jobEntry
 public:
    JobsList();
    ~JobsList();
    void addJob(Command* cmd, bool isStopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry * getJobById(int jobId);
    void removeJobById(int jobId);
    JobEntry * getLastJob(int* lastJobId);
    JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
    JobsCommand(const char* cmd_line, JobsList* jobs);
    virtual ~JobsCommand() = default;
    void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
    KillCommand(const char* cmd_line, JobsList* jobs);
    virtual ~KillCommand() = default;
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
    ForegroundCommand(const char* cmd_line, JobsList* jobs);
    virtual ~ForegroundCommand() = default;
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
    BackgroundCommand(const char* cmd_line, JobsList* jobs);
    virtual ~BackgroundCommand() = default;
    void execute() override;
};

class TailCommand : public BuiltInCommand {
 public:
    TailCommand(const char* cmd_line);
    virtual ~TailCommand() = default;
    void execute() override;
};

class TouchCommand : public BuiltInCommand {
 public:
    TouchCommand(const char* cmd_line);
    virtual ~TouchCommand() = default;
    void execute() override;
};
*/


/*______________SMALL_SHELL_____________*/

class SmallShell {
  private:
    // TODO: Add your data members
    // add the name of chprom - to change in main to print the name
    // add vector(?) - 100 processes
    // add vector(?) - jobs
    std::string name;
    std::string current_directory;
    std::string last_directory;
    SmallShell();

 public:
    Command *CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
      static SmallShell instance; // Guaranteed to be destroyed.
      // Instantiated on first use.
      return instance;
    }
    ~SmallShell();
    void executeCommand(const char* cmd_line);
    std::string getName();
    std::string getCurrentDirectory();
    std::string getLastDirectory();
    void changeName(std::string new_name);
    void setCurrentDirectory(std::string directory);
    void setLastDirectory(std::string directory);
    // TODO: add extra methods as needed
    
};

#endif //SMASH_COMMAND_H_
