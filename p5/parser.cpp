#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include "parser.hpp" 

using std::string;
using std::vector;
using std::stringstream;
using std::cout;
using std::endl;
LexemParser::LexemParser(const string &s): input_(s) {}

string LexemParser::takeType_(const string& s) {
    if (s == "&") return "subprocess";
    else if (s == "|") return "pipe";
    else if (s == "&&") return "and";
    else if (s == "||") return "or";
    else if (s == "<") return "input";
    else if (s == ">") return "output";
    else return "argument";
}

void LexemParser::parse() {
    stringstream ss;
    ss << input_;
    string temp;
    while (ss >> temp) {
        //want to check merged string like cat>out<inp
        auto prev_it = temp.begin(), it1 = prev_it;
        Lexem cur_lexem;
        string cur_type;
        while (it1 < temp.end()) {
            for (size_t i = 2; i >= 1; --i) {
                if (it1 + i <= temp.end()) {
                    cur_type = takeType_(string(it1, it1 + i));
                    if (cur_type != "argument") {
                        if (prev_it < it1) { //not only command
                            cur_lexem.lexem = string(prev_it, it1);
                            cur_lexem.type = "argument";
                            lexems_.push_back(cur_lexem);
                        }

                        cur_lexem.lexem = string(it1, it1 + i);
                        cur_lexem.type = cur_type;
                        lexems_.push_back(cur_lexem);
                        prev_it = it1 + i;
                        it1 += i - 1;
                        //cout << "FOUND" << endl;
                        break;
                    }
                }
            }
            ++it1;
        }
        if (prev_it != temp.end()) {
            cur_lexem.lexem = string(prev_it, temp.end());
            cur_lexem.type = "argument";
            lexems_.push_back(cur_lexem);
        }
    }
    //cout << lexems_.size() << endl;
}

vector<Lexem> LexemParser::get() const {
    return lexems_;
}
