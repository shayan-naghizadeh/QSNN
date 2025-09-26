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
    ap_uint<6> temp1;
    ap_uint<6> temp2;



    posit6_2() : binaryValue(0), bitCount(6), es(2), sign(0), twos(0),
                 absolute(0), rc(0), leadingCount(0), k(0), exp(0),
                 fraction(0), temp1(0), temp2(0) {}

    // Constructor
    posit6_2(ap_uint<6> binary, ap_uint<6> bits, ap_uint<6> esValue); // full

    // Debug print
    void display() const;

    // Addition
    static ap_uint<6> addition(posit6_2 &num1, posit6_2 &num2);

    posit6_2 operator+(const posit6_2& other) const;

    posit6_2 negate() const;

    posit6_2 operator-(const posit6_2& other) const;

    bool operator==(const posit6_2& other) const;

    bool operator!=(const posit6_2& other) const;

    bool operator<(const posit6_2& other) const;

    bool operator>(const posit6_2& other) const;

    posit6_2& operator+=(const posit6_2& other);

    posit6_2& operator-=(const posit6_2& other);

    bool operator<=(const posit6_2& other) const;

    bool operator>=(const posit6_2& other) const;


};

#endif // posit6_26_2_H
