#include "posit6_2.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>


int snn(
    int spike_input[50][784],
    const posit6_2 fc1_weight[784][256],
    const posit6_2 fc2_weight[256][10],
    int output_spike_counts[10],
    int input_spike_counts[784],
    int hidden_spike_counts[256],
    posit6_2 hidden_potentials[256]
);

#define NUM_SAMPLES 50
#define TIME_STEPS 50
#define NUM_INPUT_NEURONS 784

int dataset_spikes[NUM_SAMPLES][TIME_STEPS][NUM_INPUT_NEURONS] = {{{0}}};
int labels[NUM_SAMPLES] = {0};

void init_weights(posit6_2 fc1_weight[784][256],
                  posit6_2 fc2_weight[256][10]) {
    for (int i = 0; i < 784; i++) {
        for (int j = 0; j < 256; j++) {
            fc1_weight[i][j] = posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2));
        }
    }
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 10; j++) {
            fc2_weight[i][j] = posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2));
        }
    }
}

void load_weights(posit6_2 fc1_weight[784][256],
                  posit6_2 fc2_weight[256][10],
                  const std::string& fc1_file = "fc1_weights.txt",
                  const std::string& fc2_file = "fc2_weights.txt") {
    std::ifstream f1(fc1_file);
    if (!f1.is_open()) {
        std::cerr << "Cannot open " << fc1_file << "\n";
        exit(1);
    }
    for (int i = 0; i < 784; i++) {
        for (int j = 0; j < 256; j++) {
            std::string bits;
            f1 >> bits;
            ap_uint<6> val(std::stoi(bits, nullptr, 2));
            fc1_weight[i][j] = posit6_2(val, 6, 2);
        }
    }
    f1.close();

    std::ifstream f2(fc2_file);
    if (!f2.is_open()) {
        std::cerr << "Cannot open " << fc2_file << "\n";
        exit(1);
    }
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 10; j++) {
            std::string bits;
            f2 >> bits;
            ap_uint<6> val(std::stoi(bits, nullptr, 2));
            fc2_weight[i][j] = posit6_2(val, 6, 2);
        }
    }
    f2.close();
}

void load_dataset(
    int spikes[NUM_SAMPLES][TIME_STEPS][NUM_INPUT_NEURONS],
    int labels[NUM_SAMPLES],
    const std::string& spike_file = "spikes.txt",
    const std::string& label_file = "labels.txt") {

    std::ifstream lbl_in(label_file);
    if (!lbl_in.is_open()) {
        std::cerr << "Cannot open labels.txt\n";
        exit(1);
    }
    int lbl, idx = 0;
    while (lbl_in >> lbl && idx < NUM_SAMPLES) {
        labels[idx++] = lbl;
    }
    lbl_in.close();

    std::ifstream spk_in(spike_file);
    if (!spk_in.is_open()) {
        std::cerr << "Cannot open spikes.txt\n";
        exit(1);
    }
    std::string line;
    int sample_idx = 0, time_idx = 0;
    while (std::getline(spk_in, line)) {
        if (line.rfind("# sample", 0) == 0) {
            if (sample_idx >= NUM_SAMPLES) break;
            time_idx = 0;
            continue;
        }
        if (!line.empty()) {
            std::istringstream iss(line);
            int spike_value, neuron_idx = 0;
            while (iss >> spike_value && neuron_idx < NUM_INPUT_NEURONS) {
                spikes[sample_idx][time_idx][neuron_idx++] = spike_value;
            }
            ++time_idx;
            if (time_idx == TIME_STEPS) ++sample_idx;
        }
    }
    spk_in.close();
}
//
//int main() {
//    load_dataset(dataset_spikes, labels);
//
//    static posit6_2 fc1_weight[784][256];
//    static posit6_2 fc2_weight[256][10];
//
//    init_weights(fc1_weight, fc2_weight);
//    load_weights(fc1_weight, fc2_weight);
//
//    std::cout << "fc1[0][0] raw bits = "
//              << std::bitset<6>(fc1_weight[0][0].binaryValue) << "\n";
//    fc1_weight[0][0].display();
//
//    const int total_samples = NUM_SAMPLES;
//    int input[784] = {0};
//    int hidden[256] = {0};
//    int output[10] = {0};
//    posit6_2 memhidden[256];
//
//    std::cout << "--- Running SNN on Test Dataset ---\n";
//    std::cout << "Total samples to evaluate: " << total_samples << "\n";
//
//    for (int i = 0; i < total_samples; ++i) {
//        int (*sample_spikes)[NUM_INPUT_NEURONS] = dataset_spikes[i];
//        snn(sample_spikes, fc1_weight, fc2_weight, output,input,hidden,memhidden);
//
//        for (int k = 0; k < 10; ++k) {
//            std::cout << output[k] << std::endl;
//        }
//        std::cout << "--- ---------------------------- ---\n";
//
//        for (int k = 0; k < 784; ++k) {
//            std::cout << input[k] << std::endl;
//        }
//        std::cout << "--- ---------------------------- ---\n";
//        for (int k = 0; k < 256; ++k) {
//            std::cout << hidden[k] << std::endl;
//        }
//        std::cout << "--- ---------------------------- ---\n";
//        for (int k = 0; k < 256; ++k) {
//            std::cout << "--- ---------------------------- ---\n";
//
//             memhidden[k].display();
//
//            std::cout << "--- ---------------------------- ---\n";
//
//        }
//    }
//
//    return 0;
//}



int main() {
    load_dataset(dataset_spikes, labels);

        static posit6_2 fc1_weight[784][256];
        static posit6_2 fc2_weight[256][10];

        init_weights(fc1_weight, fc2_weight);
        load_weights(fc1_weight, fc2_weight);

    const int total_samples = NUM_SAMPLES;
    int correct = 0;
    int local_total = 0;
        int input[784] = {0};
        int hidden[256] = {0};
        int output[10] = {0};
        posit6_2 memhidden[256];
    std::cout << "--- Running SNN on Test Dataset ---\n";
    std::cout << "Total samples to evaluate: " << total_samples << "\n";

    for (int i = 0; i < total_samples; ++i) {
        int (*sample_spikes)[NUM_INPUT_NEURONS] = dataset_spikes[i];
        int true_label = labels[i];

        int predicted_label = snn(sample_spikes, fc1_weight, fc2_weight, output,input,hidden,memhidden);

        std::cout << "Sample " << i << ": predicted = " << predicted_label
                  << ", true = " << true_label << "\n";

        if (predicted_label == true_label) {
            ++correct;
        }

        if(local_total > 0 && (local_total % 100 == 0) ){
            std::cout<<"==============================="<<std::endl;
            std::cout<<"Number of Correct: "<<correct<<"  Total: "<<local_total<<std::endl;
            std::cout<<"==============================="<<std::endl;
        }
        local_total++;
    }

    double accuracy = (100.0 * correct) / total_samples;

    std::cout << "\n--- Evaluation Complete ---\n";
    std::cout << "Correct predictions: " << correct << " out of " << total_samples << "\n";
    std::cout << "Accuracy: " << accuracy << " %\n";

    return 0;
}

