#ifndef POSIT6_2_H
#define POSIT6_2_H

#include "ap_int.h"
#include <iostream>
#include <bitset>

class posit6_2 {
public:
    ap_uint<6> binaryValue;
    ap_uint<6> bitCount, es;
    ap_uint<1> sign;
    ap_uint<6> twos;
    ap_uint<6> absolute;
    ap_uint<1> rc;
    ap_uint<6> leadingCount;
    ap_int<6> k;     // regime exponent
    ap_uint<2> exp;  // 2-bit exponent
    ap_uint<6> fraction;

    // Constructor
    posit6_2(ap_uint<6> binary);

    // Debug print
    void display() const;

    // Addition
    static ap_uint<6> addition(posit6_2 &num1, posit6_2 &num2);
};

#endif // POSIT6_2_H
