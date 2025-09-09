// #include <iostream>
// #include <cstdint>
// #include "posit.h"
// using namespace std;

// int main() {
//     // uint8_t a = 200;
//     // uint8_t b = 100;
//     // uint8_t sum = a + b;  
//     // cout << sum << endl; 

//     // uint8_t c = a + b; 
//     // cout << (int)c << endl; 
//     posit a(0b00100101,8,2);
//     posit sum(0b00000000,8,2);
//     for (int i = 0 ; i < 25 ; i++){
//         sum = sum + a;
//     }
//     sum.display();
//     return 0;
// }


// #include <iostream>
// #include <cstdint>
// #include "posit.h"
// using namespace std;

// int main() {
//     posit a(0b00100101, 8, 2);
//     posit sum(0b00000000, 8, 2);
//     posit c(0b00000000, 8, 2);  // compensation term

//     for (int i = 0; i < 10000; i++) {
//         // Kahan summation steps
//         posit y = a - c;          // apply compensation
//         posit t = sum + y;        // temporary sum
//         c = (t - sum) - y;        // update compensation
//         sum = t;                  // update sum
//     }

//     sum.display();
//     return 0;
// }


// #include <iostream>
// #include <cstdint>
// #include "posit.h"
// using namespace std;

// int main() {
//     const int N = 10000;

//     posit a(0b00100101, 8, 2);
//     posit sum(0b00000000, 8, 2);
//     posit block = a;          // اندازه‌ی «بلوک» فعلی (a، بعد 2a، 4a، ...)

//     int n = N;
//     while (n > 0) {
//         if (n & 1)            // اگر بیت کم‌ارزش 1 است، این بلوک را به جمع اضافه کن
//             sum = sum + block;

//         block = block + block; // بلوک را دوبرابر کن (a→2a→4a→…)
//         n >>= 1;               // بیت بعدی
//     }

//     sum.display();
//     return 0;
// }


// #include <iostream>
// #include <cstdint>
// #include "posit.h"
// using namespace std;

// int main() {
//     const int N = 100;

//     posit a(0b00100101, 8, 2);
//     posit sum(0b00000000, 8, 2);
//     posit block = a;   

//     int n = N;
//     while (n > 0) {
//         if (n & 1)          
//             sum = sum + block;
//         block = block + block; 
//         n >>= 1;
//     }

//     sum.display();
//     return 0;
// }

#include <iostream>
#include <vector>
#include "posit.h"
using namespace std;

posit pairwise_sum(vector<posit> a) {
    if (a.empty()) return posit(0b00000000, 8, 2); // صفرِ posit
    while (a.size() > 1) {
        vector<posit> b;
        b.reserve((a.size() + 1) / 2);
        for (size_t i = 0; i + 1 < a.size(); i += 2) b.push_back(a[i] + a[i+1]);
        if (a.size() % 2) b.push_back(a.back());   // اگر فرد بود
        a.swap(b);
    }
    return a[0];
}

int main() {
    vector<posit> xs = {
        posit(0b00100101, 8, 2),
        posit(0b00100101, 8, 2),
        posit(0b00100101, 8, 2),
        posit(0b00100101, 8, 2),
        posit(0b00100101, 8, 2)
    };

    posit sum = pairwise_sum(xs);
    sum.display();
    return 0;
}
