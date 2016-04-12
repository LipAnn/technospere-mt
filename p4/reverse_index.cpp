#include <iostream>
#include <fstream>
#include <map>
#include <cstdint>
#include <string>
#include <cstdio>

#include "filereader.hpp"
#include "filesorter.hpp"

using std::cout;
using std::ifstream;
using std::ofstream;
using std::string;
using std::ios;
using std::remove;

string makeReverseIndex(string index) {
    //return "";
    //cout << index << std::endl;
    //cout.flush();
    FileReader fr(index);
    string file_to_sort = fr.read();
    map<uint64_t, uint64_t> word_dict = fr.getWordDict(); //check if sorted //also check shift size
    //cout << "DICT: ";
    //for (auto &it: word_dict) {
        //cout << it.first << ' ' << it.second << '\n';
    //}
    FileSorter fs(file_to_sort);
    string sorted_file = fs.sort();

    string s = "reverse_index.bin";
    ofstream reverse_index(s, ios::out | ios::binary);

    size_t i = 0;
    uint64_t prev_cnt_docs = 0, cur_offset = 0;
    for (auto it = word_dict.begin(); it != word_dict.end(); ++it, ++i) {
        reverse_index.write(reinterpret_cast<const char*>(&(it->first)), sizeof(it->first)); //ask why const?
        if (i == 0) {
            cur_offset += 16 * (word_dict.size() + 1);
            prev_cnt_docs = it->second;
            reverse_index.write(reinterpret_cast<char*>(&cur_offset), sizeof(cur_offset));
        } else {
            cur_offset += 4 + prev_cnt_docs * 8;
            prev_cnt_docs = it->second;
            reverse_index.write(reinterpret_cast<char*>(&cur_offset), sizeof(cur_offset));
        }
    }
    //cout << "WRITE WORDS AND OFFSETS\n";
    uint64_t temp = 0;
    reverse_index.write(reinterpret_cast<char*>(&temp), sizeof(temp));
    reverse_index.write(reinterpret_cast<char*>(&temp), sizeof(temp));

    ifstream sorted(sorted_file);
    
    uint64_t cur_w, cur_d;
    for (auto it = word_dict.begin(); it != word_dict.end(); ++it) {
        uint32_t cnt_docs = static_cast<int32_t>(it->second); //may change to 64
        reverse_index.write(reinterpret_cast<char*>(&cnt_docs), sizeof(cnt_docs));
        for (uint64_t j = 0; j < it->second; ++j) {
            sorted >> cur_w >> cur_d;
            reverse_index.write(reinterpret_cast<char*>(&cur_d), sizeof(cur_d));
        }
    }
    //cout << "WRITE DOCS\n";
    sorted.close();
    remove(sorted_file.c_str());
    reverse_index.close();
    return s;
}

void printReadable(string file) {
    cout << "READABLE REVERSE INDEX:\n";
    ifstream fin(file, ios::in | ios::binary);
    uint64_t cur_w, cur_offset;
    size_t cnt_words = 0;
    while (true) {
        fin.read(reinterpret_cast<char*>(&cur_w), sizeof(cur_w));
        fin.read(reinterpret_cast<char*>(&cur_offset), sizeof(cur_offset));
        
        cout << cur_w << ' ' << cur_offset << '\n';
        if (cur_w == 0 && cur_offset == 0) {
            break;
        }
        ++cnt_words;
    }

    uint32_t cnt_docs;
    uint64_t cur_doc;
    for (size_t i = 0; i < cnt_words; ++i) {
        fin.read(reinterpret_cast<char*>(&cnt_docs), sizeof(cnt_docs));
        cout << cnt_docs << ' ';
        for (uint32_t j = 0; j < cnt_docs; ++j) {
            fin.read(reinterpret_cast<char*>(&cur_doc), sizeof(cur_doc));
            cout << cur_doc << ' ';
        }
        cout << '\n';
    }
    fin.close();
}


int main(int argc, char **argv) {
    //cout << argc << ' ' << argv[1] << std::endl;
    string reverse_index = makeReverseIndex(argv[1]);
    //printReadable(reverse_index);
    return 0;
}
