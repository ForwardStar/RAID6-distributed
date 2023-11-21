#include "galois.h"

extern GaloisNumber Modulo;

bool GaloisNumber::operator == (const GaloisNumber &next) {
    for (int i = 0; i < bit_vector.size(); i++) {
        if (bit_vector[i] != next.bit_vector[i]) {
            return false;
        }
    }
    return true;
}

GaloisNumber GaloisNumber::operator ^ (int x) {
    GaloisNumber t = GaloisNumber(bit_vector.size(), 1);
    GaloisNumber a = GaloisNumber(bit_vector);
    while (x) {
        if (x & 1) {
            t = t * a;
        }
        a = a * a;
        x >>= 1;
    }
    return t;
}

GaloisNumber GaloisNumber::operator * (const GaloisNumber &next) {
    std::vector<int> new_vector;
    new_vector.assign(bit_vector.size() << 1, 0);
    for (int i = 0; i < bit_vector.size(); i++) {
        for (int j = 0; j < next.bit_vector.size(); j++) {
            if (bit_vector[i] == 1 && next.bit_vector[j] == 1) {
                new_vector[i + j] ^= 1;
            }
        }
    }
    for (int i = new_vector.size() - 1; i >= 0; i--) {
        if (i >= Modulo.bit_vector.size() - 1 && new_vector[i] == 1) {
            for (int j = i; j > i - int(Modulo.bit_vector.size()); j--) {
                new_vector[j] ^= Modulo.bit_vector[Modulo.bit_vector.size() - (i - j) - 1];
            }
        }
    }
    std::vector<int> new_vector_shrinked;
    new_vector_shrinked.resize(bit_vector.size());
    for (int i = 0; i < bit_vector.size(); i++) {
        new_vector_shrinked[i] = new_vector[i];
    }
    return GaloisNumber(new_vector_shrinked);
}

GaloisNumber GaloisNumber::operator + (const GaloisNumber &next) {
    std::vector<int> new_vector;
    for (int i = 0; i < bit_vector.size(); i++) {
        new_vector.push_back(bit_vector[i] ^ next.bit_vector[i]);
    }
    return GaloisNumber(new_vector);
}

GaloisNumber::GaloisNumber(int n, unsigned int num) {
    bit_vector.resize(n);
    for (int i = 0; i < n; i++) {
        if (num & (1 << i)) {
            bit_vector[i] = 1;
        }
        else {
            bit_vector[i] = 0;
        }
    }
}

GaloisNumber::GaloisNumber(std::vector<int> a) {
    bit_vector = a;
}