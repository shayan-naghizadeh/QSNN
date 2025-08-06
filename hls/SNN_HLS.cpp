#include <iostream>
// #include "spikes.h"
#include "SNN_HLS.h"
#include "weights.h"

class LIFNeuron {
public:
    float membrane_potential;
    bool has_fired_in_current_step;
    int last_update_time;
    float threshold_voltage;
    float leak_reversal_potential;
    float fixed_leak_amount_per_step;

    enum ResetMechanism { SUBTRACT_THRESHOLD, RESET_TO_ZERO };
    ResetMechanism current_reset_mechanism;

    LIFNeuron(float v_th = 0.8f, float e_l = 0.0f, float fixed_leak_amt = 0.0f, ResetMechanism rest_mech = RESET_TO_ZERO) :
        membrane_potential(0.0f),
        has_fired_in_current_step(false),
        last_update_time(0),
        threshold_voltage(v_th),
        leak_reversal_potential(e_l),
        fixed_leak_amount_per_step(fixed_leak_amt),
        current_reset_mechanism(rest_mech) {}

    bool update_membrane_and_check_spike(float input_current, int current_event_time) {
        has_fired_in_current_step = false;

        // int dt = current_event_time - last_update_time;
        // int num_leak_steps = dt;

        // if (num_leak_steps < 0) num_leak_steps = 0;

        // for (int i = 0; i < num_leak_steps; ++i) {
        //     if (membrane_potential > leak_reversal_potential) {
        //         membrane_potential -= fixed_leak_amount_per_step;
        //         if (membrane_potential < leak_reversal_potential) {
        //             membrane_potential = leak_reversal_potential;
        //         }
        //     }
        //     else if (membrane_potential < leak_reversal_potential) {
        //         membrane_potential += fixed_leak_amount_per_step;
        //         if (membrane_potential > leak_reversal_potential) {
        //             membrane_potential = leak_reversal_potential;
        //         }
        //     }
        // }


        membrane_potential += input_current;
        if (membrane_potential < 0)membrane_potential =0;

        if (membrane_potential >= threshold_voltage) {
            has_fired_in_current_step = true;
            if (current_reset_mechanism == SUBTRACT_THRESHOLD) {
                membrane_potential -= threshold_voltage;
            }
            else {
                membrane_potential = 0.0f;
            }
        }

        last_update_time = current_event_time;
        return has_fired_in_current_step;
    }

    float get_membrane_potential() const {
        return membrane_potential;
    }
};

enum EventType {
    INPUT_SPIKE_EVENT,
    NEURON_SPIKE_EVENT
};

struct SpikeEvent {
    int scheduled_time;
    EventType Type;
    int source_neuron_layer_id;
    int source_neuron_id;
    int target_neuron_layer_id;
    int target_neuron_id;
    float effective_current;

    SpikeEvent() :
        scheduled_time(0),
        Type(INPUT_SPIKE_EVENT),
        source_neuron_layer_id(-1),
        source_neuron_id(-1),
        target_neuron_layer_id(-1),
        target_neuron_id(-1),
        effective_current(0.0f) {}

    SpikeEvent(int s_time, EventType e_type, int s_la_id, int s_id, int t_la_id, int t_id, float e_current) :
        scheduled_time(s_time),
        Type(e_type),
        source_neuron_layer_id(s_la_id),
        source_neuron_id(s_id),
        target_neuron_layer_id(t_la_id),
        target_neuron_id(t_id),
        effective_current(e_current) {}
};

const int MAX_EVENT_IN_HEAP = 100000;

class MinHeap {
private:
    SpikeEvent heap_array[MAX_EVENT_IN_HEAP];
    int current_size;

    int parent(int i) { return (i - 1) / 2; }
    int left_child(int i) { return (2 * i + 1); }
    int right_child(int i) { return (2 * i + 2); }

    bool is_less_priority(const SpikeEvent& a, const SpikeEvent& b) {
        if (a.scheduled_time != b.scheduled_time) {
            return a.scheduled_time > b.scheduled_time;
        }
        if (a.target_neuron_layer_id != b.target_neuron_layer_id) {
            return a.target_neuron_layer_id < b.target_neuron_layer_id;
        }
        return a.target_neuron_id > b.target_neuron_id;
    }

    void heapify_up(int i) {
        while (i != 0 && is_less_priority(heap_array[parent(i)], heap_array[i])) {
            std::swap(heap_array[i], heap_array[parent(i)]);
            i = parent(i);
        }
    }

    void heapify_down(int i) {
        int highest_priority_idx = i;
        int l = left_child(i);
        int r = right_child(i);

        if (l < current_size && is_less_priority(heap_array[highest_priority_idx], heap_array[l])) {
            highest_priority_idx = l;
        }

        if (r < current_size && is_less_priority(heap_array[highest_priority_idx], heap_array[r])) {
            highest_priority_idx = r;
        }

        if (highest_priority_idx != i) {
            std::swap(heap_array[i], heap_array[highest_priority_idx]);
            heapify_down(highest_priority_idx);
        }
    }

public:

    int heap_size;

    MinHeap() : current_size(0), heap_size(0) {}

    bool is_empty() const { return current_size == 0; }
    bool is_full() const { return current_size == MAX_EVENT_IN_HEAP; }
    int size() const { return current_size; }

    void push(SpikeEvent event) {
        if (is_full()) {
            std::cerr << "Error: Heap is full" << std::endl;
            return;
        }
        heap_array[current_size] = event;
        current_size++;
        heap_size++;
        heapify_up(current_size - 1);
    }

    SpikeEvent top() {
        if (is_empty()) {
            std::cerr << "Error: Heap is empty" << std::endl;
            return SpikeEvent();
        }
        return heap_array[0];
    }

    SpikeEvent pop() {
        if (is_empty()) {
            std::cerr << "Error: Heap is empty" << std::endl;
            return SpikeEvent();
        }
        SpikeEvent root = heap_array[0];
        current_size--;
        heap_size--;
        heap_array[0] = heap_array[current_size];
        heapify_down(0);
        return root;
    }


    void clear() {
    current_size = 0;
    heap_size = 0;
}
};

const int NUM_INPUT_NEURONS = 784;
const int NUM_HIDDEN_NEURONS = 256;
const int NUM_OUTPUT_NEURONS = 10;

int snn(int spike_input[50][784]) {
    float v_threshold = 0.8f;
    float e_leak = 0.0f;
    float fixed_leak_per_step = 0.0f; // Enable leaky behavior
    LIFNeuron::ResetMechanism reset_mech = LIFNeuron::RESET_TO_ZERO;

    static LIFNeuron all_layers_input[NUM_INPUT_NEURONS];
    static LIFNeuron all_layers_hidden[NUM_HIDDEN_NEURONS];
    static LIFNeuron all_layers_output[NUM_OUTPUT_NEURONS];
    static int output_spike_counts[NUM_OUTPUT_NEURONS] = {0};

    // std::cout << "Initializing neurons..." << std::endl;
    for (int i = 0; i < NUM_INPUT_NEURONS; ++i) {
        all_layers_input[i] = LIFNeuron(v_threshold, e_leak, fixed_leak_per_step, reset_mech);
    }
    for (int i = 0; i < NUM_HIDDEN_NEURONS; ++i) {
        all_layers_hidden[i] = LIFNeuron(v_threshold, e_leak, fixed_leak_per_step, reset_mech);
    }
    for (int i = 0; i < NUM_OUTPUT_NEURONS; ++i) {
        all_layers_output[i] = LIFNeuron(v_threshold, e_leak, fixed_leak_per_step, reset_mech);
        output_spike_counts[i] = 0;
    }
    // std::cout << "Neurons initialized." << std::endl;

    int MAX_SIMULATION_TIME = 50;
    static MinHeap event_queue;
    event_queue.clear();

    // std::cout << "Loading initial input spikes into event queue..." << std::endl;
    int initial_spikes_pushed = 0;
    for (int t = 0; t < 50; ++t) {
        for (int neuron_idx = 0; neuron_idx < NUM_INPUT_NEURONS; ++neuron_idx) {
            if (spike_input[t][neuron_idx] == 1) {
                event_queue.push(SpikeEvent(t, INPUT_SPIKE_EVENT, -1, -1, 0, neuron_idx, 1.0f)); // Scaled input current
                initial_spikes_pushed++;
            }
        }
    }
    // std::cout << "Finished loading initial spikes. Total spikes pushed: " << initial_spikes_pushed << std::endl;
    // std::cout << "Heap size after initial load: " << event_queue.heap_size << std::endl;

    if (!event_queue.is_empty()) {
        // std::cout << "First event in queue: time=" << event_queue.top().scheduled_time
        //           << ", type=" << (event_queue.top().Type == INPUT_SPIKE_EVENT ? "INPUT" : "NEURON")
        //           << ", target_neuron_id=" << event_queue.top().target_neuron_id << std::endl;
    }

    int current_simulation_time = 0;
    // std::cout << "\nStarting event-driven simulation..." << std::endl;
    long long events_processed = 0;

    while (!event_queue.is_empty() && current_simulation_time <= MAX_SIMULATION_TIME) {
        SpikeEvent current_event = event_queue.pop();
        events_processed++;
        current_simulation_time = current_event.scheduled_time;

        if (current_simulation_time > MAX_SIMULATION_TIME) {
            // std::cout << "Breaking simulation: current_simulation_time (" << current_simulation_time
            //           << ") > MAX_SIMULATION_TIME (" << MAX_SIMULATION_TIME << ")" << std::endl;
            break;
        }

        if (current_event.Type == INPUT_SPIKE_EVENT) {
            LIFNeuron& target_neuron = all_layers_input[current_event.target_neuron_id];
            float initial_mp = target_neuron.membrane_potential;
            bool fired = target_neuron.update_membrane_and_check_spike(current_event.effective_current, current_simulation_time);

            // std::cout << "  Input Neuron " << current_event.target_neuron_id
            //           << ": MP_before=" << initial_mp << ", Input=" << current_event.effective_current
            //           << ", MP_after=" << target_neuron.membrane_potential << ", Fired=" << fired << std::endl;

            if (fired) {
                int source_layer_idx = 0;
                int next_layer_idx = source_layer_idx + 1;
                if (next_layer_idx < 3) {
                    for (int j = 0; j < NUM_HIDDEN_NEURONS; ++j) {
                        float effective_input_to_next_neuron = fc1_weight[current_event.target_neuron_id][j];
                        event_queue.push(SpikeEvent(current_simulation_time + 1, NEURON_SPIKE_EVENT, source_layer_idx, current_event.target_neuron_id, next_layer_idx, j, effective_input_to_next_neuron));
                    }
                }
            }
        }
        else if (current_event.Type == NEURON_SPIKE_EVENT) {
            LIFNeuron* target_neuron_ptr = nullptr;
            if (current_event.target_neuron_layer_id == 1) {
                target_neuron_ptr = &all_layers_hidden[current_event.target_neuron_id];
            }
            else if (current_event.target_neuron_layer_id == 2) {
                target_neuron_ptr = &all_layers_output[current_event.target_neuron_id];
            }
            else {
                std::cerr << "Error: Unexpected target_neuron_layer_id: " << current_event.target_neuron_layer_id << std::endl;
                continue;
            }
            LIFNeuron& target_neuron = *target_neuron_ptr;

            float initial_mp = target_neuron.membrane_potential;
            bool fired = target_neuron.update_membrane_and_check_spike(current_event.effective_current, current_simulation_time);

            // std::cout << "  Neuron Layer " << current_event.target_neuron_layer_id << ", ID " << current_event.target_neuron_id
            //           << ": MP_before=" << initial_mp << ", Input=" << current_event.effective_current
            //           << ", MP_after=" << target_neuron.membrane_potential << ", Fired=" << fired << std::endl;

            if (fired) {
                int source_layer_idx = current_event.target_neuron_layer_id;
                int next_layer_idx = source_layer_idx + 1;

                if (next_layer_idx < 3) {
                    if (source_layer_idx == 1) {
                        for (int j = 0; j < NUM_OUTPUT_NEURONS; ++j) {
                            float effective_input_to_next_neuron = fc2_weight[current_event.target_neuron_id][j];
                            event_queue.push(SpikeEvent(current_simulation_time + 1, NEURON_SPIKE_EVENT, source_layer_idx, current_event.target_neuron_id, next_layer_idx, j, effective_input_to_next_neuron));
                            // if (effective_input_to_next_neuron != 0.0f) {
                            //     std::cout << "      Scheduling spike from Hidden " << current_event.target_neuron_id
                            //               << " to Output " << j << " with current " << effective_input_to_next_neuron
                            //               << " at time: " << current_simulation_time + 1 << std::endl;
                            // }
                        }
                    }
                }
                else {
                    output_spike_counts[current_event.target_neuron_id]++;
                    // std::cout << "      Output Neuron " << current_event.target_neuron_id
                    //           << " spiked! Total spikes: " << output_spike_counts[current_event.target_neuron_id] << std::endl;
                }
            }
        }
    }

    // std::cout << "\nSimulation finished. Total events processed: " << events_processed << std::endl;
    // std::cout << "Final heap size: " << event_queue.heap_size << std::endl;

    int max_spikes = -1;
    int winning_neuron_id = -1;
    // std::cout << "\nOutput Spike Counts:" << std::endl;
    for (int i = 0; i < NUM_OUTPUT_NEURONS; ++i) {
        // std::cout << "Neuron " << i << ": " << output_spike_counts[i] << " spikes" << std::endl;
        if (output_spike_counts[i] > max_spikes) {
            max_spikes = output_spike_counts[i];
            winning_neuron_id = i;
        }
    }
    // std::cout << "Winning Neuron ID: " << winning_neuron_id << std::endl;

    for (int i = 0; i < NUM_OUTPUT_NEURONS; ++i) {
        // std::cout << "Output Neuron " << i << " final MP: " << all_layers_output[i].get_membrane_potential() << std::endl;
    }

    return winning_neuron_id;
}