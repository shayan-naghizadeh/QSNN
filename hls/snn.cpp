#include "weights.h"

class LIFNeuron {
public:
    float membrane_potential;
    bool  has_fired_in_current_step;
    int   last_update_time;
    float threshold_voltage;
    float leak_reversal_potential;
    float fixed_leak_amount_per_step;

    enum ResetMechanism { SUBTRACT_THRESHOLD, RESET_TO_ZERO };
    ResetMechanism current_reset_mechanism;

    LIFNeuron(float v_th = 0.8f, float e_l = 0.0f, float fixed_leak_amt = 0.0f,
              ResetMechanism rest_mech = RESET_TO_ZERO)
        : membrane_potential(0.0f), has_fired_in_current_step(false), last_update_time(0),
          threshold_voltage(v_th), leak_reversal_potential(e_l),
          fixed_leak_amount_per_step(fixed_leak_amt), current_reset_mechanism(rest_mech) {}

    bool update_membrane_and_check_spike(float input_current, int current_event_time) {
        has_fired_in_current_step = false;
        int dt = current_event_time - last_update_time;
        int num_leak_steps = dt > 0 ? dt : 0;

#pragma HLS PIPELINE II=1
        for (int i = 0; i < num_leak_steps; ++i) {
            if (membrane_potential > leak_reversal_potential) {
                membrane_potential -= fixed_leak_amount_per_step;
                if (membrane_potential < leak_reversal_potential) membrane_potential = leak_reversal_potential;
            } else if (membrane_potential < leak_reversal_potential) {
                membrane_potential += fixed_leak_amount_per_step;
                if (membrane_potential > leak_reversal_potential) membrane_potential = leak_reversal_potential;
            }
        }

        membrane_potential += input_current;
        if (membrane_potential < 0) membrane_potential = 0;

        if (membrane_potential >= threshold_voltage) {
            has_fired_in_current_step = true;
            if (current_reset_mechanism == SUBTRACT_THRESHOLD)
                membrane_potential -= threshold_voltage;
            else
                membrane_potential = 0.0f;
        }
        last_update_time = current_event_time;
        return has_fired_in_current_step;
    }

    float get_membrane_potential() const { return membrane_potential; }
};

enum EventType { INPUT_SPIKE_EVENT, NEURON_SPIKE_EVENT };

struct SpikeEvent {
    int   scheduled_time;
    EventType Type;
    int   source_neuron_layer_id;
    int   source_neuron_id;
    int   target_neuron_layer_id;
    int   target_neuron_id;
    float effective_current;
};

const int MAX_EVENT_IN_HEAP = 100000;

class MinHeap {
private:
    SpikeEvent heap_array[MAX_EVENT_IN_HEAP];
    int        current_size;

    int parent(int i) { return (i - 1) >> 1; }
    int left_child(int i) { return (i << 1) + 1; }
    int right_child(int i) { return (i << 1) + 2; }

    bool is_less_priority(const SpikeEvent &a, const SpikeEvent &b) {
        if (a.scheduled_time != b.scheduled_time) return a.scheduled_time > b.scheduled_time;
        if (a.target_neuron_layer_id != b.target_neuron_layer_id) return a.target_neuron_layer_id < b.target_neuron_layer_id;
        return a.target_neuron_id > b.target_neuron_id;
    }

    void swap(SpikeEvent &a, SpikeEvent &b) {
        SpikeEvent t = a;
        a           = b;
        b           = t;
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

const int NUM_INPUT_NEURONS  = 784;
const int NUM_HIDDEN_NEURONS = 256;
const int NUM_OUTPUT_NEURONS = 10;

int SNN(int spike_input[50][784]);

int SNN(int spike_input[50][784]) {
    float v_threshold        = 0.8f;
    float e_leak             = 0.0f;
    float fixed_leak_per_step = 0.0f;

    static LIFNeuron all_layers_input[NUM_INPUT_NEURONS];
    static LIFNeuron all_layers_hidden[NUM_HIDDEN_NEURONS];
    static LIFNeuron all_layers_output[NUM_OUTPUT_NEURONS];
    int   output_spike_counts[NUM_OUTPUT_NEURONS];

#pragma HLS ARRAY_PARTITION variable=output_spike_counts complete dim=1

    for (int i = 0; i < NUM_INPUT_NEURONS; ++i) {
#pragma HLS UNROLL factor=4
        all_layers_input[i] = LIFNeuron(v_threshold, e_leak, fixed_leak_per_step);
    }
    for (int i = 0; i < NUM_HIDDEN_NEURONS; ++i) {
#pragma HLS UNROLL factor=4
        all_layers_hidden[i] = LIFNeuron(v_threshold, e_leak, fixed_leak_per_step);
    }
    for (int i = 0; i < NUM_OUTPUT_NEURONS; ++i) {
#pragma HLS UNROLL
        all_layers_output[i]  = LIFNeuron(v_threshold, e_leak, fixed_leak_per_step);
        output_spike_counts[i] = 0;
    }

    MinHeap event_queue;
    for (int t = 0; t < 50; ++t) {
        for (int n = 0; n < NUM_INPUT_NEURONS; ++n) {
#pragma HLS PIPELINE II=1
            if (spike_input[t][n] == 1) {
                SpikeEvent evt;
                evt.scheduled_time        = t;
                evt.Type                  = INPUT_SPIKE_EVENT;
                evt.source_neuron_layer_id = -1;
                evt.source_neuron_id      = -1;
                evt.target_neuron_layer_id = 0;
                evt.target_neuron_id      = n;
                evt.effective_current     = 1.0f;
                event_queue.push(evt);
            }
        }
    }

    int sim_time = 0;
    while (!event_queue.is_empty() && sim_time <= 50) {
#pragma HLS PIPELINE II=1
        SpikeEvent cur = event_queue.pop();
        sim_time       = cur.scheduled_time;

        if (cur.Type == INPUT_SPIKE_EVENT) {
            bool fired = all_layers_input[cur.target_neuron_id]
                              .update_membrane_and_check_spike(cur.effective_current, sim_time);
            if (fired) {
                for (int j = 0; j < NUM_HIDDEN_NEURONS; ++j) {
#pragma HLS UNROLL factor=4
                    SpikeEvent evt;
                    evt.scheduled_time        = sim_time + 1;
                    evt.Type                  = NEURON_SPIKE_EVENT;
                    evt.source_neuron_layer_id = 0;
                    evt.source_neuron_id      = cur.target_neuron_id;
                    evt.target_neuron_layer_id = 1;
                    evt.target_neuron_id      = j;
                    evt.effective_current     = fc1_weight[cur.target_neuron_id][j];
                    event_queue.push(evt);
                }
            }
        } else if (cur.Type == NEURON_SPIKE_EVENT) {
            bool fired = false;
            if (cur.target_neuron_layer_id == 1) {
                fired = all_layers_hidden[cur.target_neuron_id].update_membrane_and_check_spike(cur.effective_current, sim_time);
            } else if (cur.target_neuron_layer_id == 2) {
                fired = all_layers_output[cur.target_neuron_id].update_membrane_and_check_spike(cur.effective_current, sim_time);
            } else {
                continue;
            }

            if (fired) {
                if (cur.target_neuron_layer_id == 1) {
                    for (int j = 0; j < NUM_OUTPUT_NEURONS; ++j) {
#pragma HLS UNROLL
                        SpikeEvent evt;
                        evt.scheduled_time        = sim_time + 1;
                        evt.Type                  = NEURON_SPIKE_EVENT;
                        evt.source_neuron_layer_id = 1;
                        evt.source_neuron_id      = cur.target_neuron_id;
                        evt.target_neuron_layer_id = 2;
                        evt.target_neuron_id      = j;
                        evt.effective_current     = fc2_weight[cur.target_neuron_id][j];
                        event_queue.push(evt);
                    }
                } else if (cur.target_neuron_layer_id == 2) {
                    output_spike_counts[cur.target_neuron_id]++;
                }
            }
        }
    }

    int max_spikes = -1;
    int winner = -1;
    for (int i = 0; i < NUM_OUTPUT_NEURONS; ++i) {
#pragma HLS UNROLL
        if (output_spike_counts[i] > max_spikes) {
            max_spikes = output_spike_counts[i];
            winner = i;
        }
    }
    return winner;
}
