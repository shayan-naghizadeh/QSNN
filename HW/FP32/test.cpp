#include <iostream> 
#include<vector>
#include<queue>
#include<functional>
#include "weights.h"
#include "SNN_MNIST.h"
#include <fstream>
#include <sstream>
// #include "TestDataset.h"


void load_dataset(
    std::vector<std::vector<std::vector<int>>>& spikes,
    std::vector<int>& labels,
    const std::string& spike_file = "spikes.txt",
    const std::string& label_file = "labels.txt"
) {
    std::ifstream lbl_in(label_file);
    if (!lbl_in.is_open()) {
        std::cerr << "Cannot open labels.txt\n";
        exit(1);
    }
    int lbl;
    while (lbl_in >> lbl) {
        labels.push_back(lbl);
    }

    std::ifstream spk_in(spike_file);
    if (!spk_in.is_open()) {
        std::cerr << "Cannot open spikes.txt\n";
        exit(1);
    }

    std::string line;
    std::vector<std::vector<int>> one_sample;
    while (std::getline(spk_in, line)) {
        if (line.rfind("# sample", 0) == 0) {
            if (!one_sample.empty()) {
                spikes.push_back(one_sample);
                one_sample.clear();
            }
        } else if (!line.empty()) {
            std::istringstream iss(line);
            std::vector<int> row;
            int x;
            while (iss >> x) row.push_back(x);
            one_sample.push_back(row);
        }
    }
    if (!one_sample.empty()) {
        spikes.push_back(one_sample);  
    }
}


int main() {
    std::vector<std::vector<std::vector<int>>> dataset_spikes;
    std::vector<int> labels;

    load_dataset(dataset_spikes, labels);

    int total_samples = 100;
    int correct = 0;

    std::cout << "--- Running SNN on Test Dataset ---\n";
    std::cout << "Total samples loaded: " << total_samples << "\n";

    for (int i = 0; i < total_samples; ++i) {
        const auto& sample_spikes = dataset_spikes[i];
        int true_label = labels[i];

        int predicted_label = run_snn(sample_spikes);

        if (predicted_label == true_label) {
            ++correct;
        }
    }

    double accuracy = (100.0 * correct) / total_samples;

    std::cout << "\n--- Evaluation Complete ---\n";
    std::cout << "Correct predictions: " << correct << "\n";
    std::cout << "Accuracy: " << accuracy << " %\n";

    return 0;
}
