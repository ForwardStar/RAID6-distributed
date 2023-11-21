#include "raid6.h"
#include "galois.h"
#include <cmath>
#include <iostream>

GaloisNumber Modulo, Generator;

bool is_irreducible(GaloisNumber x) {
    for (int i = 2; i < (1 << Modulo.bit_vector.size() - 1); i++) {
        int mbs = -1;
        for (int j = Modulo.bit_vector.size() - 1; j >= 0; j--) {
            if (i & (1 << j)) {
                mbs = j;
                break;
            }
        }
        GaloisNumber y = GaloisNumber(mbs + 1, i);
        std::vector<int> bit_vector;
        bit_vector.resize(Modulo.bit_vector.size());
        for (int j = 0; j < Modulo.bit_vector.size(); j++) {
            bit_vector[j] = Modulo.bit_vector[j];
        }
        for (int j = Modulo.bit_vector.size() - 1; j >= 0; j--) {
            if (j >= mbs && bit_vector[j] == 1) {
                for (int k = mbs; k >= 0; k--) {
                    bit_vector[j - (mbs - k)] ^= y.bit_vector[k];
                }
            }
        }
        bool is_zero = true;
        for (int j = 0; j < bit_vector.size(); j++) {
            if (bit_vector[j] != 0) {
                is_zero = false;
                break;
            }
        }
        if (is_zero) {
            return false;
        }
    }
    return true;
}

GaloisNumber find_an_irreducible(int galois_degree, int x) {
    for (int i = x + 1; i < (x << 1); i++) {
        Modulo = GaloisNumber(galois_degree + 1, i);
        if (is_irreducible(Modulo)) {
            return Modulo;
        }
    }
    return GaloisNumber();
}

std::string decompose(std::string msg, GaloisNumber x) {
    std::string result = msg;
    bool head = true;
    for (int i = x.bit_vector.size() - 1; i >= 0; i--) {
        if (x.bit_vector[i] == 1) {
            if (!head) {
                result += " + ";
            }
            if (i > 1) {
                result += "x ^ " + std::to_string(i);
            }
            else if (i == 1) {
                result += "x";
            }
            else {
                result += "1";
            }
            head = false;
        }
    }
    result += '.';
    return result;
}

GaloisNumber find_a_generator(int galois_degree) {
    for (int i = 2; i < (1 << galois_degree); i++) {
        GaloisNumber g = GaloisNumber(galois_degree, i);
        if (is_irreducible(g)) {
            GaloisNumber g_power = GaloisNumber(galois_degree, i);
            bool is_generator = true;
            for (int j = 0; j < (1 << galois_degree) - 2; j++) {
                g = g * g_power;
                if (g == g_power) {
                    is_generator = false;
                    break;
                }
            }
            if (is_generator) {
                return g_power;
            }
        }
    }
    return GaloisNumber();
}

int main() {
    Logger L;
    int galois_degree, chunk_size;
    printf("For a Galois field GF(2^n), a maximum number of 2^n storage disks is allowed, and the chunk size would be n bits.\n");
    printf("Enter the degree 'n' of the Galois field (n < 32): ");
    scanf("%d", &galois_degree);
    chunk_size = galois_degree;
    L.log(INFO, "Finding a monic irreducible polynomial (x = 2) of degree n.");
    Modulo = find_an_irreducible(galois_degree, (1 << galois_degree));
    Generator = find_a_generator(galois_degree);
    L.log(INFO, decompose("The modulo irreducible polynomial is ", Modulo));
    L.log(INFO, decompose("The generator is ", Generator));
    int num_of_disks;
    printf("Enter the number of storage disks (1 - %d). Note that two extra disks would be used for checksum: ", (1 << galois_degree));
    scanf("%d", &num_of_disks);
    int num_trunks;
    printf("Enter the maximum number of chunks a disk can store: ");
    scanf("%d", &num_trunks);
    double storage_size = (double)num_trunks * chunk_size * num_of_disks / 1024 / 8;
    std::string msg = "The total storage of this file system is ";
    L.log(INFO, msg + std::to_string(storage_size) + "KB.");
    RAID6FileSystem* fs = new RAID6FileSystem(num_of_disks, chunk_size, num_trunks);
    while (1) {
        fs->check_and_fix();
        fs->list_all_files();
        printf("=====================\n");
        printf("1. Create a file;\n");
        printf("2. Delete a file;\n");
        printf("3. View a file;\n");
        printf("4. Open and modify a file;\n");
        printf("5. Compress the disks (defragmentation);\n");
        printf("6. Exit.\n");
        printf("=====================\n");
        printf("Perform an operation: ");
        int op;
        std::string filename, content, cmd, s1, s2, s3;
        char c;
        bool invalid_file = false;
        int n;
        scanf("%d", &op);
        switch (op) {
            case 1:
                printf("Enter the file name: ");
                std::cin >> filename;
                if (fs->file_map.find(filename) != fs->file_map.end()) {
                    L.log(ERROR, "This file already exists!");
                    break;
                }
                printf("Enter the length of the file: ");
                scanf("%d", &n);
                printf("Enter the contents of the file (in binary string format, e.g., '0001000101111'): ");
                std::cin >> content;
                if (content.size() != n) {
                    L.log(ERROR, "File length does not match the contents!");
                    break;
                }
                for (int i = 0; i < n; i++) {
                    if (content[i] != '0' && content[i] != '1') {
                        L.log(ERROR, "Invalid character occurs in the contents!");
                        invalid_file = true;
                        break;
                    }
                }
                if (!invalid_file) {
                    fs->insert(filename, n, content);
                }
                break;
            case 2:
                printf("Enter the file name: ");
                std::cin >> filename;
                fs->del(filename);
                break;
            case 3:
                printf("Enter the file name: ");
                std::cin >> filename;
                printf("File length: %d, file content:\n", fs->file_length[filename]);
                std::cout << fs->retrieve(filename) << std::endl;
                break;
            case 4:
                printf("Enter the file name: ");
                std::cin >> filename;
                if (fs->file_map.find(filename) == fs->file_map.end()) {
                    L.log(ERROR, "This file already does not exist!");
                    break;
                }
                while (1) {
                    printf("File length: %d, file content:\n", fs->file_length[filename]);
                    content = fs->retrieve(filename);
                    std::cout << content << std::endl;
                    printf("Enter the operation ('H' for help):\n");
                    char op_type = getchar();
                    while (op_type != 'H' && op_type != 'I' && op_type != 'D' && op_type != 'E') {
                        op_type = getchar();
                    }
                    if (op_type == 'H') {
                        printf("For insertion, enter 'I l n s' where 0 <= l < file_length, s is a binary string to insert and n = |S|.\n");
                        printf("For deletion, enter 'D l r' where 0 <= l <= r < file_length to delete contents from l to r.\n");
                        printf("To exit the file, enter 'E'.\n");
                        printf("Examples:\n");
                        printf("I 0 00011000\n");
                        printf("D 0 0\n");
                    }
                    if (op_type == 'I') {
                        int pos = 0;
                        bool invalid_cmd = false;
                        std::cin >> cmd;
                        for (int i = 0; i < cmd.size(); i++) {
                            if (cmd[i] >= '0' && cmd[i] <= '9') {
                                pos = pos * 10 + cmd[i] - '0';
                            }
                            else {
                                invalid_cmd = true;
                                break;
                            }
                        }
                        if (invalid_cmd) {
                            L.log(ERROR, "Invalid format!");
                            std::getline(std::cin, cmd);
                            break;
                        }
                        if (pos >= fs->file_length[filename] || pos < 0) {
                            L.log(ERROR, "Insert position is invalid!");
                            std::getline(std::cin, cmd);
                            break;
                        }
                        s1 = content.substr(0, pos);
                        s2 = content.substr(pos, fs->file_length[filename]);
                        int length = 0;
                        std::cin >> cmd;
                        invalid_cmd = false;
                        for (int i = 0; i < cmd.size(); i++) {
                            if (cmd[i] >= '0' && cmd[i] <= '9') {
                                length = length * 10 + cmd[i] - '0';
                            }
                            else {
                                invalid_cmd = true;
                                break;
                            }
                        }
                        if (invalid_cmd) {
                            L.log(ERROR, "Invalid format!");
                            std::getline(std::cin, cmd);
                            break;
                        }
                        std::cin >> s3;
                        if (s3.size() != length) {
                            L.log(ERROR, "File length does not match the contents!");
                            std::getline(std::cin, cmd);
                            break;
                        }
                        invalid_file = false;
                        for (int i = 0; i < s3.size(); i++) {
                            if (s3[i] != '0' && s3[i] != '1') {
                                L.log(ERROR, "Invalid character occurs in the contents!");
                                std::getline(std::cin, cmd);
                                invalid_file = true;
                                break;
                            }
                        }
                        if (invalid_file) {
                            break;
                        }
                        fs->del(filename);
                        fs->insert(filename, s1.size() + + s3.size() + s2.size(), s1 + s3 + s2);
                        std::getline(std::cin, cmd);
                    }
                    else if (op_type == 'D') {
                        int l = 0;
                        int r = 0;
                        std::cin >> cmd;
                        bool invalid_cmd = false;
                        for (int i = 0; i < cmd.size(); i++) {
                            if (cmd[i] >= '0' && cmd[i] <= '9') {
                                l = l * 10 + cmd[i] - '0';
                            }
                            else {
                                invalid_cmd = true;
                                break;
                            }
                        }
                        if (invalid_cmd) {
                            L.log(ERROR, "Invalid format!");
                            std::getline(std::cin, cmd);
                            break;
                        }
                        std::cin >> cmd;
                        invalid_cmd = false;
                        for (int i = 0; i < cmd.size(); i++) {
                            if (cmd[i] >= '0' && cmd[i] <= '9') {
                                r = r * 10 + cmd[i] - '0';
                            }
                            else {
                                invalid_cmd = true;
                                break;
                            }
                        }
                        if (invalid_cmd) {
                            L.log(ERROR, "Invalid format!");
                            std::getline(std::cin, cmd);
                            break;
                        }
                        if (l > r || l < 0 || l >= fs->file_length[filename] || r < 0 || r >= fs->file_length[filename]) {
                            L.log(ERROR, "Delete positions are invalid!");
                            break;
                        }
                        s1 = content.substr(0, l);
                        s2 = content.substr(r + 1, fs->file_length[filename]);
                        fs->del(filename);
                        fs->insert(filename, s1.size() + s2.size(), s1 + s2);
                        std::getline(std::cin, cmd);
                    }
                    else if (op_type == 'E') {
                        break;
                    }
                }
                break;
            case 5:
                fs->defragmentation();
                break;
            case 6:
                return 0;
        }
    }
    return 0;
}