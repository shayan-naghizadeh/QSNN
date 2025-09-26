//#include "posit6_2.h"
//#include <iostream>
//#include <fstream>
//#include <sstream>
//#include <string>
//#include <cstdlib>
//#include "ap_int.h"
//
//posit6_2 snn(posit6_2 a, posit6_2 b) {
//    ap_uint<6> size  = ap_uint<6>(0b000110);
//    ap_uint<6> expo  = ap_uint<6>(0b000010);
//
//    ap_uint<6> result = posit6_2::addition(a, b);
//    posit6_2 c{result, size, expo};
//
//    return c;
//}




#include "posit6_2.h"

posit6_2 snn(posit6_2 a, posit6_2 b) {
    ap_uint<6> size  = ap_uint<6>(0b000110);
    ap_uint<6> expo  = ap_uint<6>(0b000010);

    ap_uint<6> result = posit6_2::addition(a, b);


    posit6_2 c{result, size, expo};

    return c;
}
