#ifndef POSIT_H
#define POSIT_H
#include "ap_int.h"

#include <iostream>
#include <bitset>
#include <cstdint>
//#define ap_uint<6> ap_uint<6>

class posit6_0 {
public:
	ap_uint<6> binaryValue;   // binary value form
	ap_uint<6> bitCount, es;  // es = exponent size
	ap_uint<6> sign;
	ap_uint<6> twos;
	ap_uint<6> absolute;
	ap_uint<6> rc;
	ap_uint<6> leadingCount;
	ap_uint<6> k;
	ap_uint<6> exp;
	ap_uint<6> temp1;
	ap_uint<6> fraction;
	ap_uint<6> temp2;

    posit6_0(ap_uint<6> binary, ap_uint<6> bits, ap_uint<6> esValue);

    void display() const;

    static ap_uint<6> addition(posit6_0& num1, posit6_0& num2);
};

#endif
