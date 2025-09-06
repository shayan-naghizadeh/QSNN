#include <iostream>
#include <bitset>
#include <cstdint>
#include "posit.h"


#include <iostream>
#include <bitset>
#include <cstdint>
#include "posit.h"

posit posit::operator+(const posit& other) const {
    posit a = *this;
    posit b = other;
    uint8_t sumBits = posit::addition(a, b);
    return posit(sumBits, bitCount, es);
}

posit posit::negate() const {
    uint8_t val = binaryValue;
    val = (~val) + 1;
    return posit(val, bitCount, es);
}

posit posit::operator-(const posit& other) const {
    if (binaryValue == other.binaryValue) {
        return posit(0x00, bitCount, es); 
    }

    posit negOther = other.negate();
    return *this + negOther;
}

bool posit::operator==(const posit& other) const {
    return binaryValue == other.binaryValue;
}

bool posit::operator!=(const posit& other) const {
    return !(*this == other);
}

bool posit::operator<(const posit& other) const {
    if (sign != other.sign) {
        return sign > other.sign; 
    }
    return binaryValue < other.binaryValue;
}

bool posit::operator>(const posit& other) const {
    return other < *this;
}

bool posit::operator<=(const posit& other) const {
    return !(*this > other);
}

bool posit::operator>=(const posit& other) const {
    return !(*this < other);
}

posit& posit::operator+=(const posit& other) {
    *this = *this + other;
    return *this;
}

posit& posit::operator-=(const posit& other) {
    *this = *this - other;
    return *this;
}


posit::posit(uint8_t binary, uint8_t bits, uint8_t esValue):
binaryValue(binary),bitCount(bits),es(esValue){
    sign = ((binaryValue>>(bitCount-1) ? 1 : 0));
    absolate = binaryValue & 0x7f; //number without sign before twos 
    uint8_t mask = (sign ? 0x7f : 0x00);
    twos = (sign ? (((mask ^ absolate) + sign) ^ 0x80) : binaryValue);
    rc = ((twos & 0x40 ) >> (bitCount - 2)) ? 1 : 0; //regime check

    leadingCount = 0;

    for (int i = 6 ; i >=0 ; --i){
        if ((((twos & 0x7f) >> i) & 0x01) == rc ){
            leadingCount++;
            // std::cout<<"yes\n";
        }
        else{
            break;
        }
    }

    if (rc == 0){
        k = -(leadingCount);
    }else{
        k = leadingCount - 1;
    }

    temp1 = twos << (2 + leadingCount); // after regime and termination bit
    temp2 = temp1 << es ; // after exponent

    int remaainig = 8 - (leadingCount + 2 + es);

    if (remaainig == 3){
        fraction = temp2 >> 5;
        exp = temp1 >> 6;
    }else if(remaainig == 2){
        fraction = temp2 >> 6;
        exp = temp1 >> 6;
    }else if(remaainig == 1){
        fraction = temp2 >> 7;
        exp = temp1 >> 6;
    }else if (remaainig == 0){
        fraction = 0;
        exp = temp1 >> 6;
    }else if (remaainig == -1){
        fraction = 0;
        exp = temp1 >> 7;
    }else{
        fraction = 0;
        exp = 0;
    }
}

void posit::display() const{
    std::cout<<"----------------------------------\n";
    std::cout<<"Number is: "<<std::bitset<8>(binaryValue)<<std::endl;
    std::cout<<"sign of number is: "<<std::bitset<8>(sign)<<" ("<<(int)sign<<")";
    std::cout<<"\ntwos with sign is: "<<std::bitset<8> (twos);
    std::cout<<"\nrc is: "<<std::bitset<8>(rc)<<" ("<<(int)rc<<")";
    std::cout<<"\nleadingcount is: "<<std::bitset<8>(leadingCount)<<" ("<<(int)leadingCount<<")";
    std::cout<<"\nk is: "<<std::bitset<8>(k)<<" ("<<(int)k<<")";
    std::cout<<"\nexponent is: "<<std::bitset<8>(exp);
    std::cout<<"\nfraction is: "<<std::bitset<8>(fraction);
    std::cout<<"\nsf is: "<<std::bitset<8>(k<<2+es);
    std::cout<<"\n----------------------------------\n";
}

uint8_t posit::addition(posit& num1,posit& num2){
    uint8_t signXor = num1.sign ^ num2.sign;
    uint8_t absNum1 = num1.twos & 0x7f;
    uint8_t absNum2 = num2.twos & 0x7f;
    uint8_t counter;
    const posit* larger;
    const posit* smaller;
    uint8_t result = 0;
    uint8_t LastResult = 0;
    if ((num1.binaryValue == 0b10000000) || (num2.binaryValue == 0b10000000)) return 0b10000000;

    if ( num1.twos == 0){
        result = num2.binaryValue;
        return result;
    } else if( num2.twos == 0){
        result = num1.binaryValue;
        return result;
    } else{
        if(absNum1 > absNum2){
            larger = &num1;
            smaller = &num2;
        }else{
            larger = &num2;
            smaller = &num1;                  
        }

        uint8_t sf_larger = (larger->k << 2) + larger->exp;
        uint8_t sf_smaller = (smaller->k <<2) + smaller->exp;

        uint8_t diff = sf_larger - sf_smaller;

        uint8_t larger_fraction = (0x80 + ((larger->fraction) << 4)) >> 4;
        uint8_t smaller_fraction = ((0x80 + ((smaller->fraction) << 4)) >> 4) >> diff;      //implicit 1       
        
        uint8_t sum;

        if (signXor == 1){
            result = larger_fraction - smaller_fraction;
        }else{
            result = larger_fraction + smaller_fraction;
        }

        if ((result >> 4) == 1){ ///if we have 1 in fifth position
            result = result >> 1;
            result = result & 0x07;
            sf_larger++;
            std::cout<<std::endl<<"y "<<(int)result<<std::endl;


        }else{
            counter = 0;
            if((result >> 3) == 0){ //if we don't have 1 in forth position
                counter++;
                for(int i=2 ; i>=0 ; i--){
                    if(((result >> i) & 0x01) == 1){
                        break;
                    }else{
                        counter++;
                    }
                }

            }else{
                result = result & 0x07; //if we have 1 in forth position
            }

            result = result << counter;
            sf_larger = sf_larger - counter;
            result = result & 0x07;
        }

        uint8_t local_exp = sf_larger & 0x03;
        std::cout<<std::endl<<"local exponent "<<(int)local_exp<<std::endl;

        if ((sf_larger >> 7) == 0) { //positive regime
            sf_larger = sf_larger >> 2; //regime without exponent
            uint8_t NOSFF = (3 - (8 - (sf_larger + 1 + 1 + 1 + 2))); //number of shift need for fraction to right
            uint8_t RegF = (((1 << (sf_larger + 1)) - 1) << 1);  //standard form for regime
            std::cout<<std::endl<<"local exponent "<<(int)NOSFF<<std::endl;
            std::cout<<std::endl<<"local exponent "<<(int)RegF<<std::endl;
            if (sf_larger <= 3) {
                result = result >> NOSFF;
                if (NOSFF == 0) {
                    local_exp = local_exp << 3;
                } else if (NOSFF == 1) {
                            local_exp = local_exp << 2;
                } else if (NOSFF == 2) {
                            local_exp = local_exp << 1;
                } else if (NOSFF == 3) {
                            local_exp = local_exp << 0;
                }

                std::cout<<std::endl<<"t "<<(int)result<<std::endl;
                LastResult = ( RegF << (7 - (sf_larger + 2))) + local_exp + result;
                std::cout<<std::endl<<"local result "<<(int)result<<std::endl;
                std::cout<<std::endl<<"local result "<<(int)local_exp<<std::endl;




            } else {
                     if (sf_larger == 4) {
                        LastResult = ( RegF << (7 - (sf_larger + 2))) + (local_exp >> 1);
                    } else if(sf_larger == 5) {
                        LastResult = ( RegF << (7 - (sf_larger + 2)));
                     } else{
                        LastResult = RegF;
                        }
                    }
        } else { ///negative regime
            sf_larger = (sf_larger >> 2) + 0xc0; //regime without exponent and add for save sign 
            sf_larger = (sf_larger ^ 0xff) + 1; //twos
            result = result >> (3 - (8 - (sf_larger + 1 + 2 + 1)));
            if (sf_larger <= 4){
                uint8_t NOSFF = (3 - (8 - (sf_larger + 1 + 1 + 2))); //number of shift need for fraction to right
                uint8_t RegF = (1 << (8 - (sf_larger + 2)));  //standard form for regime
                if ( NOSFF == 0 ){
                    local_exp = local_exp << 3;
                } else if (NOSFF == 1 ){
                    local_exp = local_exp << 2;
                } else if (NOSFF == 2){
                    local_exp = local_exp << 1;
                } else if (NOSFF == 3 ){
                    local_exp = local_exp << 0;
                }
                LastResult = RegF + local_exp + result;
            } else {
                if ( sf_larger == 5){
                    LastResult = (1 << (8 - (sf_larger + 2))) + (local_exp >> 1);
                } else if ( sf_larger == 6 ){
                    LastResult = (1 << (8 - (sf_larger + 2)));
                }
            }
        }

        if (larger->sign == 1) {
            LastResult = ((LastResult ^ 0xff) + 1) + 0x00;
        }
    }
    return LastResult;
}