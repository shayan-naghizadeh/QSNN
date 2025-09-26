#include "ap_int.h"
#include "posit6_2.h"



posit6_2 posit6_2::operator+(const posit6_2& other) const {
    posit6_2 a = *this;
    posit6_2 b = other;
    uint8_t sumBits = posit6_2::addition(a, b);
    return posit6_2(sumBits, bitCount, es);
}

posit6_2 posit6_2::negate() const {
    uint8_t val = binaryValue;
    val = (~val) + 1;
    return posit6_2(val, bitCount, es);
}

posit6_2 posit6_2::operator-(const posit6_2& other) const {
    if (binaryValue == other.binaryValue) {
        return posit6_2(0x00, bitCount, es);
    }

    posit6_2 negOther = other.negate();
    return *this + negOther;
}

bool posit6_2::operator==(const posit6_2& other) const {
    return binaryValue == other.binaryValue;
}

bool posit6_2::operator!=(const posit6_2& other) const {
    return !(*this == other);
}

bool posit6_2::operator<(const posit6_2& other) const {
    if (sign != other.sign) {
        return sign > other.sign;
    }
    return binaryValue < other.binaryValue;
}

bool posit6_2::operator>(const posit6_2& other) const {
    return other < *this;
}

bool posit6_2::operator<=(const posit6_2& other) const {
    return !(*this > other);
}

bool posit6_2::operator>=(const posit6_2& other) const {
    return !(*this < other);
}

posit6_2& posit6_2::operator+=(const posit6_2& other) {
    *this = *this + other;
    return *this;
}

posit6_2& posit6_2::operator-=(const posit6_2& other) {
    *this = *this - other;
    return *this;
}
    posit6_2::posit6_2(ap_uint<6> binary, ap_uint<6> bits, ap_uint<6> esValue)
        : binaryValue(binary), bitCount(bits), es(esValue) {
        sign = ((binaryValue >> (bitCount - 1) ? 1 : 0));
        absolute = binaryValue & 0x1f; // number without sign before twos
        ap_uint<6> mask = (sign ? 0x1f : 0x00);
//        twos = (ap_uint<6>(sign) ? (((mask ^ absolute) + ap_uint<6> (sign)) ^ ap_uint<6> (0x20)) : binaryValue);
        twos = (sign ? ap_uint<6>(((mask ^ absolute) + sign) ^ 0x20) : ap_uint<6>(binaryValue)); //twos have sign +++

        rc = ((twos & 0x10) >> (bitCount - 2)) ? 1 : 0; // regime check
        leadingCount = 0;

        for (int i = 4; i >= 0; --i) {
            if ((((twos & 0x1f) >> i) & 0x01) == rc) {
                leadingCount++;
            } else {
                break;
            }
        }

        if (rc == 0) {
            k = -(leadingCount);
        } else {
            k = leadingCount - 1;
        }

        temp1 = twos << (2 + leadingCount); // after regime and termination bit
        temp2 = temp1 << es;                // after exponent = fraction

        int remaainig = 6 - (leadingCount + 2 + es);

        if (remaainig == 3) {
            fraction = temp2 >> 3;
            // exp = temp1 >> 6;
        } else if (remaainig == 2) {
            fraction = temp1 >> 4;
            // exp = temp1 >> 6;
        } else if (remaainig == 1) {
            fraction = temp2 >> 3;
             exp = temp1 >> 4;
        } else if (remaainig == 0) {
            fraction = 0;
             exp = temp1 >> 4;
        } else if (remaainig == -1) {
            fraction = 0;
             exp = temp1 >> 5;
        } else {
            fraction = 0;
             exp = 0;
        }
    }

    void posit6_2::display() const {
        std::cout << "----------------------------------\n";
        std::cout << "Number is: " << std::bitset<6>(binaryValue) << std::endl;
        std::cout << "sign of number is: " << std::bitset<6>(sign) << " (" << (int)sign << ")";
        std::cout << "\ntwos with sign is: " << std::bitset<6>(twos);
        std::cout << "\nrc is: " << std::bitset<6>(rc) << " (" << (int)rc << ")";
        std::cout << "\nleadingcount is: " << std::bitset<6>(leadingCount) << " (" << (int)leadingCount << ")";
        std::cout << "\nk is: " << std::bitset<6>(k) << " (" << (int)k << ")";
        std::cout << "\nexponent is: " << std::bitset<6>(exp);
        std::cout << "\nfraction is: " << std::bitset<6>(fraction);
        std::cout << "\nsf is: " << std::bitset<6>((k<<2)+exp);
        std::cout << "\n----------------------------------\n";
    }

    ap_uint<6> posit6_2::addition(posit6_2 &num1, posit6_2 &num2) {
        ap_uint<6> signXor = num1.sign ^ num2.sign;
        ap_uint<6> absNum1 = num1.twos & 0x1f;
        ap_uint<6> absNum2 = num2.twos & 0x1f;
        ap_uint<6> counter;
        const posit6_2 *larger;
        const posit6_2 *smaller;
        ap_uint<6> result = 0;
        ap_uint<6> LastResult = 0;
        ap_uint<6> tempp = 1;
        if ((num1.binaryValue == 0b100000) || (num2.binaryValue == 0b100000))
            return 0b100000;

        if (num1.twos == 0) {
            result = num2.binaryValue;
            return result;
        } else if (num2.twos == 0) {
            result = num1.binaryValue;
            return result;
        } else {
            if (absNum1 > absNum2) {
                larger = &num1;
                smaller = &num2;
            } else {
                larger = &num2;
                smaller = &num1;
            }


            if(signXor==1 && ((num1.twos&0x1f) == (num2.twos&0x1f))){
            	return 0b000000;
            }



            ap_uint<6> sf_larger = (larger->k<<2)+larger->exp;
            ap_uint<6> sf_smaller = (smaller->k<<2)+smaller->exp;

            ap_uint<6> diff = sf_larger - sf_smaller;

            ap_uint<6> larger_fraction = (0x20 + ((larger->fraction) << 2)) >> 2;
            ap_uint<6> smaller_fraction = ((0x20 + ((smaller->fraction) << 2)) >> 2) >> diff; // implicit 1

            ap_uint<6> sum;

            if (signXor == 1) {
                result = larger_fraction - smaller_fraction;
            } else {
                result = larger_fraction + smaller_fraction;
            }

//
            if ((result >> 4) == 1) { /// if we have 1 in fifth posit6_2ion
                result = result >> 1;
                result = result & 0x07;
                sf_larger++;
//                tempp = result;

                // std::cout<<std::endl<<"y "<<(int)result<<std::endl;
            } else {
                counter = 0;
                if ((result >> 3) == 0) { // if we don't have 1 in forth posit6_2ion
                    counter++;
                    for (int i = 2; i >= 0; i--) {
                        if (((result >> i) & 0x01) == 1) {
                            break;
                        } else {
                            counter++;
                        }
                    }
                } else {
                    result = result & 0x07; // if we have 1 in forth posit6_2ion
                }
                result = result << counter;
                sf_larger = sf_larger - counter;
                result = result & 0x07;
            }

            ap_uint<6> local_exp = sf_larger & 0x03;;

            if ((sf_larger >> 5) == 0) { // posit6_2ive regime
                sf_larger = sf_larger >> 2; // regime without exponent
                ap_uint<6> NOSFF = (3 - (6 - (sf_larger + 1 + 1 + 1 + 2))); // number of shift need for fraction to right
                ap_uint<6> RegF = (((1 << (sf_larger + 1)) - 1) << 1);      // standard form for regime

                if (sf_larger == 2) {
                    result = result >> NOSFF;
                    local_exp = local_exp >> 1;
                    LastResult = (RegF << (5 - (sf_larger + 2))) + local_exp;
                }else if(sf_larger == 1){
                    result = result >> NOSFF;
                    local_exp = local_exp ;
                    LastResult = (RegF << (5 - (sf_larger + 2))) + local_exp;

                } else if(sf_larger == 3){
                    LastResult = RegF;
                }else if(sf_larger == 0){
                    result = result >> NOSFF;
                    local_exp = local_exp<<1 ;
                    LastResult = (RegF << (5 - (sf_larger + 2))) + local_exp + result;
//                    LastResult = tempp;
                }
                else{
//                	if(larger->sign == ap_uint<6>(1)){
                	LastResult = ap_uint<6>(0b011111);}
//                	else {
//                		LastResult = ap_uint<6>(0b011111);
//                	}
//                }




            } else { /// negative regime
                sf_larger = (sf_larger >> 2) + ap_uint<6>(0b110000);     // regime without exponent and add for save sign
                sf_larger = (sf_larger ^ 0x3f) + 1;   // twos
                result = result >> (3 - (6 - (sf_larger + 1 + 2 + 1)));
                ap_uint<6> NOSFF = (3 - (6 - (sf_larger + 1 + 1 + 0))); // number of shift need for fraction to right
                ap_uint<6> RegF = (1 << (6 - (sf_larger + 2)));         // standard form for regime

                if (sf_larger == 3) {
                	local_exp = local_exp >> 1;
                	LastResult = RegF + local_exp;
                }
                else if(sf_larger == 2){
                	local_exp = local_exp;
                	LastResult = RegF + local_exp;

                }
                else if(sf_larger == 1) {
                	local_exp = local_exp<<1;
                	LastResult = RegF + local_exp + result;

                }else{
                	LastResult = RegF;
                }
            }

            if (larger->sign == 1) {
                LastResult = ((LastResult & 0x1f)^0x3f)+1;
            }

            return LastResult;
        }}
