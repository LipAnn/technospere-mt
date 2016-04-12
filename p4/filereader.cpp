#include <iostream>
#include <cstdio>
#include <utility>
#include <map>
#include <cstdint>
#include <fstream>
#include <string>

#include "filereader.hpp"

using std::map;
using std::cout;
using std::ifstream;
using std::ofstream;
using std::string;
using std::ios;

FileReader::FileReader(string f): filename_(f) {}

string FileReader::read() {
    ifstream fin(filename_, ios::in | ios::binary);
    uint64_t cur_docid = 0, cur_word = 0;
    uint32_t cnt_words = 0;
    string output_filename = "pairs";
    ofstream fout(output_filename);
    while (fin.read(reinterpret_cast<char*>(&cur_docid), sizeof(cur_docid))) {
        //cout << "CUR_DOCID: " << cur_docid << ' ';
        fin.read(reinterpret_cast<char*>(&cnt_words), sizeof(cnt_words));
        //cout << cnt_words << ' ';
        for (uint32_t i = 0; i < cnt_words; ++i) {
            fin.read(reinterpret_cast<char*>(&cur_word), sizeof(cur_word));
            //cout << cur_word << ' ';
            if (word_dict_.find(cur_word) == word_dict_.end()) {
                word_dict_[cur_word] = 1;
            } else {
                word_dict_[cur_word] += 1;
            }
            fout << cur_word << ' ' << cur_docid << ' ';
        }
        //cout << '\n';
    }
    fin.close();
    fout.close();
    return output_filename;
}

map<uint64_t, uint64_t> FileReader::getWordDict() const {
    return word_dict_;
}


