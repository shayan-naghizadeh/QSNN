#include <iostream>
#include "weights.h"
#include "posit.h"

// The LIFNeuron class remains unchanged.
class LIFNeuron {
public:
    posit membrane_potential;
    bool has_fired_in_current_step;
    int last_update_time;
    posit threshold_voltage;
    posit leak_reversal_potential;
    posit fixed_leak_amount_per_step;

    enum ResetMechanism { SUBTRACT_THRESHOLD, RESET_TO_ZERO };
    ResetMechanism current_reset_mechanism;

    LIFNeuron(posit v_th = posit(0,8,2), posit e_l = posit(0,8,2), posit fixed_leak_amt = posit(0,8,2), ResetMechanism rest_mech = RESET_TO_ZERO) :
        membrane_potential(posit(0,8,2)),
        has_fired_in_current_step(false),
        last_update_time(0),
        threshold_voltage(v_th),
        leak_reversal_potential(e_l),
        fixed_leak_amount_per_step(fixed_leak_amt),
        current_reset_mechanism(rest_mech) {}

    // This function updates the neuron's potential based on a single accumulated current.
    bool update_membrane_and_check_spike(posit input_current, int current_event_time) {
        has_fired_in_current_step = false;

        membrane_potential += input_current;
        if (membrane_potential < posit(0,8,2)) membrane_potential = posit(0,8,2);

        if (membrane_potential >= threshold_voltage) {
            has_fired_in_current_step = true;
            if (current_reset_mechanism == SUBTRACT_THRESHOLD) {
                membrane_potential -= threshold_voltage;
            }
            else {
                membrane_potential = posit(0,8,2);
            }
        }

        last_update_time = current_event_time;
        return has_fired_in_current_step;
    }

    posit get_membrane_potential() const {
        return membrane_potential;
    }
};

// EventType enum and SpikeEvent struct remain unchanged.
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
    posit effective_current;

    SpikeEvent() :
        scheduled_time(0),
        Type(INPUT_SPIKE_EVENT),
        source_neuron_layer_id(-1),
        source_neuron_id(-1),
        target_neuron_layer_id(-1),
        target_neuron_id(-1),
        effective_current(posit(0,8,2)) {}

    SpikeEvent(int s_time, EventType e_type, int s_la_id, int s_id, int t_la_id, int t_id, posit e_current) :
        scheduled_time(s_time),
        Type(e_type),
        source_neuron_layer_id(s_la_id),
        source_neuron_id(s_id),
        target_neuron_layer_id(t_la_id),
        target_neuron_id(t_id),
        effective_current(e_current) {}
};

// The MinHeap class remains unchanged.
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


// --- CHANGE START: HLS-friendly 2-level adder tree function ---
// This function implements the specific 2-level adder for up to 4 inputs.
// It uses a switch statement which is very efficient for HLS synthesis.
posit adder_tree_sum(posit currents[], int count) {
    posit p_zero = posit(0, 8, 2);

    // This structure maps directly to multiplexers in hardware.
    switch (count) {
        case 0:
            return p_zero;
        case 1:
            return currents[0];
        case 2:
            // Level 1 addition
            return currents[0] + currents[1];
        case 3:
            // Level 1: (a+b), Level 2: (sum)+c
            return (currents[0] + currents[1]) + currents[2];
        case 4: {
            // This is the exact 2-level logic you requested.
            // Level 1 additions
            posit sum1 = currents[0] + currents[1];
            posit sum2 = currents[2] + currents[3];
            // Level 2 addition
            return sum1 + sum2;
        }
        default: {
            // Fallback for more than 4 inputs. A simple loop is synthesizable.
            // This could be further optimized with a more complex tree if needed.
            posit total_sum = p_zero;
            for (int i = 0; i < count; ++i) {
                total_sum += currents[i];
            }
            return total_sum;
        }
    }
}
// --- CHANGE END ---


int snn(int spike_input[50][784]) {
    posit v_threshold = posit(0b00111101,8,2);
    posit e_leak = posit(0,8,2);
    posit fixed_leak_per_step = posit(0,8,2);
    LIFNeuron::ResetMechanism reset_mech = LIFNeuron::RESET_TO_ZERO;

    static LIFNeuron all_layers_input[NUM_INPUT_NEURONS];
    static LIFNeuron all_layers_hidden[NUM_HIDDEN_NEURONS];
    static LIFNeuron all_layers_output[NUM_OUTPUT_NEURONS];
    static int output_spike_counts[NUM_OUTPUT_NEURONS] = {0};

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

    int MAX_SIMULATION_TIME = 50;
    static MinHeap event_queue;
    event_queue.clear();

    for (int t = 0; t < 50; ++t) {
        for (int neuron_idx = 0; neuron_idx < NUM_INPUT_NEURONS; ++neuron_idx) {
            if (spike_input[t][neuron_idx] == 1) {
                event_queue.push(SpikeEvent(t, INPUT_SPIKE_EVENT, -1, -1, 0, neuron_idx, posit(0b01000000,8,2)));
            }
        }
    }

    int current_simulation_time = 0;
    long long events_processed = 0;
    static bool fired_this_tick_input [NUM_INPUT_NEURONS];
    static bool fired_this_tick_hidden[NUM_HIDDEN_NEURONS];
    static bool fired_this_tick_output[NUM_OUTPUT_NEURONS];
    int last_tick = -1;
    for (int i = 0; i < NUM_INPUT_NEURONS;  ++i) fired_this_tick_input[i]  = false;
    for (int i = 0; i < NUM_HIDDEN_NEURONS; ++i) fired_this_tick_hidden[i] = false;
    for (int i = 0; i < NUM_OUTPUT_NEURONS; ++i) fired_this_tick_output[i] = false;

    while (!event_queue.is_empty() && current_simulation_time <= MAX_SIMULATION_TIME) {
        SpikeEvent current_event = event_queue.pop();
        events_processed++;
        current_simulation_time = current_event.scheduled_time;

        if (current_simulation_time > MAX_SIMULATION_TIME) {
            break;
        }

        if (current_simulation_time != last_tick) {
            for (int i = 0; i < NUM_INPUT_NEURONS;  ++i) fired_this_tick_input[i]  = false;
            for (int i = 0; i < NUM_HIDDEN_NEURONS; ++i) fired_this_tick_hidden[i] = false;
            for (int i = 0; i < NUM_OUTPUT_NEURONS; ++i) fired_this_tick_output[i] = false;
            last_tick = current_simulation_time;
        }

        // --- CHANGE START: Logic to group events and use the adder tree ---
        // 1. Use a fixed-size array instead of std::vector for HLS.
        const int MAX_SIMULTANEOUS_INPUTS = 32; // A safe upper bound, increased to 32.
        // **FIX**: Explicitly initialize the array to satisfy the compiler,
        // as the 'posit' class lacks a default constructor.
        posit currents_to_sum[MAX_SIMULTANEOUS_INPUTS] = {
            posit(0,8,2), posit(0,8,2), posit(0,8,2), posit(0,8,2),
            posit(0,8,2), posit(0,8,2), posit(0,8,2), posit(0,8,2),
            posit(0,8,2), posit(0,8,2), posit(0,8,2), posit(0,8,2),
            posit(0,8,2), posit(0,8,2), posit(0,8,2), posit(0,8,2),
            posit(0,8,2), posit(0,8,2), posit(0,8,2), posit(0,8,2),
            posit(0,8,2), posit(0,8,2), posit(0,8,2), posit(0,8,2),
            posit(0,8,2), posit(0,8,2), posit(0,8,2), posit(0,8,2),
            posit(0,8,2), posit(0,8,2), posit(0,8,2), posit(0,8,2)
        };
        int current_count = 0;

        // 2. Store the first event's current.
        currents_to_sum[current_count++] = current_event.effective_current;
        int target_layer = current_event.target_neuron_layer_id;
        int target_id = current_event.target_neuron_id;

        // 3. Group all other events for the same neuron at the same time.
        while (!event_queue.is_empty() &&
               event_queue.top().scheduled_time == current_simulation_time &&
               event_queue.top().target_neuron_layer_id == target_layer &&
               event_queue.top().target_neuron_id == target_id)
        {
            // Ensure we don't overflow our fixed-size array.
            if (current_count < MAX_SIMULTANEOUS_INPUTS) {
                SpikeEvent next_event = event_queue.pop();
                currents_to_sum[current_count++] = next_event.effective_current;
            } else {
                // If we exceed the max, stop grouping to prevent memory errors.
                // In a real hardware design, this might be an error flag.
                break;
            }
        }

        // 4. Sum the collected currents using our HLS-friendly adder tree.
        posit total_input_current = adder_tree_sum(currents_to_sum, current_count);

        // 5. Update the target neuron exactly ONCE with the final sum.
        if (current_event.Type == INPUT_SPIKE_EVENT) {
            LIFNeuron& target_neuron = all_layers_input[target_id];
            if (!fired_this_tick_input[target_id]) {
                bool fired = target_neuron.update_membrane_and_check_spike(total_input_current, current_simulation_time);
                if (fired) {
                    fired_this_tick_input[target_id] = true;
                    for (int j = 0; j < NUM_HIDDEN_NEURONS; ++j) {
                        event_queue.push(SpikeEvent(current_simulation_time + 1, NEURON_SPIKE_EVENT, 0, target_id, 1, j, fc1_weight[target_id][j]));
                    }
                }
            }
        }
        else if (current_event.Type == NEURON_SPIKE_EVENT) {
            LIFNeuron* target_neuron_ptr = nullptr;
            bool already_fired = false;

            if (target_layer == 1) {
                target_neuron_ptr = &all_layers_hidden[target_id];
                already_fired = fired_this_tick_hidden[target_id];
            } else if (target_layer == 2) {
                target_neuron_ptr = &all_layers_output[target_id];
                already_fired = fired_this_tick_output[target_id];
            }

            if (target_neuron_ptr != nullptr && !already_fired) {
                bool fired = target_neuron_ptr->update_membrane_and_check_spike(total_input_current, current_simulation_time);
                if (fired) {
                    if (target_layer == 1) {
                        fired_this_tick_hidden[target_id] = true;
                        // Propagate spikes to the next layer (output)
                        for (int j = 0; j < NUM_OUTPUT_NEURONS; ++j) {
                            event_queue.push(SpikeEvent(current_simulation_time + 1, NEURON_SPIKE_EVENT, 1, target_id, 2, j, fc2_weight[target_id][j]));
                        }
                    } else { // target_layer == 2
                        fired_this_tick_output[target_id] = true;
                        output_spike_counts[target_id]++;
                    }
                }
            }
        }
        // --- CHANGE END ---
    }

    int max_spikes = -1;
    int winning_neuron_id = -1;
    for (int i = 0; i < NUM_OUTPUT_NEURONS; ++i) {
        if (output_spike_counts[i] > max_spikes) {
            max_spikes = output_spike_counts[i];
            winning_neuron_id = i;
        }
    }
    return winning_neuron_id;
}

