#ifndef RAID6
#define RAID6
#include <stdio.h>
#include <string.h>
#include <filesystem>
#include <vector>
#include <set>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include "logger.h"
#include "galois.h"

class RAID6FileSystem {
    private:
        void write_disk(std::string, std::vector<GaloisNumber> &checksum_q);
        void compute_checksum(std::vector<GaloisNumber> &checksum_p, std::vector<GaloisNumber> &checksum_q);

    public:
        int n, m, chunk_size;
        std::set<std::pair<int, int>> pieces;
        std::unordered_map<std::string, int> file_map;
        std::unordered_map<std::string, int> file_length;

        void check_and_fix();

        void insert(std::string filename, int length, std::string content);
        void del(std::string filename);
        std::string retrieve(std::string filename);

        void list_all_files();

        // num_disks: 3 - 257, 2 of them would be used for checksum;
        RAID6FileSystem(int num_disks, int c_size, int num_trunks);
        ~RAID6FileSystem() {};
};

#endif