#include "raid6.h"
#include "galois.h"
#include <cmath>
#include <iostream>

GaloisNumber Modulo;

int find_a_prime(int x) {
    for (int i = x; i < (x << 1); i++) {
        bool is_prime = true;
        for (int j = 2; j <= int(sqrt(i)); j++) {
            if (i % j == 0) {
                is_prime = false;
                break;
            }
        }
        if (is_prime) {
            return i;
        }
    }
}

std::string decompose(int x) {
    std::string result = "The modulo irreducible polynomial is ";
    bool head = true;
    for (int i = 31; i >= 0; i--) {
        if (x & (1 << i)) {
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

int main() {
    Logger L;
    int galois_degree, chunk_size;
    printf("For a Galois field GF(2^n), a maximum number of 2^n storage disks is allowed.\n");
    printf("Enter the degree 'n' of the Galois field (n < 32): ");
    scanf("%d", &galois_degree);
    chunk_size = galois_degree;
    L.log(INFO, "chunk_size = n.");
    L.log(INFO, "Finding a monic irreducible polynomial (x = 2) of degree n.");
    int modulo = find_a_prime((1 << galois_degree));
    Modulo = GaloisNumber(galois_degree + 1, modulo);
    for (int i = 0; i < Modulo.bit_vector.size(); i++) {
        printf("%d", Modulo.bit_vector[i]);
    }
    L.log(INFO, decompose(modulo));
    int num_of_disks;
    printf("Enter the number of storage disks (1 - %d). Note that two extra disks would be used for checksum: ", (1 << galois_degree));
    scanf("%d", &num_of_disks);
    int num_trunks;
    printf("Enter the maximum number of chunks a disk can store: ");
    scanf("%d", &num_trunks);
    double storage_size = (double)num_trunks * chunk_size * num_of_disks / 1024 / 1024;
    std::string msg = "The total storage of this file system is ";
    L.log(INFO, msg + std::to_string(storage_size) + "MB.");
    RAID6FileSystem* fs = new RAID6FileSystem(num_of_disks, chunk_size, num_trunks);
    while (1) {
        fs->check_and_fix();
        fs->list_all_files();
        printf("=====================\n");
        printf("1. Create a file;\n");
        printf("2. Delete a file;\n");
        printf("3. View a file;\n");
        printf("4. Modify a file;\n");
        printf("5. Exit.\n");
        printf("=====================\n");
        printf("Perform an operation: ");
        int op;
        std::string filename, content;
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
                printf("Enter the contents of the file (in binary format, e.g., '0001000101111'): ");
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
                printf("File content:\n");
                std::cout << fs->retrieve(filename) << std::endl;
                break;
            case 4:
                break;
            case 5:
                return 0;
        }
    }
    return 0;
}