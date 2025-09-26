#include <bitset>
#include "weights62.h"
#include "posit6_2.h"

// The LIFNeuron class with corrections for encapsulation and initialization.
class LIFNeuron {
private:
    posit6_2 membrane_potential;

public:
    bool has_fired_in_current_step;
    int last_update_time;
    posit6_2 threshold_voltage;
    posit6_2 leak_reversal_potential;
    posit6_2 fixed_leak_amount_per_step;

    enum ResetMechanism { SUBTRACT_THRESHOLD, RESET_TO_ZERO };
    ResetMechanism current_reset_mechanism;

    // --- FIX: Using the correct 3-argument constructor for posit6_2 ---
    LIFNeuron() :
        membrane_potential(posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2))),
        has_fired_in_current_step(false),
        last_update_time(0),
        threshold_voltage(posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2))),
        leak_reversal_potential(posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2))),
        fixed_leak_amount_per_step(posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2))),
        current_reset_mechanism(RESET_TO_ZERO) {}

    LIFNeuron(posit6_2 v_th, posit6_2 e_l, posit6_2 fixed_leak_amt, ResetMechanism rest_mech = RESET_TO_ZERO) :
        membrane_potential(posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2))),
        has_fired_in_current_step(false),
        last_update_time(0),
        threshold_voltage(v_th),
        leak_reversal_potential(e_l),
        fixed_leak_amount_per_step(fixed_leak_amt),
        current_reset_mechanism(rest_mech) {}

    // --- Reverted to using the overloaded operators you defined. ---
    bool update_membrane_and_check_spike(posit6_2 input_current, int current_event_time) {
        has_fired_in_current_step = false;
        membrane_potential += input_current;

        posit6_2 p_zero = posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2));

        // Clamp potential to zero if it becomes negative
        if (membrane_potential < p_zero)
            membrane_potential = p_zero;

        if (membrane_potential >= threshold_voltage) {
            has_fired_in_current_step = true;
            if (current_reset_mechanism == SUBTRACT_THRESHOLD) {
                membrane_potential -= threshold_voltage;
            } else {
                membrane_potential = p_zero;
            }
        }
        last_update_time = current_event_time;
        return has_fired_in_current_step;
    }

    posit6_2 get_membrane_potential() const { return membrane_potential; }
};

enum EventType { INPUT_SPIKE_EVENT, NEURON_SPIKE_EVENT };

struct SpikeEvent {
    int scheduled_time;
    EventType Type;
    int source_neuron_layer_id;
    int source_neuron_id;
    int target_neuron_layer_id;
    int target_neuron_id;
    posit6_2 effective_current;

    SpikeEvent() :
        scheduled_time(0), Type(INPUT_SPIKE_EVENT), source_neuron_layer_id(-1),
        source_neuron_id(-1), target_neuron_layer_id(-1), target_neuron_id(-1),
        effective_current(posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2))) {}

    SpikeEvent(int s_time, EventType e_type, int s_la_id, int s_id, int t_la_id, int t_id, posit6_2 e_current) :
        scheduled_time(s_time), Type(e_type), source_neuron_layer_id(s_la_id),
        source_neuron_id(s_id), target_neuron_layer_id(t_la_id),
        target_neuron_id(t_id), effective_current(e_current) {}
};

const int MAX_EVENT_IN_HEAP = 50000;
class MinHeap {
private:
    SpikeEvent heap_array[MAX_EVENT_IN_HEAP];
    int current_size;

    int parent(int i) { return (i - 1) / 2; }
    int left_child(int i) { return (2 * i + 1); }
    int right_child(int i) { return (2 * i + 2); }

    bool is_less_priority(const SpikeEvent& a, const SpikeEvent& b) {
        if (a.scheduled_time != b.scheduled_time) return a.scheduled_time > b.scheduled_time;
        if (a.target_neuron_layer_id != b.target_neuron_layer_id) return a.target_neuron_layer_id < b.target_neuron_layer_id;
        return a.target_neuron_id > b.target_neuron_id;
    }

    void swap(SpikeEvent &a, SpikeEvent &b) {
        SpikeEvent t = a;
        a = b;
        b = t;
    }

    void heapify_up(int i) {
    #pragma HLS INLINE off
        while (i != 0 && is_less_priority(heap_array[parent(i)], heap_array[i])) {
            swap(heap_array[i], heap_array[parent(i)]);
            i = parent(i);
        }
    }

    void heapify_down(int i) {
    #pragma HLS INLINE off
        while (true) {
            int hi = i;
            int l  = left_child(i);
            int r  = right_child(i);
            if (l < current_size && is_less_priority(heap_array[hi], heap_array[l])) hi = l;
            if (r < current_size && is_less_priority(heap_array[hi], heap_array[r])) hi = r;
            if (hi != i) {
                swap(heap_array[i], heap_array[hi]);
                i = hi;
            } else {
                break;
            }
        }
    }

public:
    int heap_size;
    MinHeap() : current_size(0), heap_size(0) {}

    bool is_empty() const { return current_size == 0; }
    bool is_full()  const { return current_size == MAX_EVENT_IN_HEAP; }
    int  size()     const { return current_size; }

    void push(SpikeEvent event) {
        #pragma HLS INLINE off
        if (is_full()) return;
        heap_array[current_size++] = event;
        heap_size++;
        heapify_up(current_size - 1);
    }

    SpikeEvent top() { return heap_array[0]; }

    SpikeEvent pop() {
        #pragma HLS INLINE off
        SpikeEvent root = heap_array[0];
        current_size--; heap_size--;
        heap_array[0]  = heap_array[current_size];
        heapify_down(0);
        return root;
    }

    void clear() { current_size = 0; heap_size = 0; }
};

const int NUM_INPUT_NEURONS = 784;
const int NUM_HIDDEN_NEURONS = 256;
const int NUM_OUTPUT_NEURONS = 10;

posit6_2 adder_tree_sum(posit6_2 currents[], int count) {
    posit6_2 p_zero = posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2));
    if (count == 0) return p_zero;

    posit6_2 total_sum = p_zero;
    SUM_LOOP: for (int i = 0; i < count; ++i) {
        #pragma HLS PIPELINE
        total_sum += currents[i];
    }
    return total_sum;
}


int snn(int spike_input[50][784]) {
    posit6_2 v_threshold = posit6_2(ap_uint<6>(0b001111), ap_uint<6>(6), ap_uint<6>(2));
    posit6_2 e_leak = posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2));
    posit6_2 fixed_leak_per_step = posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2));

    static LIFNeuron all_layers_input[NUM_INPUT_NEURONS];
    static LIFNeuron all_layers_hidden[NUM_HIDDEN_NEURONS];
    static LIFNeuron all_layers_output[NUM_OUTPUT_NEURONS];
    static int output_spike_counts[NUM_OUTPUT_NEURONS] = {0};
    #pragma HLS ARRAY_PARTITION variable=output_spike_counts complete dim=1

    INIT_INPUT: for (int i = 0; i < NUM_INPUT_NEURONS; ++i) {
        #pragma HLS PIPELINE
        all_layers_input[i] = LIFNeuron(v_threshold, e_leak, fixed_leak_per_step);
    }
    INIT_HIDDEN: for (int i = 0; i < NUM_HIDDEN_NEURONS; ++i) {
        #pragma HLS PIPELINE
        all_layers_hidden[i] = LIFNeuron(v_threshold, e_leak, fixed_leak_per_step);
    }
    INIT_OUTPUT: for (int i = 0; i < NUM_OUTPUT_NEURONS; ++i) {
        #pragma HLS PIPELINE
        all_layers_output[i] = LIFNeuron(v_threshold, e_leak, fixed_leak_per_step);
        output_spike_counts[i] = 0;
    }

    const int MAX_SIMULATION_TIME = 50;
    MinHeap event_queue;
    LOAD_SPIKES: for (int t = 0; t < 50; ++t) {
        for (int n = 0; n < NUM_INPUT_NEURONS; ++n) {
            #pragma HLS PIPELINE II=1
            if (spike_input[t][n] == 1) {
                event_queue.push(SpikeEvent(
                    t, INPUT_SPIKE_EVENT, -1, -1, 0, n,
                    posit6_2(ap_uint<6>(0b010000), ap_uint<6>(6), ap_uint<6>(2))
                ));
            }
        }
    }

    int current_simulation_time = 0;
    static bool fired_this_tick_input [NUM_INPUT_NEURONS];
    static bool fired_this_tick_hidden[NUM_HIDDEN_NEURONS];
    static bool fired_this_tick_output[NUM_OUTPUT_NEURONS];
    int last_tick = -1;

    INIT_FIRED_INPUT: for (int i = 0; i < NUM_INPUT_NEURONS;  ++i) fired_this_tick_input[i]  = false;
    INIT_FIRED_HIDDEN: for (int i = 0; i < NUM_HIDDEN_NEURONS; ++i) fired_this_tick_hidden[i] = false;
    INIT_FIRED_OUTPUT: for (int i = 0; i < NUM_OUTPUT_NEURONS; ++i) fired_this_tick_output[i] = false;

    PROCESS_EVENTS: while (!event_queue.is_empty() && current_simulation_time <= MAX_SIMULATION_TIME) {
        #pragma HLS LOOP_TRIPCOUNT min=100 max=50000

        SpikeEvent current_event = event_queue.pop();
        current_simulation_time = current_event.scheduled_time;

        if (current_simulation_time > MAX_SIMULATION_TIME) {
            break;
        }

        if (current_simulation_time != last_tick) {
            RESET_FIRED_INPUT: for (int i = 0; i < NUM_INPUT_NEURONS;  ++i) fired_this_tick_input[i]  = false;
            RESET_FIRED_HIDDEN: for (int i = 0; i < NUM_HIDDEN_NEURONS; ++i) fired_this_tick_hidden[i] = false;
            RESET_FIRED_OUTPUT: for (int i = 0; i < NUM_OUTPUT_NEURONS; ++i) fired_this_tick_output[i] = false;
            last_tick = current_simulation_time;
        }

        const int MAX_SIMULTANEOUS_INPUTS = 32;
        // --- FIX: Explicitly initialize every element of the array. ---
        posit6_2 currents_to_sum[MAX_SIMULTANEOUS_INPUTS] = {
            posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)), posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)),
            posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)), posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)),
            posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)), posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)),
            posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)), posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)),
            posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)), posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)),
            posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)), posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)),
            posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)), posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)),
            posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)), posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)),
            posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)), posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)),
            posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)), posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)),
            posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)), posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)),
            posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)), posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)),
            posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)), posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)),
            posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)), posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)),
            posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)), posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)),
            posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2)), posit6_2(ap_uint<6>(0), ap_uint<6>(6), ap_uint<6>(2))
        };


        int current_count = 0;
        currents_to_sum[current_count++] = current_event.effective_current;
        int target_layer = current_event.target_neuron_layer_id;
        int target_id = current_event.target_neuron_id;

        bool continue_grouping = true;
        GROUP_EVENTS: for (int k = 0; k < MAX_SIMULTANEOUS_INPUTS - 1; ++k) {
            #pragma HLS PIPELINE
            if (continue_grouping && !event_queue.is_empty()) {
                const SpikeEvent &peek = event_queue.top();
                if (peek.scheduled_time == current_simulation_time &&
                    peek.target_neuron_layer_id == target_layer &&
                    peek.target_neuron_id == target_id) {

                    SpikeEvent next_event = event_queue.pop();
                    currents_to_sum[current_count++] = next_event.effective_current;
                } else {
                    continue_grouping = false;
                }
            }
        }

        posit6_2 total_input_current = adder_tree_sum(currents_to_sum, current_count);

        if (current_event.Type == INPUT_SPIKE_EVENT) {
            if (!fired_this_tick_input[target_id]) {
                bool fired = all_layers_input[target_id].update_membrane_and_check_spike(total_input_current, current_simulation_time);
                if (fired) {
                    fired_this_tick_input[target_id] = true;
                    PROPAGATE_INPUT: for (int j = 0; j < NUM_HIDDEN_NEURONS; ++j) {
                        #pragma HLS PIPELINE
                        event_queue.push(SpikeEvent(current_simulation_time + 1, NEURON_SPIKE_EVENT, 0, target_id, 1, j, fc1_weight[target_id][j]));
                    }
                }
            }
        }
        else if (current_event.Type == NEURON_SPIKE_EVENT) {
            if (target_layer == 1) {
                if (!fired_this_tick_hidden[target_id]) {
                    bool fired = all_layers_hidden[target_id].update_membrane_and_check_spike(total_input_current, current_simulation_time);
                    if (fired) {
                        fired_this_tick_hidden[target_id] = true;
                        PROPAGATE_HIDDEN: for (int j = 0; j < NUM_OUTPUT_NEURONS; ++j) {
                            #pragma HLS PIPELINE
                            event_queue.push(SpikeEvent(current_simulation_time + 1, NEURON_SPIKE_EVENT, 1, target_id, 2, j, fc2_weight[target_id][j]));
                        }
                    }
                }
            } else if (target_layer == 2) {
                if (!fired_this_tick_output[target_id]) {
                    bool fired = all_layers_output[target_id].update_membrane_and_check_spike(total_input_current, current_simulation_time);
                    if (fired) {
                        fired_this_tick_output[target_id] = true;
                        output_spike_counts[target_id]++;
                    }
                }
            }
        }
    }

    int max_spikes = -1;
    int winning_neuron_id = -1;
    FIND_WINNER: for (int i = 0; i < NUM_OUTPUT_NEURONS; ++i) {
        #pragma HLS PIPELINE
        if (output_spike_counts[i] > max_spikes) {
            max_spikes = output_spike_counts[i];
            winning_neuron_id = i;
        }
    }
    return winning_neuron_id;
}

