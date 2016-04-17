#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <set>

#include "parser.hpp"

using std::string;
using std::cin;
using std::set;
using std::vector;

class Call {
private:
    int readfd_;
    int writefd_;
    vector<char*> argv_;

public:
    Call(const vector<Lexem>&, size_t&, int readfd=0, int writefd=1);
    
    void setWriteFd(int);
    void setReadFd(int);
    void closeFd();
    int exec();
};

struct ExitStat {
public:
    int exit_status;

    ExitStat(int st = 0);
    bool success() const;
};

class Shell {
private:
    int pipefd_[2];
    int readfd_;
    int prev_readfd_;
    int writefd_;
    ExitStat prev_exit_status_;
    set<pid_t> subprocesses_;

    int calcCalls_(const vector<Lexem>&);
    void addSubprocess_(pid_t);
    void watchSubprocesses_();
    void terminalWatchSubprocesses_();
public:
    Shell();
    int run();
};
