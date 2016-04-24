#include <iostream>
#include <cstdio>
#include <map>
#include <fstream>
#include <vector>
#include <utility>
#include <string>
#include <set>

using std::ifstream;
using std::ofstream;
using std::vector;
using std::pair;
using std::string;
using std::set;

struct IdsStruct {
    uint64_t file_pos_idx;
    uint64_t next_offset;
    uint64_t word;
    uint64_t doc;

    IdsStruct(uint64_t, uint64_t, uint64_t, uint64_t); 
    bool operator<(const IdsStruct&) const;
};

class FileSorter {
public:
    FileSorter(string);
    string sort();
private:    
    string unsortedfile_;
    string partly_sorted_file_;  
    string sortedfile_;
    uint64_t available_memory_; // = 1024 * 1024 * 1024 * 1.0;//1024 * 1024 * 1.2;
    size_t filesize_;
    bool initialize_;
    size_t max_pairs_; // = available_memory_ / 16;
    vector<pair<uint64_t, uint64_t>> sort_buf_;
    vector<size_t> file_positions_;
    vector<size_t> next_file_positions_;
    set<IdsStruct> heap_;

    size_t getFileSize(string&);
    void partlySort();
    pair<uint64_t, uint64_t> getMin(ifstream&);

};
