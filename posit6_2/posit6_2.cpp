#include "posit6_2.h"

// Constructor
posit6_2::posit6_2(ap_uint<6> binary)
    : binaryValue(binary), bitCount(6), es(2) {

    // 1. Sign
    sign = (binaryValue >> (bitCount - 1)) ? 1 : 0;
    absolute = binaryValue & 0x1F;

    ap_uint<6> mask = (sign ? 0x1F : 0x00);
    twos = (sign ? ap_uint<6>(((mask ^ absolute) + sign) ^ 0x20) : binaryValue);

    // 2. Regime
    rc = ((twos & 0x10) >> (bitCount - 2)) ? 1 : 0;
    leadingCount = 0;
    for (int i = 4; i >= 0; --i) {
        if ((((twos & 0x1F) >> i) & 0x01) == rc) {
            leadingCount++;
        } else {
            break;
        }
    }

    if (rc == 0) {
        k = -((ap_int<6>)leadingCount);
    } else {
        k = (ap_int<6>)leadingCount - 1;
    }

    // 3. Exponent
    ap_uint<6> afterRegime = twos << (leadingCount + 2);
    exp = (afterRegime >> (6 - es)) & 0x3;

    // 4. Fraction
    ap_uint<6> fracShifted = afterRegime << es;
    int remaining = 6 - (leadingCount + 2 + es);
    if (remaining > 0) {
        fraction = fracShifted >> (6 - remaining);
    } else {
        fraction = 0;
    }
}

// Debug
void posit6_2::display() const {
    std::cout << "----------------------------------\n";
    std::cout << "Binary:   " << std::bitset<6>(binaryValue) << "\n";
    std::cout << "Sign:     " << (int)sign << "\n";
    std::cout << "Regime k: " << (int)k << "\n";
    std::cout << "Exponent: " << (int)exp << "\n";
    std::cout << "Fraction: " << (int)fraction << "\n";
    std::cout << "----------------------------------\n";
}

// Addition
ap_uint<6> posit6_2::addition(posit6_2 &num1, posit6_2 &num2) {
    // Handle NaR
    if ((num1.binaryValue == 0b100000) || (num2.binaryValue == 0b100000))
        return 0b100000;

    // Handle zero
    if (num1.twos == 0) return num2.binaryValue;
    if (num2.twos == 0) return num1.binaryValue;

    // Effective scale factor = k*2^es + exp
    ap_int<6> sf1 = (num1.k << num1.es) + (ap_int<6>)num1.exp;
    ap_int<6> sf2 = (num2.k << num2.es) + (ap_int<6>)num2.exp;

    // Fractions with hidden bit
    ap_uint<6> frac1 = (0x20 | ((num1.fraction & 0xF) << 1)) & 0x3F;
    ap_uint<6> frac2 = (0x20 | ((num2.fraction & 0xF) << 1)) & 0x3F;

    // Align
    posit6_2 *larger, *smaller;
    ap_int<6> sfL, sfS;
    ap_uint<6> fracL, fracS;

    if (sf1 >= sf2) {
        larger = &num1; smaller = &num2;
        sfL = sf1; sfS = sf2;
        fracL = frac1; fracS = frac2 >> (sfL - sfS);
    } else {
        larger = &num2; smaller = &num1;
        sfL = sf2; sfS = sf1;
        fracL = frac2; fracS = frac1 >> (sfL - sfS);
    }

    // Add/sub
    ap_int<7> resultFrac; // one more bit for safety
    if (num1.sign ^ num2.sign) {
        resultFrac = (ap_int<7>)fracL - (ap_int<7>)fracS;
    } else {
        resultFrac = (ap_int<7>)fracL + (ap_int<7>)fracS;
    }

    ap_int<6> resultSF = sfL;

    // Normalize
    while (resultFrac >= (1 << 6)) {
        resultFrac >>= 1;
        resultSF++;
    }
    while ((resultFrac & (1 << 5)) == 0 && resultFrac > 0) {
        resultFrac <<= 1;
        resultSF--;
    }

    // Rebuild fields
    ap_int<6> k = resultSF >> 2;
    ap_uint<2> exp = resultSF & 0x3;

    ap_uint<6> regime;
    if (k >= 0) {
        ap_uint<6> run = (ap_uint<6>)(k + 1);
        if (run >= 5) return (larger->sign ? 0b111111 : 0b011111);
        regime = ((1 << run) - 1) << (5 - run);
    } else {
        ap_uint<6> run = (ap_uint<6>)(-k);
        if (run >= 5) return (larger->sign ? 0b111111 : 0b000001);
        regime = 1 << (5 - run);
    }

    ap_uint<6> encoded = regime | (exp << 3) | ((resultFrac >> 2) & 0x7);

    // Apply sign
    if ((num1.sign ^ num2.sign) && (resultFrac < 0)) {
        encoded = ((encoded & 0x3F) ^ 0x3F) + 1;
    } else if (larger->sign == 1) {
        encoded = ((encoded & 0x3F) ^ 0x3F) + 1;
    }

    return encoded & 0x3F; // keep 6 bits
}
