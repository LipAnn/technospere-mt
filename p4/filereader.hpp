#include <iostream>
#include <cstdio>
#include <utility>
#include <map>
#include <cstdint>
#include <string>

using std::map;
using std::string;

class FileReader {
private:
    map<uint64_t, uint64_t> word_dict_;
    string filename_;
public:
    FileReader(string);
    string read();
    map<uint64_t, uint64_t> getWordDict() const;
};
