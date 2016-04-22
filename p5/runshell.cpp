#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <set>

#include "parser.hpp"
#include "runshell.hpp"

using std::string;
using std::cin;
using std::cout;
using std::cerr;
using std::vector;
using std::getline;
using std::endl;
using std::flush;
using std::copy;
using std::max;
using std::set;

set<int> cur_childs;

Call::Call(const vector<Lexem> &lexems, size_t &idx, int readfd, int writefd) {
    readfd_ = readfd;
    writefd_ = writefd;
    size_t j = idx + 1;
    while (j < lexems.size() && lexems[j].type == "argument") {
        ++j;
    }
    argv_.resize(j - idx);
    for (size_t k = idx; k < j; ++k) {
        argv_[k - idx] = const_cast<char*>(lexems[k].lexem.data()); //filename and args
    }
    idx = j;
    
    if (idx < lexems.size() && lexems[idx].type == "subprocess") {
        ++idx;
    }

    if (idx < lexems.size() && lexems[idx].type == "input") {
        ++idx;
        int fd = open(lexems[idx].lexem.c_str(), O_RDONLY, 0666);
        ++idx;
        readfd_ = fd;
    }

    if (idx < lexems.size() && lexems[idx].type == "output") {
        ++idx;
        int fd = open(lexems[idx].lexem.c_str(),  O_CREAT | O_TRUNC | O_APPEND | O_WRONLY, 0666);
        ++idx;
        writefd_ = fd;
    }
    
    if (idx < lexems.size() && lexems[idx].type == "input") {
        ++idx;
        int fd = open(lexems[idx].lexem.c_str(), O_RDONLY, 0666);
        ++idx;
        readfd_ = fd;
    }
}

void Call::setWriteFd(int fd) { 
    writefd_ = fd;
}

void Call::setReadFd(int fd) {
    readfd_ = fd;
}

void Call::closeFd() {
    if (readfd_ != 0) {
        close(readfd_);
    }
    if (writefd_ != 1) {
        close(writefd_);
    }
}

int Call::exec() {
    if (readfd_ != 0) {
        dup2(readfd_, 0);
        close(readfd_);
    }
    if (writefd_ != 1) {
        dup2(writefd_, 1);
        close(writefd_);
    }
    char *c_argv[argv_.size() + 1];
    
    for (size_t i = 0; i < argv_.size(); ++i) {
        c_argv[i] = argv_[i];
    }
    c_argv[argv_.size()] = nullptr;
    //return execvp(c_argv[0], c_argv);
    if (execvp(c_argv[0], c_argv) < 0) {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}


ExitStat::ExitStat(int stat): exit_status(stat) {}

bool ExitStat::success() const {
    return WIFEXITED(exit_status) && WEXITSTATUS(exit_status) == 0;
}

Shell::Shell() {
    readfd_ = 0;
    prev_readfd_ = 0;
    writefd_ = 1;
    prev_exit_status_ = 0;
}

void Shell::addSubprocess_(pid_t pid) {
    subprocesses_.insert(pid);
    cerr << "Spawned child process " << pid << endl;
}

void Shell::watchSubprocesses_() {
    if (subprocesses_.empty()) {
        return;
    }
    int status;
    set<pid_t> erased;
    for (auto &pid: subprocesses_) {
        if (waitpid(pid, &status, WNOHANG) == pid) {
            erased.insert(pid);
    cerr << "Process " << pid << " exited: " << WEXITSTATUS(status) << endl;
        }
    }
    for (auto &pid: erased) {
        subprocesses_.erase(pid);
    }
}

void Shell::terminalWatchSubprocesses_() {
    //cout << subprocesses_.size();
    if (subprocesses_.empty()) {
        return;
    }
    //cout << "WTF!!!" << endl;
    int statuses[subprocesses_.size()];
    int idx = 0;
    for (size_t i = 0; i < subprocesses_.size(); ++i) {
        statuses[i] = 0;
    }
    for (auto &pid: subprocesses_) {
        //cout << "PID: " << pid << endl;
        waitpid(pid, statuses + idx, 0);
        //perror("");
        ++idx;
    }
    idx = 0;
    for (auto &pid: subprocesses_) {
        //cout << statuses[idx] << endl;
        cerr << "Process " << pid << " exited: " << WEXITSTATUS(statuses[idx]) << endl;
        ++idx;
    }
    subprocesses_.clear();
}

void Shell::calcAnd(Call &call, int &status) {
    if (prev_exit_status_.success()) {
        int pid = fork();
        if (pid == 0) {
            exit(call.exec());
        }
        call.closeFd();
        //prev_executed = true;
        cur_childs.insert(pid);
        waitpid(pid, &status, 0);
        cur_childs.erase(pid);
        prev_exit_status_ = ExitStat(status);
    } //else {
    //prev_executed = false;
    //}
}

void Shell::calcOr(Call &call, int &status) {
    if (!prev_exit_status_.success()) {
        int pid = fork();
        if (pid == 0) {
            exit(call.exec());
        }
        call.closeFd();
        //prev_executed = true;
        cur_childs.insert(pid);
        waitpid(pid, &status, 0);
        cur_childs.erase(pid);
        prev_exit_status_ = ExitStat(status);
    } //else {
   //prev_executed = false;
    //}
}

void Shell::calcCommand(Call &call, int &status) {
    int pid = fork();
    if (pid == 0) {
        exit(call.exec());
    }
    call.closeFd();
    //prev_executed = true;
    cur_childs.insert(pid);
    waitpid(pid, &status, 0);
    cur_childs.erase(pid);
    prev_exit_status_ = ExitStat(status);
}

void Shell::calcPipeline(Call &call, int &status, const vector<Lexem> &lexems, size_t &idx, string &prev_op) {
    vector<int> pipe_pids;
    bool first = true;
    while (idx < lexems.size() && lexems[idx].type == "pipe") {
        pipe(pipefd_);
        prev_readfd_ = pipefd_[0];
        call.setWriteFd(pipefd_[1]);
        
        if (first && ((prev_op == "and" && !prev_exit_status_.success()) ||
                        (prev_op == "or" && prev_exit_status_.success()))) {
            call.closeFd();
            ++idx;
            call = Call(lexems, idx, prev_readfd_);
            first = false;
            continue;
        }
        int pid = fork();
        if (pid == 0) {
            exit(call.exec());
        }
        pipe_pids.push_back(pid);
        cur_childs.insert(pid);
        call.closeFd();
        ++idx;
        call = Call(lexems, idx, prev_readfd_);
        first = false;
    }

    int pid = fork();
    if (pid == 0) {
        exit(call.exec());
    }
    pipe_pids.push_back(pid);
    cur_childs.insert(pid);
    call.closeFd();

    for (size_t i = 0; i < pipe_pids.size(); ++i) {
        if (i == pipe_pids.size() - 1) {
            waitpid(pipe_pids[i], &status, 0);
            cur_childs.erase(pipe_pids[i]);
        } else {
            waitpid(pipe_pids[i], nullptr, 0);
            cur_childs.erase(pipe_pids[i]);
        }
    }
    
    prev_exit_status_ = ExitStat(status);
    prev_readfd_ = 0;
}


int Shell::calcCalls_(const vector<Lexem> &lexems) {
    size_t idx = 0;
    int status;
    //bool prev_executed = true;
    string prev_op = "";
    if (lexems.empty()) {
        return 0;
    }
    prev_exit_status_ = ExitStat(0);
    while (idx < lexems.size()) {
        readfd_ = prev_readfd_;
        Call call(lexems, idx, readfd_, writefd_);
        
        if (idx < lexems.size() && lexems[idx].type == "pipe") {
            calcPipeline(call, status, lexems, idx, prev_op);
        } else if (prev_op == "") {
            calcCommand(call, status);
        } else if (prev_op == "and") {
            calcAnd(call, status);
        } else if (prev_op == "or") {
            calcOr(call, status);
        }

        if (idx < lexems.size()) {
            prev_op = lexems[idx].type;
            ++idx;
        }
        watchSubprocesses_();
        //cout << WEXITSTATUS(status) << endl;

    }
    //cout << "ST: " << status << ' ' << WEXITSTATUS(status) << endl;
    return WEXITSTATUS(status);
}

void handler(int signum) {
    signal(signum, handler);
    for (auto &child: cur_childs) {
        kill(child, signum);
    }
}

int Shell::run() {
    signal(SIGINT, handler);
    string cur_commands;
    vector<pid_t> pids;
    while (getline(cin, cur_commands)) {
        //cout << "COMMAND: " << cur_commands << endl;
        LexemParser lp(cur_commands); 
        lp.parse();
        vector<Lexem> lexems = lp.get();
        
        if (!lexems.empty() && lexems.back().type == "subprocess") {
            pid_t pid = fork();
            if (pid == 0) {
                int s = calcCalls_(lexems);
                exit(s);
            }
            addSubprocess_(pid);
        } else {
            calcCalls_(lexems);
        }
    }
    terminalWatchSubprocesses_();
    return 0;
}

int main() {
    Shell sh;
    sh.run();
    return 0;
}
