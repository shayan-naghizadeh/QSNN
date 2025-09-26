#include "posit6_2.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include "weights62.h"


posit6_2 snn(posit6_2 a, posit6_2 b);



int main(){
	posit6_2 a(0b101011,6,2);//-4
	posit6_2 b(0b100110,6,2);//2

	posit6_2 c = snn(x1,x2);
	x1.display();
	x2.display();
	c.display();

}
