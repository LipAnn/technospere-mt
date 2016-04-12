#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <cstdint>
#include <fstream>
#include <string>
#include <ios>

using std::ifstream;
using std::ofstream;
using std::map;
using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::ios;


int main(int argc, char **argv)
{
    ifstream fin(argv[1], ios::in | ios::binary);
    uint64_t cur_docid = 0, cur_word = 0;
    uint32_t cnt_words = 0;
    map<uint64_t, vector<uint64_t>> word_dict;
    while (fin.read(reinterpret_cast<char*>(&cur_docid), sizeof(cur_docid))) {
        //cout << "CUR_DOCID: " << cur_docid << ' ';
        fin.read(reinterpret_cast<char*>(&cnt_words), sizeof(cnt_words));
        //cout << cnt_words << ' ';
        for (uint32_t i = 0; i < cnt_words; ++i) {
            fin.read(reinterpret_cast<char*>(&cur_word), sizeof(cur_word));
            //cout << cur_word << ' ';
            word_dict[cur_word].push_back(cur_docid);
        }
    }
    fin.close();

    ofstream right_ans("right.bin", ios::out | ios::binary);
    size_t i = 0;
    uint64_t prev_cnt_docs = 0, cur_offset = 0;
    for (auto it = word_dict.begin(); it != word_dict.end(); ++it, ++i) {
        right_ans.write(reinterpret_cast<const char*>(&(it->first)), sizeof(it->first)); //ask why const?
        if (i == 0) {
            cur_offset += 16 * (word_dict.size() + 1);
            prev_cnt_docs = (it->second).size();
            right_ans.write(reinterpret_cast<char*>(&cur_offset), sizeof(cur_offset));
        } else {
            cur_offset += 4 + prev_cnt_docs * 8;
            prev_cnt_docs = (it->second).size();
            right_ans.write(reinterpret_cast<char*>(&cur_offset), sizeof(cur_offset));
        }
    }
    //cout << "WRITE WORDS AND OFFSETS\n";
    uint64_t temp = 0;
    right_ans.write(reinterpret_cast<char*>(&temp), sizeof(temp));
    right_ans.write(reinterpret_cast<char*>(&temp), sizeof(temp));

    uint64_t cur_w, cur_d;
    for (auto it = word_dict.begin(); it != word_dict.end(); ++it) {
        uint32_t cnt_docs = static_cast<int32_t>((it->second).size()); //may change to 64
        right_ans.write(reinterpret_cast<char*>(&cnt_docs), sizeof(cnt_docs));
        for (auto j = (it->second).begin(); j < (it->second).end(); ++j) {
            right_ans.write(reinterpret_cast<char*>(&(*j)), sizeof(*j));
        }
    }
    
    right_ans.close();
	return 0;	
}
