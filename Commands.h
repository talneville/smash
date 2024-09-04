#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string.h>
#include <map>


#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
/*
* Macro providing a “safe” way to invoke system calls
*/
/* #define DO_SYS( syscall )                                                  \                                        \
    std::string syscall_name = #syscall;                                   \
    std::string error_message = "smash error: " + syscall_name + " failed";\
    perror( error_message.c_str() );                                       \
*/
/*_____________________________COMMANDS_______________________________*/

class Command {
  protected:
    std::string original_cmd_line;
    std::string cmd_line;
    int num_of_args;
    std::vector<std::string> args;  
    bool is_background_command;
    int pid;

  public:
    Command(const char* cmd_line, int pid = -1);
    virtual ~Command() = default;
    virtual void execute() = 0;
    bool isBackroundCommand() const;
    bool isForegroundCommand() const;
    void changePid(int pid);
    void changeStateToFg(bool fg);
    int getPid() const;
    const std::vector<std::string> getArgs() const;
    const std::string getCmdLine() const;
    const std::string getOriginalCmdLine() const;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};


/*_______________________JOBS_LIST_______________________________*/

class JobsList;

class JobsList {
  public:
  class JobEntry {
    public:
      typedef enum {FOREGROUND, BACKGROUND, STOPPED, DEAD}State;
      JobEntry(const Command* _cmd, State _state, time_t timer);
      ~JobEntry() = default;
      void changeState(State _state);

      /*_______________________GETTERS_______________________________*/
      const Command* getCommand();
      int getJobPid();
      State getState();
      time_t getTimer();
      std::string getCmdLine();

      /*_______________________SETTERS_______________________________*/
      void setTimer();
      
      private:
      const Command* cmd;
      State state; 
      time_t timer;
      //int pid;
      //std::string cmd_line;
    // to add - if the job needs to be deleted
    };
 
  private:
  std::map<int, JobEntry*> job_list;
  int num_of_jobs;
  
  public:
    JobsList(int _num_of_jobs = 0);
    ~JobsList() = default;
    void addJob(const Command* cmd, JobEntry::State state);
    void printJobsList();
    void printBeforeQuit();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry* getJobById(int jobId);
    JobEntry* getJobByPid(int pid);

    void removeJobById(int jobId);
    void removeJobByPid(int jobPid);
    JobEntry* getLastStoppedJob();
    int getMaxJobIdInList();
    // void deleteAllJobs();
    // void changeIsRunning();

};

/*_______________________SMALL_SHELL_______________________________*/

class SmallShell {
  private:
    std::string name;
    std::string current_directory;
    std::string last_directory;
    Command* current_command;
    JobsList job_list;
    int current_foreground_pid;
    bool is_running;
    SmallShell();

 public:    

    /*_______________________GETTERS_______________________________*/
    std::string getName();
    std::string getCurrentDirectory();
    std::string getLastDirectory();
    Command* getCurrentCommand();
    int getCurrentForegroundPid();
    JobsList::JobEntry* getMaxJob();
    JobsList::JobEntry* getMaxStoppedJob();

    /*_______________________SETTERS_______________________________*/
    void changeName(std::string new_name);
    void setCurrentDirectory(std::string directory);
    void setLastDirectory(std::string directory);
    void setCurrentCommand(Command* new_command);
    void changeCurrentForegroundPid(int new_pid);
    void changeStatusByPid(int pid, JobsList::JobEntry::State state);
    void addJob(const Command* command, JobsList::JobEntry::State state);
   
    
    /*_______________________METHODES_______________________________*/
    const int NO_FOREGROUND_PROCESS = 0;
    
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
      static SmallShell instance; // Guaranteed to be destroyed.
      // Instantiated on first use.
      return instance;
    }
    ~SmallShell();
    
    bool isRunning();
    void closeSmash();

    bool isRedirection(const std::vector<std::string>& args);
    bool isPipe(const std::vector<std::string>& args);
    JobsList::JobEntry* jobExists(int job_id);
    void printAllJobs();
    void printAllJobsBeforeKill();
    void killAllJobs();
    void removeJobByPid(int job_pid);
    Command *CreateCommand(const char* cmd_line);
    void executeCommand(const char* cmd_line);
};




class BuiltInCommand : public Command {
  public:
    BuiltInCommand(const char* cmd_line, int pid = -1);
    virtual ~BuiltInCommand() = default;
};

class ExternalCommand : public Command { 
  std::string prog {"/bin/bash"};
  std::string flag {"-c"};
  
  public:
    ExternalCommand(const char* cmd_line);
    virtual ~ExternalCommand() = default;
    void execute() override;
};

class PipeCommand : public Command {
  public:
    typedef enum {TO_STDOUT, TO_STERROR}PipeType;
    PipeCommand(const char* cmd_line);
    virtual ~PipeCommand() = default;
    void execute() override;
  
  private:
    Command* first_cmd;
    Command* second_cmd;
    PipeType type;
};

class RedirectionCommand : public Command {
  public:
    typedef enum {OVERRIDE, APPEND}RedirectionType;
    explicit RedirectionCommand(const char* cmd_line);
    virtual ~RedirectionCommand() = default;
    void execute() override;
  
  private:
    Command* first_cmd;
    RedirectionType type;
    std::string output_file;
  //void prepare() override;
  //void cleanup() override;
};


/*_______BUILT_IN_COMMANDS_______*/

class ChpromptCommand : public BuiltInCommand {
  std::string new_name;
  public:
    ChpromptCommand(const char* cmd_line, int pid);
    virtual ~ChpromptCommand() = default;
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
  public:
    ShowPidCommand(const char* cmd_line, int pid);
    virtual ~ShowPidCommand() = default;
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
  public:
    GetCurrDirCommand(const char* cmd_line, int pid);
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
    ChangeDirCommand(const char* cmd_line, int pid);
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

   class invalidJobId : public Exception {
  
     public:
      invalidJobId(std::string error_message):Exception(error_message){};
      ~invalidJobId() = default;
   };

   class InvalidArguments : public Exception {
  
     public:
      InvalidArguments();
      ~InvalidArguments() = default;
   };

   class FG_InvliadArgs : public Exception {
      public:
      FG_InvliadArgs();
   };
  
  //used for BG and FG
  class JobDoesntExist : public Exception {  
     public:
      JobDoesntExist(std::string error_message) : Exception(error_message) {}
      ~JobDoesntExist() = default;
  };

  class jobsListIsEmpty : public Exception {
  
     public:
      jobsListIsEmpty();
      ~jobsListIsEmpty() = default;
  };

  class BG_InvliadArgs : public Exception {
      public:
      BG_InvliadArgs();
   };
   
   class NoStoppedJobs : public Exception {
  
     public:
      NoStoppedJobs();
      ~NoStoppedJobs() = default;
  };

   class JobAlreadyRunning : public Exception {  
     public:
      JobAlreadyRunning(std::string error_message) : Exception(error_message) {}
      ~JobAlreadyRunning() = default;
  };

   class TAIL_InvliadArgs : public Exception {
      public:
      TAIL_InvliadArgs();
   };

/*___________EXTERNAL_COMMANDS_EXCEPTIONS_________*/

   



class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
    JobsCommand(const char* cmd_line, int pid = -1);
    virtual ~JobsCommand() = default;
    void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 int signal_number;
 int job_id;
 JobsList::JobEntry* job_entry;
 void checkValidArguments();
 bool isNumber(const std::string& str);
 public:
    KillCommand(const char* cmd_line);
    virtual ~KillCommand() = default;
    void execute() override;
    int getSignal();
    int getPid();
};

class QuitCommand : public BuiltInCommand {
// TODO: Add your data members 
public:
    QuitCommand(const char* cmd_line);
    virtual ~QuitCommand() = default;
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 int job_id;
 JobsList::JobEntry* job_entry;
 void checkValidArguments();
 public:
    ForegroundCommand(const char* cmd_line);
    virtual ~ForegroundCommand() = default;
    void execute() override;
    int getPid();
    Command* getJobsCommand();
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 int job_id;
 JobsList::JobEntry* job_entry;
 void checkValidArguments();
 public:
    BackgroundCommand(const char* cmd_line);
    virtual ~BackgroundCommand() = default;
    void execute() override;
    int getPid();
    Command* getJobsCommand();
};


class TailCommand : public BuiltInCommand {
  int num_of_lines;
  std::string filename;
  off_t findPosition(int fd);

 public:
    TailCommand(const char* cmd_line);
    virtual ~TailCommand() = default;
    void execute() override;
    void CheckValidArgs();
};

class TouchCommand : public BuiltInCommand {
 public:
    TouchCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}
    virtual ~TouchCommand() = default;
    void execute() override;
};


#endif //SMASH_COMMAND_H_
