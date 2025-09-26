#include "posit6_2.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include "ap_int.h"


posit6_2 snn(posit6_2 a,posit6_2 b){

//	posit6_0 c(ap_uint<6>(0b000000), ap_uint<6>(6), ap_uint<6>(0));
//	posit6_0 d(ap_uint<6>(0b000000), ap_uint<6>(6), ap_uint<6>(0));
	ap_uint<6> result = posit6_2::addition(a, b);
	posit6_2 c(result, ap_uint<6>(6), ap_uint<6>(2));

	return c;





};
