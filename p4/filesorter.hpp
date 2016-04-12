#include <iostream>
#include <cstdio>
#include <map>
#include <fstream>
#include <vector>
#include <utility>
#include <string>

using std::ifstream;
using std::ofstream;
using std::vector;
using std::pair;
using std::string;

class FileSorter {
public:
    FileSorter(string);
    string sort();
private:    
    string unsortedfile_;
    string partly_sorted_file_;  
    string sortedfile_;
    static const uint64_t available_memory_ = 1024 * 1024 * 1024 * 1.5;//1024 * 1024 * 1.2;
    size_t filesize_;
    static const size_t max_pairs_ = available_memory_ / 128;
    vector<pair<uint64_t, uint64_t>> sort_buf_;
    vector<size_t> file_positions_;
    vector<size_t> next_file_positions_;

    size_t getFileSize(string&);
    void partlySort();
    pair<pair<int64_t, int64_t>, pair<uint64_t, uint64_t>> getMin(ifstream&);

};
