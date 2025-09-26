// #ifndef WEIGHTS_H
// #define WEIGHTS_H

// #include <vector>
// #include "posit6_2.h"
// extern const posit6_2 fc1_weight[784][256];
// extern const posit6_2 fc2_weight[256][10];

// #endif



#ifndef WEIGHTS_H
#define WEIGHTS_H

#include "posit6_2.h"

extern const int num_input_neurons;
extern const int num_hidden_neurons;
extern const int num_output_neurons;

extern const posit6_2 fc1_weight[784][256];
extern const posit6_2 fc2_weight[256][10];

#endif
