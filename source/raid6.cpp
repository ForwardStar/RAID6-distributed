#include "raid6.h"

void RAID6FileSystem::write_disk(std::string path, std::vector<uint64_t> &checksum_q) {
    namespace fs = std::filesystem;
    fs::create_directories(path);
    for (int j = 0; j < m; j++) {
        std::ofstream f(path + "/chunk" + std::to_string(j));
        for (int i = chunk_size - 1; i >= 0; i--) {
            if (checksum_q[j] & (1ull << i)) {
                f << "1";
            }
            else {
                f << "0";
            }
        }
    }
}

void RAID6FileSystem::compute_checksum(std::vector<uint64_t> &checksum_p, std::vector<uint64_t> &checksum_q) {
    uint64_t g = 1;
    for (int i = 0; i < n; i++) {
        if (!std::filesystem::is_directory("./filesystem/disk" + std::to_string(i))) {
            g <<= 1;
            g %= p;
            continue;
        }
        for (int j = 0; j < m; j++) {
            uint64_t contents = 0;
            std::ifstream f;
            f.open("./filesystem/disk" + std::to_string(i) + "/chunk" + std::to_string(j));
            if (f) {
                int k = chunk_size - 1;
                char c;
                while (f >> c) {
                    contents ^= ((c - '0') << k);
                    k--;
                }
            }
            if (checksum_p.size() <= j) {
                checksum_p.push_back(contents);
            }
            else {
                checksum_p[j] ^= contents;
            }
            if (checksum_q.size() <= j) {
                checksum_q.push_back(contents * g % p);
            }
            else {
                checksum_q[j] ^= (contents * g % p);
            }
        }
        g <<= 1;
        g %= p;
    }
}

void RAID6FileSystem::check_and_fix() {
    Logger L;
    L.log(INFO, "Checking whether there are missing disks.");
    std::unordered_set<int> lost_disks;
    for (int i = 0; i < n; i++) {
        if (std::filesystem::is_directory("./filesystem/disk" + std::to_string(i))) {
            continue;
        }
        else {
            lost_disks.insert(i);
        }
    }
    bool checksum1_corrupt = false;
    bool checksum2_corrupt = false;
    if (!std::filesystem::is_directory("./filesystem/disk_checksum1")) {
        checksum1_corrupt = true;
    }
    if (!std::filesystem::is_directory("./filesystem/disk_checksum2")) {
        checksum2_corrupt = true;
    }
    std::string msg = "";
    int corrupt_num = lost_disks.size() + checksum1_corrupt + checksum2_corrupt;
    if (corrupt_num == 0) {
        msg += "No missing disks detected.";
        L.log(INFO, msg);
        return;
    }
    else {
        msg += std::to_string(corrupt_num) + " disks are corrupted: ";
        for (auto u : lost_disks) {
            msg += "disk" + std::to_string(u) + " ";
        }
        if (checksum1_corrupt) {
            msg += "disk_checksum1";
        }
        if (checksum2_corrupt) {
            msg += "disk_checksum2";
        }
    }
    L.log(INFO, msg);
    if (corrupt_num > 2) {
        L.log(ERROR, "More than 2 disks are missing. I could not fix it.");
        return;
    }
    L.log(INFO, "No more than 2 disks are corrupted. I am trying to fix it...");
    std::vector<uint64_t> checksum_p;
    std::vector<uint64_t> checksum_q;
    compute_checksum(checksum_p, checksum_q);
    std::vector<uint64_t> real_checksum_p;
    std::vector<uint64_t> real_checksum_q;
    if (!checksum1_corrupt) {
        for (int j = 0; j < m; j++) {
            uint64_t contents = 0;
            std::ifstream f;
            f.open("./filesystem/disk_checksum1");
            if (f) {
                int k = 63;
                char c;
                while (f >> c) {
                    contents ^= ((c - '0') << k);
                    k--;
                }
            }
            real_checksum_p.push_back(contents);
        }
    }
    if (!checksum2_corrupt) {
        for (int j = 0; j < m; j++) {
            uint64_t contents = 0;
            std::ifstream f;
            f.open("./filesystem/disk_checksum2");
            if (f) {
                int k = 63;
                char c;
                while (f >> c) {
                    contents ^= ((c - '0') << k);
                    k--;
                }
            }
            real_checksum_q.push_back(contents);
        }
    }
    if (corrupt_num == 1) {
        if (!checksum1_corrupt && !checksum2_corrupt) {
            for (int j = 0; j < m; j++) {
                for (int i = chunk_size - 1; i >= 0; i--) {
                    checksum_p[j] ^= real_checksum_p[j];
                }
            }
            int u = *lost_disks.begin();
            write_disk("./filesystem/disk" + std::to_string(u), checksum_p);
        }
        else if (!checksum1_corrupt && checksum2_corrupt) {
            write_disk("./filesystem/disk_checksum2", checksum_q);
        }
        else if (checksum1_corrupt && !checksum2_corrupt) {
            write_disk("./filesystem/disk_checksum1", checksum_p);
        }
    }
    if (corrupt_num == 2) {
        if (!checksum1_corrupt && checksum2_corrupt) {
            int u = *lost_disks.begin();
            namespace fs = std::filesystem;
            fs::create_directories("./filesystem/disk" + std::to_string(u));
            fs::create_directories("./filesystem/disk_checksum2");
            uint64_t g = 1;
            for (int i = 0; i < u; i++) {
                g <<= 1;
                g %= p;
            }
            for (int j = 0; j < m; j++) {
                std::ofstream f("./filesystem/disk" + std::to_string(u) + "/chunk" + std::to_string(j));
                for (int i = chunk_size - 1; i >= 0; i--) {
                    if ((checksum_p[j] ^ real_checksum_p[j]) & (1ull << i)) {
                        f << "1";
                    }
                    else {
                        f << "0";
                    }
                }
                checksum_q[j] ^= ((checksum_p[j] ^ real_checksum_p[j]) * g % p);
            }
            for (int j = 0; j < m; j++) {
                std::ofstream f("./filesystem/disk_checksum2/chunk" + std::to_string(j));
                for (int i = chunk_size - 1; i >= 0; i--) {
                    if (checksum_q[j] & (1ull << i)) {
                        f << "1";
                    }
                    else {
                        f << "0";
                    }
                }
            }
        }
        else if (checksum1_corrupt && !checksum2_corrupt) {

        }
        else if (!checksum1_corrupt && !checksum2_corrupt) {

        }
        else {
            namespace fs = std::filesystem;
            fs::create_directories("./filesystem/disk_checksum1");
            for (int j = 0; j < m; j++) {
                std::ofstream f("./filesystem/disk_checksum1/chunk" + std::to_string(j));
                for (int i = chunk_size - 1; i >= 0; i--) {
                    if (checksum_p[j] & (1ull << i)) {
                        f << "1";
                    }
                    else {
                        f << "0";
                    }
                }
            }
            fs::create_directories("./filesystem/disk_checksum2");
            for (int j = 0; j < m; j++) {
                std::ofstream f("./filesystem/disk_checksum2/chunk" + std::to_string(j));
                for (int i = chunk_size - 1; i >= 0; i--) {
                    if (checksum_q[j] & (1ull << i)) {
                        f << "1";
                    }
                    else {
                        f << "0";
                    }
                }
            }
        }
    }
}

void RAID6FileSystem::insert(std::string filename, int length, std::string content) {
    for (auto u : pieces) {
        if ((u.second - u.first) * chunk_size >= length) {
            file_map[filename] = u.first;
            file_length[filename] = length;
            for (int i = u.first; i < u.second; i++) {
                if ((i - u.first) * chunk_size >= length) {
                    break;
                }
                int disk_idx = i / m;
                int chunk_idx = i % m;
                std::string chunk_content = "";
                for (int j = 0; j < chunk_size; j++) {
                    if ((i - u.first) * chunk_size + j < length) {
                        chunk_content += content[(i - u.first) * chunk_size + j];
                    }
                }
                std::ofstream f("./filesystem/disk" + std::to_string(disk_idx) + "/chunk" + std::to_string(chunk_idx));
                f << chunk_content;
            }
            std::vector<uint64_t> checksum_p, checksum_q;
            compute_checksum(checksum_p, checksum_q);
            write_disk("./filesystem/disk_checksum1", checksum_p);
            write_disk("./filesystem/disk_checksum2", checksum_q);
            int l = u.first;
            int r = u.second;
            pieces.erase(u);
            if (length % chunk_size == 0) {
                pieces.insert(std::make_pair(l + length / chunk_size, r));
            }
            else {
                pieces.insert(std::make_pair(l + length / chunk_size + 1, r));
            }
            break;
        }
    }
}

void RAID6FileSystem::del(std::string filename) {
    Logger L;
    if (file_map.find(filename) == file_map.end()) {
        L.log(ERROR, "This file does not exist!");
        return;
    }
    int l = file_map[filename];
    int r = l + file_length[filename] / chunk_size;
    if (file_length[filename] % chunk_size > 0) {
        r++;
    }
    pieces.insert(std::make_pair(l, r));
    file_map.erase(filename);
    file_length.erase(filename);
    for (int i = l; i < r; i++) {
        int disk_idx = i / m;
        int chunk_idx = i % m;
        std::string path = "./filesystem/disk" + std::to_string(disk_idx) + "/chunk" + std::to_string(chunk_idx);
        std::remove(path.c_str());
    }
    std::vector<uint64_t> checksum_p, checksum_q;
    compute_checksum(checksum_p, checksum_q);
    write_disk("./filesystem/disk_checksum1", checksum_p);
    write_disk("./filesystem/disk_checksum2", checksum_q);
}

std::string RAID6FileSystem::retrieve(std::string filename) {
    Logger L;
    if (file_map.find(filename) == file_map.end()) {
        L.log(ERROR, "This file does not exist!");
        return "";
    }
    int l = file_map[filename];
    int r = l + file_length[filename] / chunk_size;
    if (file_length[filename] % chunk_size > 0) {
        r++;
    }
    std::string content = "";
    for (int i = l; i < r; i++) {
        int disk_idx = i / m;
        int chunk_idx = i % m;
        std::string path = "./filesystem/disk" + std::to_string(disk_idx) + "/chunk" + std::to_string(chunk_idx);
        std::ifstream f(path);
        std::string sub_content;
        f >> sub_content;
        content += sub_content;
    }
    return content;
}

void RAID6FileSystem::list_all_files() {
    std::string msg = "Current files: { ";
    Logger L;
    for (auto u : file_map) {
        msg += u.first + " ";
    }
    msg += "}.";
    L.log(INFO, msg);
}

RAID6FileSystem::RAID6FileSystem(int num_of_disks, int modulo, int c_size, int num_trunks) {
    n = num_of_disks;
    m = num_trunks;
    p = modulo;
    chunk_size = c_size;
    Logger L;
    L.log(INFO, "Initializing RAID6 distributed file system...");
    pieces.insert(std::make_pair(0, n * m));
    namespace fs = std::filesystem;
    fs::remove_all("./filesystem");
    fs::create_directories("./filesystem");
    for (int i = 0; i < num_of_disks; i++) {
        fs::create_directories("./filesystem/disk" + std::to_string(i));
    }
    fs::create_directories("./filesystem/disk_checksum1");
    fs::create_directories("./filesystem/disk_checksum2");
    L.log(INFO, "Successfully initialize RAID6 distributed file system!");
}