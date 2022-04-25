#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <time.h>
#include <utime.h>


using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

const std::string WHITESPACE = " \n\r\t\f\v";

string _ltrim(const std::string& s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
      args[i] = (char*)malloc(s.length()+1);
      memset(args[i], 0, s.length()+1);
      strcpy(args[i], s.c_str());
      args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

const std::string _removeBackgroundSign(const char* cmd_line) {
    string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
      return str;
    }
    // if the command line does not end with & then return
    if (str[idx] != '&') {
      return str;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    str.erase(idx, 1);
    // // truncate the command line string up to the last non-space character
    // cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
    // const string tmp(cmd_line);
    return str; 
}

/*______________________________SMALL_SHELL_______________________________*/

SmallShell::SmallShell() : name("smash"), current_directory("not set"), last_directory("not set") {
// TODO: add your implementation
    
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

string SmallShell::getName() { 
    return this->name;
}

string SmallShell::getCurrentDirectory() {
     return this->current_directory;
}

string SmallShell::getLastDirectory() {
     return this->last_directory;
}


void SmallShell::changeName(string new_name) {
    this->name = new_name;
}

void SmallShell::setLastDirectory(string directory){
    this->last_directory = directory;
}
void SmallShell::setCurrentDirectory(string directory){
    this->current_directory = directory;
}


/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/

Command * SmallShell::CreateCommand(const char* cmd_line) {
    // For example:
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    
    if (firstWord.compare("chprompt") == 0) {
        return new ChpromptCommand(cmd_line);
    }
    else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    }
    else if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    }
    
    else if (firstWord.compare("cd") == 0) {
        try{
            SmallShell& smash = SmallShell::getInstance();
            return new ChangeDirCommand(cmd_line);
        }
        catch(Exception& e) {
            cout << e.what() << endl;
            return nullptr;
        }        
    }
    else {
      return new ExternalCommand(cmd_line);
    }
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    Command* cmd = CreateCommand(cmd_line);
    if(cmd == nullptr){
        return;
    }
    cmd->execute();
}
    // Please note that you must fork smash process for some commands (g, external commands....)



/*______________________________JOBS_ENTRY______________________________*/

JobsList::JobEntry::JobEntry(State _state) : state(_state) {}

void JobsList::JobEntry::changeState(State _state){
    this->state = _state;
}

/*______________________________JOBS_LIST_______________________________*/


JobsList::JobsList(){}

void addJob(Command* cmd, bool isStopped = false){
    //delet_all jobs()
}
void printJobsList();
void killAllJobs();
void removeFinishedJobs();
JobEntry * getJobById(int jobId);
void removeJobById(int jobId);
JobEntry * getLastJob(int* lastJobId);
JobEntry *getLastStoppedJob(int *jobId);
JobEntry * getMaxJobIdInList();



/*_____________________________COMMANDS_______________________________*/


Command::Command(const char* cmd_line) : cmd_line(_removeBackgroundSign(cmd_line)), 
                is_background_command(_isBackgroundComamnd(cmd_line)) {
    //
    string cmd_s = _trim(string(cmd_line));
    //string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    char* args[20];

    this->num_of_args = _parseCommandLine(this->cmd_line.c_str(), args);
    for(int i = 0; i < this->num_of_args; i++){
        this->args.push_back(args[i]);
    }
}

/*_________________BUILT_IN_COMMANDS________________*/

BuiltInCommand::BuiltInCommand(const char* cmd_line) : Command(cmd_line) {}

/*__________CHPROMPT_COMMAND___________*/

ChpromptCommand::ChpromptCommand(const char* cmd_line) : 
                BuiltInCommand(cmd_line) {
                    if(this->num_of_args == 1){
                        this->new_name = "smash";
                    }
                    else {
                        this->new_name = this->args[1];
                    }
                }



void ChpromptCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    smash.changeName(this->new_name);
}

/*__________SHOW_PID_COMMAND___________*/

ShowPidCommand::ShowPidCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}

void ShowPidCommand::execute() {
    cout << "smash pid is " << getpid() << endl;
}

/*___________PWD_COMMAND___________*/

GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}
   
void GetCurrDirCommand::execute() {
    char* temp_path =  getcwd(nullptr, 0);
    cout << temp_path << endl;
    free(temp_path);
}

/*___________CD_COMMAND___________*/
  
ChangeDirCommand::ChangeDirCommand(const char* cmd_line) :   
          BuiltInCommand(cmd_line) {
            if(this->num_of_args > this->MAX_NUM_OF_ARGS) {
                throw TooManyArgs();
            }
            if(this->num_of_args < this->MIN_NUM_OF_ARGS) {
                throw WhyDoYouMakeProblems();
            }
            //'-' was passed so change to last directory
            if(this->args[1].compare("-") == 0) {
                SmallShell& smash = SmallShell::getInstance();
                //check if there was a last directory or wasnt set yet
                if(smash.getLastDirectory().compare("not set") == 0) {
                    throw OldPwdNotSet();
                }
                else {
                    this->new_dir = smash.getLastDirectory();
                }
            }
            //full path was passed
            else {
                this->new_dir = this->args[1];
            }           
}

void ChangeDirCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    char* temp_path = getcwd(nullptr, 0);
    smash.setLastDirectory(temp_path);
    free(temp_path);

    DO_SYS(chdir(this->new_dir.c_str()));
    temp_path = getcwd(nullptr, 0);
    smash.setCurrentDirectory(temp_path);
    free(temp_path);
}


/*___________EXTERNAL_COMMANDS_________*/

static void chkStatus(int pid, int stat)
{
    if( WIFEXITED(stat) ) {
        printf("%d exit code=%d\n",pid, WEXITSTATUS(stat));
    }
    else if( WIFSIGNALED(stat) ) {
        printf("%d died on signal=%d\n",pid, WTERMSIG(stat));
    }
}

ExternalCommand::ExternalCommand(const char* cmd_line) : Command(cmd_line) {}

void ExternalCommand::execute() {
    // to check if last arg is "&" - to run in the backround (if - else)
    //fork then call bash with "-c" "ExternalCommand and parameters"
    int status;
    int pid = fork();
    if( pid < 0 ) {
        perror("fork failed");
    }
    else if( pid == 0 ) { // child
        char* const argv[4] = {(char*)this->prog, "-c", (char*)this->cmd_line.c_str(), nullptr };
        execv((char*)this->prog, argv);
        perror("execv failed");
    }
    else { // parent
        if(this->is_background_command == true){
            //add the job to job list
            return;
        }
        else{
            if( wait(&status) < 0 ) {
                perror("wait failed");
            }
            //if got error
            else {
                chkStatus(pid,status);
            }
        }

    }
}

/*_________________JOBS_COMMANDS________________*/

/*_________________JOBS_COMMAND________________*/

JobsCommand::JobsCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line){

}

void JobsCommand::execute() {

}

/*_________________KILL_COMMAND________________*/

KillCommand::KillCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line) {

}

void KillCommand::execute() {
    
}


ForegroundCommand::ForegroundCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line) {

}

void ForegroundCommand::execute() {
    
}

BackgroundCommand::BackgroundCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line){

}

void BackgroundCommand::execute() {
    
}


/*__________________COMMANDS_EXCEPTIONS___________________*/

Exception::Exception(std::string error_message) : error_message(error_message) {}
const char* Exception::what() const noexcept {
    return error_message.c_str();
}

/*___________BUILTIN_COMMANDS_EXCEPTIONS_________*/

OldPwdNotSet::OldPwdNotSet():Exception("smash error: cd: OLDPWD not set") {}
TooManyArgs::TooManyArgs():Exception("smash error: cd: too many arguments") {}
WhyDoYouMakeProblems::WhyDoYouMakeProblems():Exception("we will hunt you down!!") {}