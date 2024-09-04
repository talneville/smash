#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <iomanip>
#include "Commands.h"
#include <time.h>
#include <utime.h>
#include <assert.h>


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
                               \
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



/*_______________________SMALL_SHELL_______________________________*/

SmallShell::SmallShell() : name("smash"), current_directory("not set"), 
                            last_directory("not set"), current_command(nullptr),
                            is_running(true) {}

SmallShell::~SmallShell() {
// TODO: add your implementation == default??
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

Command* SmallShell::getCurrentCommand() {
    return this->current_command;
}

int SmallShell::getCurrentForegroundPid() {
    if((this->current_command == nullptr) || (this->current_command->isBackroundCommand())) {
        return 0;
    }
    return this->current_foreground_pid;
}

JobsList::JobEntry* SmallShell::getMaxJob(){
    int max_job_id = this->job_list.getMaxJobIdInList();
    return this->job_list.getJobById(max_job_id);
}

JobsList::JobEntry* SmallShell::getMaxStoppedJob(){
    return this->job_list.getLastStoppedJob();
}

void SmallShell::changeName(string new_name) {
    this->name = new_name;
}

void SmallShell::setCurrentDirectory(string directory){
    this->current_directory = directory;
}

void SmallShell::setLastDirectory(string directory){
    this->last_directory = directory;
}

void SmallShell::setCurrentCommand(Command* new_command){
    this->current_command = new_command;
}

void SmallShell::changeCurrentForegroundPid(int new_pid) { 
    this->current_foreground_pid = new_pid;
}

/* this function changes the state of a job by its PID, if the job is 
    not in the job list - this functions creates a jobEntry and adds it to the job list 
    USED IN CTRL Z HANDLER*/
void SmallShell::changeStatusByPid(int pid, JobsList::JobEntry::State state) {
    JobsList::JobEntry* entry = this->job_list.getJobByPid(pid);
    
    //process isn't in jobslist - add to jobslist
    // entry == nullptr if and only if the process is fg that was STOPPED before
    // - was not added to the list before
    if(entry == nullptr) {
        this->job_list.addJob(this->current_command, state);
        return;
    }
    //process is in joblist
    //cout << "no way we are here" << endl;
    entry->changeState(state);
}

void SmallShell::addJob(const Command* command, JobsList::JobEntry::State state) {
    this->job_list.addJob(command, state);
}

bool SmallShell::isRunning() { 
    return this->is_running;
}

void SmallShell::closeSmash() {
    this->is_running = false;
}

bool SmallShell::isRedirection(const std::vector<std::string>& args) {
    std::vector<std::string>::const_iterator it;
    for(it = args.begin(); it != args.end(); it++) {
        if(((*it).compare(">") == 0) || ((*it).compare(">>") == 0)) {
                return true;
            }
    }    
    return false;
}

bool SmallShell::isPipe(const std::vector<std::string>& args) {
    std::vector<std::string>::const_iterator it;
    for(it = args.begin(); it != args.end(); it++) {
        if(((*it).compare("|") == 0) || ((*it).compare("|&") == 0)) {
                return true;
            }
    }    
    return false;
}

JobsList::JobEntry* SmallShell::jobExists(int job_id){
    return this->job_list.getJobById(job_id);
}

void SmallShell::printAllJobs() {
    this->job_list.printJobsList();
}

void SmallShell::printAllJobsBeforeKill(){
    this->job_list.printBeforeQuit();
}

void SmallShell::killAllJobs() {
    this->job_list.killAllJobs();
}

void SmallShell::removeJobByPid(int job_pid){
    this->job_list.removeJobByPid(job_pid);
    return;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/

Command * SmallShell::CreateCommand(const char* cmd_line) {
    // For example:
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    int smash_pid = getpid();
    //parsing the cmd_line
    char* args[20];
    std::vector<std::string> arg_vec;
    int num_of_args = _parseCommandLine(cmd_line, args);
    for(int i = 0; i < num_of_args; i++){
        arg_vec.push_back(args[i]);
    }

    if(cmd_s.empty()) {
        return nullptr;
    }
    if(isRedirection(arg_vec) == true) {
        try{
            return new RedirectionCommand(cmd_line);  
        }
        catch(Exception& e) {
            //the exception was already printed by the first create call
            return nullptr;
        }   
    }
    if(isPipe(arg_vec) == true) {
        try{
            return new PipeCommand(cmd_line);   
        }
        catch(Exception& e) {
            //the exception was already printed by the first create call
            return nullptr;
        }   
    }
    if (firstWord.compare("chprompt") == 0) {
        return new ChpromptCommand(cmd_line, smash_pid);
    }
    else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line, smash_pid);
    }
    else if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line, smash_pid);
    }
    else if (firstWord.compare("jobs") == 0) {
        return new JobsCommand(cmd_line, smash_pid);
    }
    else if (firstWord.compare("quit") == 0) {
        return new QuitCommand(cmd_line);
    }
    else if (firstWord.compare("kill") == 0) {
        try{
            return new KillCommand(cmd_line);
        }
        catch(Exception& e) {
            cerr << e.what() << endl;
            return nullptr;
        }  
    }
    else if (firstWord.compare("fg") == 0) {
        try{
            return new ForegroundCommand(cmd_line);
        }
        catch(Exception& e) {
            cerr << e.what() << endl;
            return nullptr;
        }  
    }
    else if (firstWord.compare("bg") == 0) {
        try{
            return new BackgroundCommand(cmd_line);
        }
        catch(Exception& e) {
            cerr << e.what() << endl;
            return nullptr;
        }  
    }
    else if (firstWord.compare("cd") == 0) {
        try{
            return new ChangeDirCommand(cmd_line, smash_pid);
        }
        catch(Exception& e) {
            cerr << e.what() << endl;
            return nullptr;
        }        
    }
    else if (firstWord.compare("tail") == 0) {
        try{
            return new TailCommand(cmd_line);
        }
        catch(Exception& e) {
            cerr << e.what() << endl;
            return nullptr;
        }  
    }
    else if (firstWord.compare("touch") == 0) {
        try{
            return new TouchCommand(cmd_line);
        }
        catch(Exception& e) {
            cerr << e.what() << endl;
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
    this->job_list.removeFinishedJobs();
    this->current_command = CreateCommand(cmd_line);
    if(this->current_command == nullptr){
        return;
    }
    this->current_command->execute();
    this->changeCurrentForegroundPid(0);
    // when to delete --> no good now maby in the bg
    this->current_command = nullptr;
    this->job_list.removeFinishedJobs();
}
    // Please note that you must fork smash process for some commands (g, external commands....)



/*_______________________JOBS_LIST_______________________________*/
/*_________JOB_ENTRY________*/

JobsList::JobEntry::JobEntry(const Command* _cmd, State _state, time_t timer) : 
                cmd(_cmd), state(_state), timer(timer) {}

void JobsList::JobEntry::changeState(State _state){
    this->state = _state;
}

/*_________GETTERS___________*/

const Command* JobsList::JobEntry::getCommand(){
    return this->cmd;
}
int JobsList::JobEntry::getJobPid() {
    return this->cmd->getPid();
}

JobsList::JobEntry::State JobsList::JobEntry::getState(){
    return this->state;
}

time_t JobsList::JobEntry::getTimer(){
    return this->timer;
}

std::string JobsList::JobEntry::getCmdLine(){
    return this->cmd->getOriginalCmdLine();
}

/*_________SETTERS___________*/

void JobsList::JobEntry::setTimer(){
    time_t p_time;
    int res = time(&p_time);
    if(res < 0 ){
        perror("smash error: time failed");
        return;
    }
    this->timer = p_time;
    return;
}



/*______________________________JOBS_LIST_______________________________*/

JobsList::JobsList(int _num_of_jobs) {}

void JobsList::addJob(const Command* cmd, JobEntry::State state) {
    if(cmd == nullptr) {
        return;
    }
    this->removeFinishedJobs();
    time_t p_time;
    int res = time(&p_time);
    if(res < 0 ) {
        perror("smash error: time failed");
        return;
    }
    JobsList::JobEntry* new_entry = new JobEntry(cmd, state, p_time);
    this->job_list.emplace(this->getMaxJobIdInList() + 1, new_entry);
    this->num_of_jobs++;      
}

void JobsList::printJobsList() {
    this->removeFinishedJobs();
    std::map<int, JobEntry*>::iterator it;
    for (it = this->job_list.begin(); it != this->job_list.end(); it++) {
        //format: [<job-id>] <command> : <process id> <seconds elapsed> (stopped)
        time_t p_time;
        int res = time(&p_time);
        if(res < 0) {
            perror("smash error: time failed");
            return;
        }
        cout << "[" << it->first << "] " << it->second->getCmdLine()  << " : " 
                    << it->second->getJobPid() << " " << difftime(p_time, it->second->getTimer()) << " secs";
        if(it->second->getState() == JobsList::JobEntry::STOPPED) {
            cout << " (stopped)";
        } 
        cout << endl;
    }
}

void JobsList::printBeforeQuit(){
    this->removeFinishedJobs();
    cout << "smash: sending SIGKILL signal to " << this->num_of_jobs << " jobs:" << endl;
    std::map<int, JobEntry*>::iterator it;
    for (it = this->job_list.begin(); it != this->job_list.end(); it++) {
        //smash: sending SIGKILL signal to 3 jobs:
        //format: [pid] : <command>
        cout << it->second->getJobPid() << ": " << it->second->getCmdLine() << endl;
    }
}

void JobsList::killAllJobs(){
    std::map<int, JobEntry*>::iterator it = this->job_list.begin();
    
    while (it != this->job_list.end()) {
       removeJobById(it->first);
       it = this->job_list.begin();
    }
}

void JobsList::removeFinishedJobs(){
    std::map<int, JobEntry*>::iterator it = this->job_list.begin();    
    
    while(it != this->job_list.end()) {
        //if waitpid returned with the pid of the process then it was in zombie state and has finished
        
        //int status;
        pid_t return_pid = waitpid(it->second->getJobPid(), nullptr, WNOHANG); /* WNOHANG def'd in wait.h */
        
        if (return_pid == 0) {
            it++;
        /* child is still running */
        } 
        else if (return_pid == it->second->getJobPid() || return_pid == -1) {
        /* child is finished. exit status in status */
            delete(it->second);
            // remove from map
            it = this->job_list.erase(it--); 
            this->num_of_jobs--;              
        } 
        else{
        //   someone else waited for him!
            //cout << "we are in remove finished jobs" << endl;
            /* error shouldnt reach this!
            if(it->second != nullptr){
                 delete(it->second);
                 // remove from map
                 it = this->job_list.erase(it--); 
                 this->num_of_jobs--;  
            }*/
            it++;
        }  
    }
}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    std::map<int, JobEntry*>::iterator it;
    it = this->job_list.find(jobId);
    if(it == this->job_list.end()) {
        // throw "STOP MESSING AROUND!"
        // if excist , if not --> exception
        return nullptr;
    }
    return this->job_list[jobId];
}

JobsList::JobEntry* JobsList::getJobByPid(int pid) {
    std::map<int, JobEntry*>::iterator it;
    for (it = this->job_list.begin(); it != this->job_list.end(); it++) {
        if(it->second->getJobPid() == pid) {
            return it->second;
        }
    }
    return nullptr;
}

//note: fg processes taht were added to joblist wont get removed if they go back to foreground
void JobsList::removeJobById(int jobId) {
    std::map<int, JobEntry*>::iterator it = this->job_list.begin();
    
    while (it != this->job_list.end()) {
        if(it->first == jobId) {
            int status;
            pid_t return_pid = waitpid(it->second->getJobPid(), &status, WNOHANG); /* WNOHANG def'd in wait.h */
            if (return_pid == 0) {
                /* child is still running */
                kill(it->second->getJobPid(), SIGKILL);
                pid_t return_pid_after_kill = waitpid(it->second->getJobPid(),nullptr,WUNTRACED); /* WNOHANG def'd in wait.h */
                if (return_pid_after_kill == it->second->getJobPid()) {
                    delete(it->second);
                    // remove from map
                    it = this->job_list.erase(it--); 
                    this->num_of_jobs--;
                }
                else{
                /* error */
                }              
            } 
            else if (return_pid == it->second->getJobPid()) {
                /* child is finished. exit status in   status */
                delete(it->second);
                // remove from map
                it = this->job_list.erase(it--); 
                this->num_of_jobs--;              
            } 
            else{
            /* error */
            }             
        }
        else {
            it++;
        }
    }
}

void JobsList::removeJobByPid(int jobPid){
    std::map<int, JobEntry*>::iterator it = this->job_list.begin();
    removeFinishedJobs();
    while (it != this->job_list.end()) {
        cout << "im in for loop" << endl;
        if(it->second->getJobPid() == jobPid) {
            //if waitpid returned 0 then it hasnt finished and need to send kil
            int status;
            pid_t return_pid = waitpid(it->second->getJobPid(), &status, WNOHANG); /* WNOHANG def'd in wait.h */
            if (return_pid == 0) {
                /* child is still running */
                kill(it->second->getJobPid(), SIGKILL);
                //pid_t return_pid_after_kill = waitpid(it->second->getJobPid(),nullptr,WUNTRACED); /* WNOHANG def'd in wait.h */
                delete(it->second);
                // remove from map
                it = this->job_list.erase(it--); 
                this->num_of_jobs--;
            } 
            else
            {
                /* child is finished. exit status in   status */
                delete(it->second);
                // remove from map
                it = this->job_list.erase(it--); 
                this->num_of_jobs--;              
            }
        }
        else{
            it++;
        }
    }
}

JobsList::JobEntry* JobsList::getLastStoppedJob(){
    std::map<int, JobEntry*>::reverse_iterator it;
    //iterate from end to beginning so we get max stopped job
    for (it = this->job_list.rbegin(); it != this->job_list.rend(); it++) {
        if(it->second->getState() == JobsList::JobEntry::STOPPED) {
            return it->second;
        }
    }
    return nullptr;
}

int JobsList::getMaxJobIdInList(){
   // cout << "get Max job" << endl;
    int max = 0;
    std::map<int, JobEntry*>::iterator it;
    for (it = this->job_list.begin(); it != this->job_list.end(); it++) {
        if(it->first > max) {
            max = it->first;
        }
    }
    return max;
}


/*_____________________________COMMANDS_______________________________*/

Command::Command(const char* cmd_line, int pid) :  original_cmd_line(cmd_line),
        cmd_line(_removeBackgroundSign(cmd_line)), is_background_command(_isBackgroundComamnd(cmd_line)) {
    string cmd_s = _trim(string(cmd_line));
    //string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    char* args[20];

    this->num_of_args = _parseCommandLine(this->cmd_line.c_str(), args);
    for(int i = 0; i < this->num_of_args; i++){
        this->args.push_back(args[i]);
    }
}

bool Command::isBackroundCommand() const {
    return this->is_background_command;
}

bool Command::isForegroundCommand() const {
    return !this->is_background_command;
}

void Command::changePid(int pid){
    this->pid = pid;
}

void Command::changeStateToFg(bool fg){
    if(fg == true){
        this->is_background_command = false;
    }
    else{
        this->is_background_command = true;
    }
}

int Command::getPid() const {
    return this->pid;
}

const std::vector<std::string> Command::getArgs() const {
    return this->args;
}

const std::string Command::getCmdLine() const {
    return this->cmd_line;
}

const std::string Command::getOriginalCmdLine() const {
    return this->original_cmd_line;
}

/*_________________BUILT_IN_COMMANDS________________*/

BuiltInCommand::BuiltInCommand(const char* cmd_line, int pid) : 
                Command(cmd_line, pid) {}

/*__________CHPROMPT_COMMAND___________*/

ChpromptCommand::ChpromptCommand(const char* cmd_line, int pid) : 
                BuiltInCommand(cmd_line, pid) {
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

ShowPidCommand::ShowPidCommand(const char* cmd_line, int pid) : BuiltInCommand(cmd_line, pid) {}

void ShowPidCommand::execute() {
    cout << "smash pid is " << getpid() << endl;
}

/*___________PWD_COMMAND___________*/

GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line, int pid) : BuiltInCommand(cmd_line, pid) {}
   
void GetCurrDirCommand::execute() {
    char* temp_path =  getcwd(nullptr, 0);
    if(temp_path == nullptr ){
        perror("smash error: getcwd failed");
        return;
    }
    cout << temp_path << endl;
    free(temp_path);
}

/*___________CD_COMMAND___________*/
  
ChangeDirCommand::ChangeDirCommand(const char* cmd_line, int pid) :   
          BuiltInCommand(cmd_line, pid) {
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
    if(temp_path == nullptr) {
        perror("smash error: getcwd failed");
        return;
    }
    if(chdir(this->new_dir.c_str()) < 0) {
        perror("smash error: chdir failed");
        free(temp_path);
        return;
    }
    //only if syscall worked we sholud change last directory
    smash.setLastDirectory(temp_path);
    free(temp_path);

    temp_path = getcwd(nullptr, 0);
    if(temp_path == nullptr) {
        perror("smash error: getcwd failed");
        return;
    }
    smash.setCurrentDirectory(temp_path);
    free(temp_path);
}

void TailCommand::CheckValidArgs(){
    //invalid num of arg
    if(this->num_of_args > 3 || this->num_of_args == 1) {
        throw TAIL_InvliadArgs();
    }
    //check if num of lines is a number
    if(this->num_of_args == 3){ 
        if(this->args[1].substr(0,1).compare("-") != 0){
            throw TAIL_InvliadArgs();
        }
        try{
            string num = this->args[1].substr(1, this->args[1].size() - 1);
            this->num_of_lines = stoi(num);
            this->filename = this->args[2];
        }
        catch(const std::invalid_argument& e){
            throw TAIL_InvliadArgs();
        }
    }
    if(this->num_of_args == 2) {
            this->num_of_lines = 10;
            this->filename = this->args[1];
    }
}

TailCommand::TailCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {
    this->CheckValidArgs();
}

off_t TailCommand::findPosition(int fd) {    
    char buffer;
    int line = 0;
    int filesize = lseek(fd,(off_t) 0, SEEK_END); //filesize is lastby +offset
    if(filesize < 0){
        perror("smash error: lseek failed");
        return 0;
    }
    int i = filesize - 1;
    for (; i >= 0; i--) { //read byte by byte from end
        lseek(fd, (off_t) i, SEEK_SET);
        int read_res = read(fd, &buffer, 1);
        if(read_res < 0){
            perror("smash error: read failed");
            return 0;
        }
        if(read_res == 1 && line < this->num_of_lines) {
            /* end of line or  */
            if (buffer == '\n' && i < filesize - 1) {    
                line++;           
            }
            if(line == this->num_of_lines){
                break;
            }
        }
        //if read got to end of file
        else{
            break;
        }
    }
    return i + 1;
}

void TailCommand::execute() {
    mode_t given_mode = 0655;
    int fd = open(this->filename.c_str(), O_RDONLY, given_mode);
    if(fd == -1){
        perror("smash error: open failed");
        return;
    }
    
    char buffer;
    string temp;
    vector<string> vec;
    off_t offset = findPosition(fd);
    int lseek_res = lseek(fd, offset, SEEK_SET);
    if(lseek_res < 0){
            perror("smash error: lseek failed");
            return;
    }
    //read into string then at end of line add to vector
    while(read(fd, &buffer, 1) == 1) {
        write(1, &buffer, 1);
    }

}

vector<string> splitString(string string_to_split, char splitter) {
    int first = 0;
    int second = 0;
    vector<string> vec;
    for(unsigned int i = 0; i < string_to_split.size(); i++) {
        if(string_to_split[i] == splitter) {
            second = i;
            vec.push_back(string_to_split.substr(first, second - first));
            first = second + 1;
        }
        if(i + 1 == string_to_split.size()) {
            second = i;
            vec.push_back(string_to_split.substr(first, second - first + 1));
        }
    }
    return vec;
} 

void TouchCommand::execute(){
    if(this->num_of_args != 3){
        cerr << "smash error: touch: invalid arguments" << endl;
        return;
    }
    struct tm time;
    char splitter = ':';
    vector<string> time_info = splitString(this->args[2], splitter);
    //fill time struct
    time.tm_sec = stoi(time_info[0]);
    time.tm_min = stoi(time_info[1]);
    time.tm_hour = stoi(time_info[2]);
    time.tm_mday = stoi(time_info[3]);
    time.tm_mon = stoi(time_info[4]) - 1;
    time.tm_year = stoi(time_info[5]) - 1900;
    time.tm_isdst = -1;

    struct utimbuf utime_par;
    utime_par.actime  = mktime(&time);
    utime_par.modtime = mktime(&time);
    //pass to utime to change the timestamp
    if(utime(this->args[1].c_str(),&utime_par) != 0)
    {
      perror("smash error: utime failed");
      return;
    }
}

/*___________EXTERNAL_COMMANDS_________*/

ExternalCommand::ExternalCommand(const char* cmd_line) : Command(cmd_line) {}


void ExternalCommand::execute() {
    // to check if last arg is "&" - to run in the backround (if - else)
    //fork then call bash with "-c" "ExternalCommand and parameters"
    SmallShell& smash = SmallShell::getInstance();
    int pid = fork();
    
    if( pid < 0 ) {
        perror("fork failed");
        return;
    }


    /* child */
    if( pid == 0 ) { 
        setpgrp();
        int child_pid = getpid();
        smash.getCurrentCommand()->changePid(child_pid);
        char* const argv[4] = {(char* const)this->prog.c_str(), (char* const)this->flag.c_str(), (char*)this->cmd_line.c_str(), nullptr };
        int ret_execv = execv(this->prog.c_str(), argv);
        if(ret_execv < 0 ){
            perror("smash error: execv failed");
            return;
        }
    }

    /* parent */
    else { 
        smash.getCurrentCommand()->changePid(pid);
        //foreground
        if(smash.getCurrentCommand()->isForegroundCommand() == true) {
            waitpid(pid, nullptr, WUNTRACED);
            return;
        }
        //background
        else {
            smash.addJob(smash.getCurrentCommand(), JobsList::JobEntry::BACKGROUND);
            return;
        }
    }
}

/*_________________JOBS_COMMANDS________________*/

/*_________________JOBS_COMMAND________________*/

JobsCommand::JobsCommand(const char* cmd_line, int pid) : BuiltInCommand(cmd_line, pid) {}

void JobsCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    smash.printAllJobs();    
}

/*_________________KILL_COMMAND________________*/

void KillCommand::checkValidArguments(){
    if(this->num_of_args != 3){
        throw InvalidArguments();
    }
    //check if starts with '-'
    if(this->args[1][0] != '-'){
        throw InvalidArguments();
    }
    //check if is a number
    
    string num = this->args[1].substr(1, this->args[1].size() - 1);
    
    if(this->isNumber(num) == false) {
        throw InvalidArguments();
    }
    try {
    //check if valid signal
        int signal_num = stoi(num);
        if(signal_num < 1 || signal_num > 31){
            throw InvalidArguments();
        }
    }
    catch(const std::invalid_argument& e){
        throw InvalidArguments();
    }

    string num2 = this->args[2];
    
    try {
        //check if valid job id number
        stoi(num2);
    }
    catch(const std::invalid_argument& e){
        throw InvalidArguments();
    }
}

bool KillCommand::isNumber(const string& str){
    for (char const &c : str) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
}

KillCommand::KillCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {
        this->checkValidArguments();
        //check arguments
        //TODO: ADD THROW INVALID ARGUMENTS
        this->signal_number = stoi(this->args[1].erase(0,1));
        this->job_id = stoi(this->args[2]);
        //check job id
        SmallShell& smash = SmallShell::getInstance();
        this->job_entry = smash.jobExists(this->job_id);
        if(job_entry == nullptr){
            //throw smash error: kill: job-id <job-id> does not exist
            throw invalidJobId("smash error: kill: job-id " + this->args[2] + " does not exist");
        }
}

void KillCommand::execute() {  
    //if signal_number = 2 or 20 then send other stop and kill - so we dont use the handlers on children
    if(this->getSignal() == SIGINT){
        kill(this->getPid(),SIGKILL);
    }
    else if(this->getSignal()  == SIGTSTP){
        kill(this->getPid(),SIGSTOP);
    }
    else{
        kill(this->getPid(),this->getSignal());
    }
    cout << "signal number " << this->getSignal() << " was sent to pid " << this->getPid() << endl;
}

int KillCommand::getPid(){
    return this->job_entry->getJobPid();
}
    
int KillCommand::getSignal(){
    return this->signal_number;
}

/*_________________QUIT_COMMAND________________*/

QuitCommand::QuitCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}

void QuitCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    if(this->num_of_args > 1 && this->args[1].compare("kill") == 0){
        smash.printAllJobsBeforeKill();   
        smash.killAllJobs();
    }
    smash.closeSmash();
}

/*_________________FG_COMMAND________________*/

void ForegroundCommand::checkValidArguments() {
    SmallShell& smash = SmallShell::getInstance();
    //invalid num of arg
    if(this->num_of_args > 2) {
        throw FG_InvliadArgs();
    }
    if(this->num_of_args == 2){ 
        try{
            int job_id = stoi(this->args[1]);
            if(smash.jobExists(job_id) == nullptr){
                throw JobDoesntExist("smash error: fg: job-id " + this->args[1] + " does not exist");
            }
        }
        catch(const std::invalid_argument& e){
            throw FG_InvliadArgs();
        }
    }
    if(this->num_of_args == 1 && smash.getMaxJob() == 0) {
        throw jobsListIsEmpty();
    }
}

ForegroundCommand::ForegroundCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {
    SmallShell& smash = SmallShell::getInstance();
    this->checkValidArguments();
    if(this->num_of_args == 2) {  
        this->job_id = stoi(this->args[1]);
        this->job_entry = smash.jobExists(job_id);
    }
    else{
        this->job_entry = smash.getMaxJob();
    }

}

void ForegroundCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    Command * fg_cmd = this->getJobsCommand();
    int fg_pid = fg_cmd->getPid();
    //print
    cout << fg_cmd->getOriginalCmdLine() << " : " << fg_pid << endl;
    fg_cmd->changeStateToFg(true);
    smash.setCurrentCommand(this->getJobsCommand());
    smash.changeCurrentForegroundPid(fg_pid);
    //continue job
    kill(fg_pid,SIGCONT);
    //bring to foreground
    // change current command to foreground
    int status;
    waitpid(fg_pid, &status, WUNTRACED);
    //check the status if got a sigstop then reset timer but dont remove from the jobslist 

    if(WIFSTOPPED(status) == true){
       //cout << "is it STOPPED OR KIILLED OR BOTH??" << endl;
       this->job_entry->setTimer();
       
    }
    //if()
    else{
        //need to remove 'manually' because if we use removeFinishedJobs then the waitpid there will return -1 (error)
        //smash.removeJobByPid(fg_pid);
    }
    return;
}

int ForegroundCommand::getPid() {
    if(this->job_entry == nullptr){
        return 0;
    }
    else{
        return this->job_entry->getJobPid();
    }
}

Command* ForegroundCommand::getJobsCommand(){
    if(this->job_entry == nullptr){
        return nullptr;
    }
    else{
        return (Command*)this->job_entry->getCommand();
    }
}

/*_________________BG__COMMAND________________*/


void BackgroundCommand::checkValidArguments() {
    SmallShell& smash = SmallShell::getInstance();
    //invalid num of arg
    if(this->num_of_args > 2) {
        throw BG_InvliadArgs();
    }
    //job id errors
    if(this->num_of_args == 2){ 
        try{
            int job_id = stoi(this->args[1]);
            JobsList::JobEntry* check_job =  smash.jobExists(job_id);
            if(check_job == nullptr){
                throw JobDoesntExist("smash error: bg: job-id " + this->args[1] + " does not exist");
            }
            else if(check_job->getState() != JobsList::JobEntry::STOPPED){
                throw JobAlreadyRunning("smash error: bg: job-id " + this->args[1] + " is already running in the background");
            }
            else{

            }
        }
        catch(const std::invalid_argument& e){
            throw BG_InvliadArgs();
        }
    }
    if(this->num_of_args == 1 && smash.getMaxStoppedJob() == nullptr) {
        throw NoStoppedJobs();
    }
}

BackgroundCommand::BackgroundCommand(const char* cmd_line) : BuiltInCommand(cmd_line){
    SmallShell& smash = SmallShell::getInstance();
    this->checkValidArguments();
    if(this->num_of_args == 2) {  
        this->job_id = stoi(this->args[1]);
        this->job_entry = smash.jobExists(job_id);
    }
    else{
        this->job_entry = smash.getMaxStoppedJob();
    }
}

void BackgroundCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    Command * bg_cmd = this->getJobsCommand();
    int bg_pid = bg_cmd->getPid();
    //print
    cout << bg_cmd->getOriginalCmdLine() << " : " << bg_pid << endl;
    //continue job
    kill(bg_pid,SIGCONT);
    //kee running in background - dont wait!
    //change status in jobslist from STOPPED to BACKGROUND!!
    smash.changeStatusByPid(bg_pid,JobsList::JobEntry::BACKGROUND);
    return;
}

int BackgroundCommand::getPid() {
    if(this->job_entry == nullptr){
        return 0;
    }
    else{
        return this->job_entry->getJobPid();
    }
}

Command* BackgroundCommand::getJobsCommand(){
    if(this->job_entry == nullptr){
        return nullptr;
    }
    else{
        return (Command*)this->job_entry->getCommand();
    }
}

/*_________________SPECIAL_COMMANDS________________*/

/*_________________REDIRECTION_COMMANDS________________*/

RedirectionCommand::RedirectionCommand(const char* cmd_line) : Command(cmd_line) {
    SmallShell& smash = SmallShell::getInstance();
    //get output filename
    if(this->args[this->num_of_args - 1].compare("&") == 0){
        this->output_file = this->args[this->num_of_args - 2];
    }
    else{
        this->output_file = this->args[this->num_of_args - 1];
    }
    
    //create first command
    string temp;
    temp.clear();
    for(vector<string>::iterator it = this->args.begin(); it != this->args.end(); it++) {
        if(it->compare(">") == 0) {
            this->type = OVERRIDE;
            break;
        }
        if(it->compare(">>") == 0) {
            this->type = APPEND;
            break;
        }
        temp += *(it);
        temp += " ";
    }
    
    //const char* first_cmd_line;
    this->first_cmd = smash.CreateCommand(temp.c_str());
    if(this->first_cmd == nullptr){
        //if had invalid args than still create the file
        int fd = open(this->output_file.c_str(),O_RDWR | O_CREAT, 0655);
        if(fd < 0 ){
            perror("smash error: open failed");
        }
        throw InvalidArguments();
    }
}

void RedirectionCommand::execute(){
    int stdout_copy = dup(1);
    if(stdout_copy < 0){
            perror("smash error: dup failed");
    }
    close(1);
    int fd = -1;

    mode_t given_mode=0655;
    if(this->type == RedirectionCommand::OVERRIDE){
        fd = open(this->output_file.c_str(),O_RDWR | O_CREAT | O_TRUNC, given_mode);
    }
    else{
       fd = open(this->output_file.c_str(),O_RDWR | O_CREAT | O_APPEND, given_mode); 
    }
    if (fd < 0) 
    { 
        perror("smash error: open failed"); 
        close(1);
        close(fd);
        dup2(stdout_copy, 1);
        close(stdout_copy);
        return;
    } 
    this->first_cmd->execute();
    close(1);
    close(fd);
    dup2(stdout_copy, 1);
    close(stdout_copy);
  }

/*__________________PIPE_COMMANDS___________________*/

  PipeCommand::PipeCommand(const char* cmd_line): Command(cmd_line){
    SmallShell& smash = SmallShell::getInstance();
    
    //create first command
    string temp1;
    temp1.clear();
    vector<string>::iterator it = this->args.begin();
    for(; it != this->args.end(); it++) {
        if(it->compare("|") == 0) {
            this->type = TO_STDOUT;
            break;
        }
        if(it->compare("|&") == 0) {
            this->type = TO_STERROR;
            break;
        }
        temp1 += *(it);
        temp1 += " ";
    }
    
    //const char* first_cmd_line;
    this->first_cmd = smash.CreateCommand(temp1.c_str());
    if(this->first_cmd == nullptr){
        throw InvalidArguments();
    }

    //create second command
    string temp2;
    temp2.clear();
    it++;
    for(; it != this->args.end(); it++) {
        temp2 += *(it);
        temp2 += " ";
    }
    
    //const char* first_cmd_line;
    this->second_cmd = smash.CreateCommand(temp2.c_str());
        if(this->second_cmd == nullptr){
        throw InvalidArguments();
    }

  }

  void PipeCommand::execute() {
    //make pipe
    int fd[2];
    int pipe_res = pipe(fd);
    if(pipe_res < 0 ) {
        perror("smash error: pipe failed");
        return;
    }
    
    int pid1 = fork();
    if (pid1 == 0) {
    // first child
        if(this->type == TO_STDOUT){
            int dup_res = dup2(fd[1],1);
            if(dup_res < 0 ){
                perror("smash error: pipe failed");
                return;
            }
        }

        else{
            int dup_res = dup2(fd[1],2);
            if(dup_res < 0 ){
                perror("smash error: pipe failed");
                return;
            }
        }
        int close_res = close(fd[0]);
        if(close_res < 0 ){
            perror("smash error: pipe failed");
            return;
        }
        close_res = close(fd[1]);
        if(close_res < 0 ){
            perror("smash error: pipe failed");
            return;
        }
        this->first_cmd->execute();
        exit(0);
    }

    int pid2 = fork();
    if(pid2 == 0) {
    // second child
        dup2(fd[0],0);
        close(fd[0]);
        close(fd[1]);
        this->second_cmd->execute();
        exit(0);   
    }
    int close_res = close(fd[0]);
    if(close_res < 0 ){
            perror("smash error: pipe failed");
            return;
        }
    close_res = close(fd[1]);
    if(close_res < 0 ){
            perror("smash error: pipe failed");
            return;
        }
    //wait for all children
    if((waitpid(pid1, nullptr, WUNTRACED) != -1) && (waitpid(pid2, nullptr, WUNTRACED) != -1)) {
        return;
    }
    else {
        perror("smash error: waitpid failed");
        return;
    }
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
InvalidArguments::InvalidArguments():Exception("smash error: kill: invalid arguments") {}
FG_InvliadArgs::FG_InvliadArgs():Exception("smash error: fg: invalid arguments") {}
jobsListIsEmpty::jobsListIsEmpty():Exception("smash error: fg: jobs list is empty") {}
NoStoppedJobs::NoStoppedJobs():Exception("smash error: bg: there is no stopped jobs to resume") {}
BG_InvliadArgs::BG_InvliadArgs():Exception("smash error: bg: invalid arguments") {}
TAIL_InvliadArgs::TAIL_InvliadArgs():Exception("smash error: tail: invalid arguments") {}
