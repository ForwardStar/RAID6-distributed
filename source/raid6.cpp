#include "raid6.h"

extern GaloisNumber Modulo, Generator;

void RAID6FileSystem::write_disk(std::string path, std::vector<GaloisNumber> &checksum_q) {
    namespace fs = std::filesystem;
    fs::create_directories(path);
    for (int j = 0; j < m; j++) {
        std::ofstream f(path + "/chunk" + std::to_string(j));
        for (int i = 0; i < checksum_q[j].bit_vector.size(); i++) {
            f << checksum_q[j].bit_vector[i];
        }
    }
}

void RAID6FileSystem::compute_checksum(std::vector<GaloisNumber> &checksum_p, std::vector<GaloisNumber> &checksum_q) {
    GaloisNumber g = GaloisNumber(Modulo.bit_vector.size() - 1, 2);
    for (int i = n - 1; i >= 0; i--) {
        if (!std::filesystem::is_directory("./filesystem/disk" + std::to_string(i))) {
            for (int j = 0; j < m; j++) {
                if (checksum_q.size() <= j) {
                    checksum_q.push_back(GaloisNumber(Modulo.bit_vector.size() - 1, 0));
                }
                else {
                    checksum_q[j] = checksum_q[j] * g;
                }
            }
            continue;
        }
        for (int j = 0; j < m; j++) {
            std::vector<int> bit_vector;
            bit_vector.assign(Modulo.bit_vector.size() - 1, 0);
            std::ifstream f;
            f.open("./filesystem/disk" + std::to_string(i) + "/chunk" + std::to_string(j));
            if (f) {
                int k = 0;
                char c;
                while (f >> c) {
                    bit_vector[k++] = c - '0';
                }
            }
            if (checksum_p.size() <= j) {
                checksum_p.push_back(GaloisNumber(bit_vector));
            }
            else {
                checksum_p[j] = checksum_p[j] + GaloisNumber(bit_vector);
            }
            if (checksum_q.size() <= j) {
                checksum_q.push_back(GaloisNumber(bit_vector));
            }
            else {
                checksum_q[j] = checksum_q[j] * g + GaloisNumber(bit_vector);
            }
        }
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
    std::vector<GaloisNumber> checksum_p;
    std::vector<GaloisNumber> checksum_q;
    compute_checksum(checksum_p, checksum_q);
    std::vector<GaloisNumber> real_checksum_p;
    std::vector<GaloisNumber> real_checksum_q;
    if (!checksum1_corrupt) {
        for (int j = 0; j < m; j++) {
            std::vector<int> bit_vector;
            bit_vector.assign(Modulo.bit_vector.size() - 1, 0);
            std::ifstream f;
            f.open("./filesystem/disk_checksum1/chunk" + std::to_string(j));
            if (f) {
                int k = 0;
                char c;
                while (f >> c) {
                    bit_vector[k++] = c - '0';
                }
            }
            real_checksum_p.push_back(GaloisNumber(bit_vector));
        }
    }
    if (!checksum2_corrupt) {
        for (int j = 0; j < m; j++) {
            std::vector<int> bit_vector;
            bit_vector.assign(Modulo.bit_vector.size() - 1, 0);
            std::ifstream f;
            f.open("./filesystem/disk_checksum2/chunk" + std::to_string(j));
            if (f) {
                int k = 0;
                char c;
                while (f >> c) {
                    bit_vector[k++] = c - '0';
                }
            }
            real_checksum_q.push_back(GaloisNumber(bit_vector));
        }
    }
    if (corrupt_num == 1) {
        if (!checksum1_corrupt && !checksum2_corrupt) {
            for (int j = 0; j < m; j++) {
                checksum_p[j] = checksum_p[j] + real_checksum_p[j];
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
            GaloisNumber g = Generator ^ u;
            for (int j = 0; j < m; j++) {
                checksum_p[j] = checksum_p[j] + real_checksum_p[j];
                checksum_q[j] = (checksum_q[j] + checksum_p[j]) * g;
            }
            write_disk("./filesystem/disk" + std::to_string(u), checksum_p);
            write_disk("./filesystem/disk_checksum2", checksum_q);
        }
        else if (checksum1_corrupt && !checksum2_corrupt) {
            int u = *lost_disks.begin();
            namespace fs = std::filesystem;
            fs::create_directories("./filesystem/disk" + std::to_string(u));
            fs::create_directories("./filesystem/disk_checksum2");
            GaloisNumber g = Generator ^ ((1 << Modulo.bit_vector.size() - 1) - u - 1);
            for (int j = 0; j < m; j++) {
                checksum_q[j] = (checksum_q[j] + real_checksum_q[j]) * g;
                checksum_p[j] = checksum_p[j] + checksum_q[j];
            }
            write_disk("./filesystem/disk" + std::to_string(u), checksum_q);
            write_disk("./filesystem/disk_checksum1", checksum_p);
        }
        else if (!checksum1_corrupt && !checksum2_corrupt) {
            int u = -1, v = -1;
            for (auto w : lost_disks) {
                if (u == -1) {
                    u = w;
                }
                else {
                    v = w;
                }
            }
            if (u > v) {
                std::swap(u, v);
            }
            GaloisNumber g1 = Generator ^ (v - u);
            GaloisNumber g2 = Generator ^ ((1 << Modulo.bit_vector.size() - 1) - u - 1);
            GaloisNumber A = g1 * ((g1 + GaloisNumber(Modulo.bit_vector.size() - 1, 1)) ^ ((1 << Modulo.bit_vector.size() - 1) - 2));
            GaloisNumber B = g2 * ((g1 + GaloisNumber(Modulo.bit_vector.size() - 1, 1)) ^ ((1 << Modulo.bit_vector.size() - 1) - 2));
            std::vector<GaloisNumber> D1, D2;
            for (int j = 0; j < m; j++) {
                checksum_p[j] = checksum_p[j] + real_checksum_p[j];
                checksum_q[j] = checksum_q[j] + real_checksum_q[j];
                D1.push_back(A * checksum_p[j] + B * checksum_q[j]);
                D2.push_back(checksum_p[j] + D1[D1.size() - 1]);
            }
            write_disk("./filesystem/disk" + std::to_string(u), D1);
            write_disk("./filesystem/disk" + std::to_string(v), D2);
        }
        else {
            write_disk("./filesystem/disk_checksum1", checksum_p);
            write_disk("./filesystem/disk_checksum2", checksum_q);
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
            std::vector<GaloisNumber> checksum_p, checksum_q;
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
            return;
        }
    }
    Logger L;
    L.log(ERROR, "Create file failed for insufficient storage space! Try deleting some files or defragmentation.");
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
    std::vector<GaloisNumber> checksum_p, checksum_q;
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
    content = content.substr(0, file_length[filename]);
    return content;
}

void RAID6FileSystem::defragmentation() {
    Logger L;
    L.log(INFO, "Performing defragmentation...");
    std::unordered_map<std::string, std::string> files;
    std::vector<std::string> filenames;
    for (auto u : file_map) {
        filenames.push_back(u.first);
    }
    for (auto u : filenames) {
        files[u] = retrieve(u);
        del(u);
    }
    pieces.clear();
    pieces.insert(std::make_pair(0, n * m));
    for (auto u : files) {
        insert(u.first, int(u.second.size()), u.second);
    }
    L.log(INFO, "Successfully finish defragmentation!");
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

RAID6FileSystem::RAID6FileSystem(int num_of_disks, int c_size, int num_trunks) {
    n = num_of_disks;
    m = num_trunks;
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