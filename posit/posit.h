#ifndef POSIT_H
#define POSIT_H

#include <iostream>
#include <bitset>
#include <cstdint>

class posit {
public:
    uint8_t binaryValue;   // binary value form
    uint8_t bitCount, es;  // es = exponent size
    uint8_t sign;
    uint8_t twos;
    uint8_t absolate;
    uint8_t rc;
    uint8_t leadingCount;
    uint8_t k;
    uint8_t exp;
    uint8_t temp1;
    uint8_t fraction;
    uint8_t temp2;

    posit(uint8_t binary, uint8_t bits, uint8_t esValue);

    void display() const;

    static uint8_t addition(posit& num1, posit& num2);
    
    posit operator+(const posit& other) const;

    posit negate() const;          

    posit operator-(const posit& other) const; 

    bool operator==(const posit& other) const;

    bool operator!=(const posit& other) const;

    bool operator<(const posit& other) const;
    
    bool operator>(const posit& other) const;


};

#endif
