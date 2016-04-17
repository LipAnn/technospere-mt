#pragma once

#include <iostream>
#include <string>

using std::string;
using std::vector;

struct Lexem {
    string lexem;
    string type;
};

class LexemParser {
public:
    LexemParser(const string&);
    void parse();
    vector<Lexem> get() const;

private:
    string input_;
    vector<Lexem> lexems_;

    string takeType_(const string&);
};
