#include <iostream>
#include <cstdio>
#include <map>
#include <cstdint>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <climits>
#include <vector>
#include <utility>
#include <ios>
#include <iterator>

#include "filesorter.hpp"

using std::ifstream;
using std::ofstream;
using std::vector;
using std::pair;
using std::make_pair;
using std::cout;
using std::copy;
using std::ostream_iterator;
using std::remove;

size_t FileSorter::getFileSize(string &file) {
    ifstream ftemp(file);
    ftemp.seekg(0, ftemp.end);
    size_t ans = ftemp.tellg();
    ftemp.close();
    return ans;
}

void FileSorter::partlySort() {
    ifstream fin(unsortedfile_);
    ofstream fout(partly_sorted_file_);
    uint64_t cur_w = 0, cur_d = 0;
    size_t read_pairs = 0;
    file_positions_.push_back(0);
    while (fin >> cur_w >> cur_d) {
        sort_buf_[read_pairs] = make_pair(cur_w, cur_d);        
        read_pairs += 1;
        
        if (read_pairs >= max_pairs_) { //flush sorted part to file
            std::sort(sort_buf_.begin(), sort_buf_.end());                
            for (size_t i = 0; i < read_pairs; ++i) {
                fout << sort_buf_[i].first << ' ' << sort_buf_[i].second << ' ';
            }
            read_pairs = 0;
            int64_t cur_offset = fin.tellg();
            if (cur_offset == -1) {
                file_positions_.push_back(filesize_);
                next_file_positions_.push_back(filesize_);
            } else {
                file_positions_.push_back(cur_offset);
                next_file_positions_.push_back(cur_offset);
            }
        }
    }
    if (read_pairs > 0) {
        std::sort(sort_buf_.begin(), sort_buf_.begin() + read_pairs);                
        for (size_t i = 0; i < read_pairs; ++i) {
            fout << sort_buf_[i].first << ' ' << sort_buf_[i].second << ' ';
        }
        read_pairs = 0;
        next_file_positions_.push_back(filesize_);
    } else {
        file_positions_.pop_back();
    }
    assert(file_positions_.size() == next_file_positions_.size());
    //cout << "POSITIONS: " << file_positions_.size() << ' ' << file_positions_[0] << ' ' << next_file_positions_[0] << '\n';
    fin.close();
    remove(unsortedfile_.c_str());
    fout.close();
}


pair<pair<int64_t, int64_t>,  pair<uint64_t, uint64_t>> FileSorter::getMin(ifstream &fin) {
    pair<uint64_t, uint64_t> min_pair(UINT64_MAX, UINT64_MAX), tmp_pair;
    int64_t min_ind = -1;
    uint64_t cur_w = -1, cur_d = -1;
    int64_t next_offset = -1;
    bool was_read = false;
    //cout << "CUR_POS IN GET_MIN: ";
    //copy(file_positions_.begin(), file_positions_.end(), ostream_iterator<size_t>(cout, " "));
    //cout << "\n";
    //cout << "NEXT_POS IN GET_MIN: ";
    //copy(next_file_positions_.begin(), next_file_positions_.end(), ostream_iterator<size_t>(cout, " "));
    //cout << "\n";
    for (size_t i = 0; i < file_positions_.size(); ++i) {
        if (file_positions_[i] < next_file_positions_[i]) {
            fin.seekg(file_positions_[i], fin.beg);
            //cout << "HERE " << file_positions_[i] << '\n';
            if (fin >> cur_w >> cur_d) {
                //if (cur_w == 53) {
                    //cout << "BUG: " << file_positions_[i] << ' ' << fin.tellg() << '\n';
                    //for (int j = 0; j < 25; ++j) {
                    //fin.seekg(file_positions_[i], fin.beg);
                    //if (fin >> cur_w >> cur_d) {
                    //    cout << "OK\n";
                    //}
                   // }
                //}
                was_read = true;
                //cout << "POS: " << file_positions_[i] << std::endl;
                //cout << "SHIFT WORD/DOC: " << cur_w << ' ' << cur_d << '\n';
                tmp_pair = make_pair(cur_w, cur_d);
                if (tmp_pair < min_pair) {
                    min_pair = tmp_pair;
                    min_ind = i;
                    next_offset = fin.tellg();
                }
            } else {
                //cout << "HMMM:\n";
                fin.close();
                fin.open(partly_sorted_file_);
            }
        }
    }
    if (next_offset == -1) {
        next_offset = filesize_;
    }
    if (!was_read) {
        min_ind = -1;
    }
    return make_pair(make_pair(min_ind, next_offset), min_pair);
}

string FileSorter::sort() {
    partlySort();
    ifstream fin(partly_sorted_file_);
    ofstream fout(sortedfile_);

    while (true) {
        auto cur_min = getMin(fin);
        //cout << "CUR_MIN: " << cur_min.first.first << ' ' << cur_min.first.second << ' ' << cur_min.second.first << ' ' << cur_min.second.second << '\n';
        //cout << "CUR_POSITIONS: ";
        //copy(file_positions_.begin(), file_positions_.end(), ostream_iterator<size_t>(cout, " "));
        //cout << "\n";
        //cout << "NEXT_POSITIONS: ";
        //copy(next_file_positions_.begin(), next_file_positions_.end(), ostream_iterator<size_t>(cout, " "));
        //cout << "\n";
        if (cur_min.first.first == -1) {
            //fin.seekg(0);
            //uint64_t cur_w = -1, cur_d = -1;
            //if (fin >> cur_w >> cur_d) {
                //cout << "WTF?!!!!\n";
            //}
            break;
        }

        file_positions_[cur_min.first.first]  = cur_min.first.second;
        fout << cur_min.second.first << ' ' << cur_min.second.second << ' ';
    }
    fin.close();
    remove(partly_sorted_file_.c_str());
    fout.close();
    return sortedfile_;
}

FileSorter::FileSorter(string unsorted_file): unsortedfile_(unsorted_file), partly_sorted_file_("partly_sorted_pairs"), sortedfile_("sorted_pairs") {
    sort_buf_.resize(max_pairs_);
    filesize_ = getFileSize(unsortedfile_);
    //cout << "UNSORTED SIZE: " << filesize_ << '\n';
    file_positions_.reserve(filesize_ / (2 * max_pairs_) + 2);
    next_file_positions_.reserve(filesize_ / (2 * max_pairs_) + 2);
}

