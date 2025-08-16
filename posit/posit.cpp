#include <iostream>
#include <bitset>
#include <cstdint>


class posit{

    public:
        uint8_t binaryValue;//binary value form
        uint8_t bitCount, es;//es = number of exponent value bitCount = number of bits 
        uint8_t sign;
        uint8_t twos;
        uint8_t absolate;
        uint8_t rc;
        uint8_t leadingCount;
        uint8_t k; // value of leadingcount in posit system
        uint8_t exp;
        uint8_t temp1;
        uint8_t fraction;
        uint8_t temp2;



        posit(uint8_t binary, uint8_t bits, uint8_t esValue):
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

        void display() const{
            std::cout<<"----------------------------------\n";
            std::cout<<"Number is: "<<std::bitset<8>(binaryValue)<<std::endl;
            std::cout<<"sign of number is: "<<std::bitset<8>(sign)<<" ("<<(int)sign<<")";
            std::cout<<"\ntwos with sign is: "<<std::bitset<8> (twos);
            std::cout<<"\nrc is: "<<std::bitset<8>(rc)<<" ("<<(int)rc<<")";
            std::cout<<"\nleadingcount is: "<<std::bitset<8>(leadingCount)<<" ("<<(int)leadingCount<<")";
            std::cout<<"\nk is: "<<std::bitset<8>(k)<<" ("<<(int)k<<")";
            std::cout<<"\nexponent is: "<<std::bitset<8>(exp);
            std::cout<<"\nfraction is: "<<std::bitset<8>(fraction);
            std::cout<<"\n----------------------------------\n";

        }






};

int main(){
    posit a(0b11111111,8,2);
    a.display();
}