#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>     
//#include "weights.h"
int SNN(int spike_input[50][784]);

#define NUM_SAMPLES 100
#define TIME_STEPS 50
#define NUM_INPUT_NEURONS 784

// Fixed-size arrays
int dataset_spikes[NUM_SAMPLES][TIME_STEPS][NUM_INPUT_NEURONS] = {{{0}}};
int labels[NUM_SAMPLES] = {0};

// Load spike and label data from text files
void load_dataset(
    int spikes[NUM_SAMPLES][TIME_STEPS][NUM_INPUT_NEURONS],
    int labels[NUM_SAMPLES],
    const std::string& spike_file = "spikes.txt",
    const std::string& label_file = "labels.txt"
) {
    std::ifstream lbl_in(label_file);
    if (!lbl_in.is_open()) {
        std::cerr << "Cannot open labels.txt\n";
        exit(1);
    }

    int lbl, idx = 0;
    while (lbl_in >> lbl && idx < NUM_SAMPLES) {
        labels[idx++] = lbl;
    }

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
            if (neuron_idx != NUM_INPUT_NEURONS) {
                std::cerr << "Spike line has " << neuron_idx
                          << " values instead of 784 at sample "
                          << sample_idx << ", timestep " << time_idx << "\n";
                exit(1);
            }

            ++time_idx;
            if (time_idx == TIME_STEPS) ++sample_idx;
        }
    }
}

// Main function
int main() {
    load_dataset(dataset_spikes, labels);

    const int total_samples = NUM_SAMPLES;
    int correct = 0;

    std::cout << "--- Running SNN on Test Dataset ---\n";
    std::cout << "Total samples to evaluate: " << total_samples << "\n";

    for (int i = 0; i < total_samples; ++i) {
        int (*sample_spikes)[NUM_INPUT_NEURONS] = dataset_spikes[i];
        int true_label = labels[i];

        int predicted_label = SNN(sample_spikes);

        std::cout << "Sample " << i << ": predicted = " << predicted_label
                  << ", true = " << true_label << "\n";

        if (predicted_label == true_label) {
            ++correct;
        }
    }

    double accuracy = (100.0 * correct) / total_samples;

    std::cout << "\n--- Evaluation Complete ---\n";
    std::cout << "Correct predictions: " << correct << " out of " << total_samples << "\n";
    std::cout << "Accuracy: " << accuracy << " %\n";

    return 0;
}
