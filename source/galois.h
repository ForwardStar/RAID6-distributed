#ifndef GALOIS
#define GALOIS
#include <vector>
#include <iostream>

class GaloisNumber {
    public:
        bool operator == (const GaloisNumber &next);
        GaloisNumber operator ^ (int x);
        GaloisNumber operator * (const GaloisNumber &next);
        GaloisNumber operator + (const GaloisNumber &next);

        std::vector<int> bit_vector;

        // n: Galois degree;
        GaloisNumber(int n, unsigned int num);
        GaloisNumber(std::vector<int> a);
        GaloisNumber() {};
};

#endif