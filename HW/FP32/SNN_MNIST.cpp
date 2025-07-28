
#include <iostream> 
#include<vector>
#include<queue>
#include<functional>



class LIFNeuron {
    public:
    float membrane_potential;
    bool has_fired_in_current_steps;
    int last_update_time;

    float threshold_voltage;
    float leak_reversal_potential;
    float fixed_leak_amount_per_step;

    enum ResetMechanism { SUBTRACT_THRESHOLD , RESET_TO_ZERO  };
    ResetMechanism current_reset_mechanism;

    LIFNeuron (float v_th=0.5, float e_l=0, float fixed_leak_amt = 0.05 , ResetMechanism rest_mech=RESET_TO_ZERO):
    membrane_potential(0.0f),
    has_fired_in_current_steps(false),
    last_update_time(0) ,
    threshold_voltage(v_th),
    leak_reversal_potential(e_l),
    fixed_leak_amount_per_step(fixed_leak_amt),
    current_reset_mechanism(rest_mech){}


    bool update_membrane_and_check_spike(float input_current , int current_event_time ){

        has_fired_in_current_steps = false;

        int dt = current_event_time - last_update_time;

        int num_leak_steps = dt ; 

        if (num_leak_steps < 0 ) num_leak_steps = 0 ;

        for ( int i=0 ; i < num_leak_steps ; ++i ){

        if (membrane_potential> leak_reversal_potential){
            membrane_potential-= fixed_leak_amount_per_step;
            if (membrane_potential< leak_reversal_potential){
                membrane_potential= leak_reversal_potential;
            }
        }else if (membrane_potential< leak_reversal_potential){
            membrane_potential+= fixed_leak_amount_per_step;
            if(membrane_potential> leak_reversal_potential){
                membrane_potential= leak_reversal_potential;
            }
        }

        }

        membrane_potential+= input_current;

        if (membrane_potential>= threshold_voltage){

            has_fired_in_current_steps = true;

            if (current_reset_mechanism == SUBTRACT_THRESHOLD ){
                membrane_potential-= threshold_voltage;

            }else if (current_reset_mechanism == RESET_TO_ZERO){
                membrane_potential= 0.0f;
            }
        }

        last_update_time = current_event_time;
        
        return has_fired_in_current_steps;

    }



    float get_membrane_potential() const {

        return membrane_potential;
    }

    };


enum EventType{
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

    bool operator>(const SpikeEvent& other)const{
        if (scheduled_time != other.scheduled_time){
            return scheduled_time > other.scheduled_time;
        }
        return target_neuron_id > other.target_neuron_id;
    }


};


int main(){


// LIFNeuron firstNeuron;
// std::cout<<firstNeuron.get_membrane_potential();
// std::cout<<"\n"<<firstNeuron.update(0.40);
// std::cout<<"\n"<<firstNeuron.get_membrane_potential();
// std::cout<<"\n"<<firstNeuron.update(0);
// std::cout<<"\n"<<firstNeuron.get_membrane_potential();

const int num_input_neurons = 3;
const int num_hidden_neurons = 4;
const int num_output_neurons = 2;

float v_threshold = 1.0f;
float e_leak = 0;
float fixed_leak_per_step = 0.05;
LIFNeuron::ResetMechanism reset_mech = LIFNeuron::RESET_TO_ZERO ;


std::vector<std::vector<LIFNeuron>> all_layers;

all_layers.emplace_back();
for ( int i = 0; i < num_input_neurons ; ++i) {
    all_layers[0].emplace_back( v_threshold , e_leak , fixed_leak_per_step , reset_mech);
}


all_layers.emplace_back();
for ( int i = 0 ; i < num_hidden_neurons ; ++i ) {
    all_layers[1].emplace_back( v_threshold , e_leak , fixed_leak_per_step , reset_mech);
}

all_layers.emplace_back();
for ( int i = 0 ; i < num_output_neurons ; ++i ) {
    all_layers[2].emplace_back( v_threshold , e_leak , fixed_leak_per_step , reset_mech );
}


std::vector<std::vector<std::vector<float>>> all_weights;

std::vector<std::vector<float>> weights_ih( num_input_neurons , std::vector<float>(num_hidden_neurons));
weights_ih[0][0] = 0.6f; weights_ih[0][1] = 0.3f; weights_ih[0][2] = 0.1f; weights_ih[0][3] = 0.8f;

weights_ih[1][0] = 0.2f; weights_ih[1][1] = 0.7f; weights_ih[1][2] = 0.4f; weights_ih[1][3] = 0.1f;

weights_ih[2][0] = 0.9f; weights_ih[2][1] = 0.1f; weights_ih[2][2] = 0.6f; weights_ih[2][3] = 0.2f;

all_weights.push_back(weights_ih);

std::vector < std::vector<float>> weights_ho ( num_hidden_neurons , std::vector<float>(num_output_neurons));
weights_ho[0][0] = 0.5f; weights_ho[0][1] = 0.3f;
weights_ho[1][0] = 0.2f; weights_ho[1][1] = 0.8f;
weights_ho[2][0] = 0.7f; weights_ho[2][1] = 0.1f;
weights_ho[3][0] = 0.4f; weights_ho[3][1] = 0.6f;
all_weights.push_back(weights_ho);



std::priority_queue<SpikeEvent , std::vector<SpikeEvent> , std::greater<SpikeEvent>> event_queue;

int current_simulation_time = 0.0;

event_queue.push({ 20 , INPUT_SPIKE_EVENT , -1 , -1 , 0 , 0 , 1.2f });
event_queue.push({ 50 , INPUT_SPIKE_EVENT , -1 , -1 , 0 , 1 , 1.0f });
event_queue.push({ 70 , INPUT_SPIKE_EVENT , -1 , -1 , 0 , 2 , 0.5f });

int simulation_end_time = 100;

std::cout<<"---starting simulation---\n";

while( !event_queue.empty() & current_simulation_time <= simulation_end_time ){

    SpikeEvent current_event = event_queue.top();

    event_queue.pop();

    current_simulation_time = current_event.scheduled_time;

    if ( current_simulation_time > simulation_end_time ){

        break;

    }

    std::cout<<"\nTime  "<<current_simulation_time<<" - processing event type ";

    if ( current_event.Type == INPUT_SPIKE_EVENT){

        std::cout << "INPUT_SPIKE_EVENT (to Input Layer Neuron " << current_event.target_neuron_id << ")\n";

        LIFNeuron& target_neuron = all_layers[0][current_event.target_neuron_id];

        bool fired = target_neuron.update_membrane_and_check_spike ( current_event.effective_current , current_simulation_time );

        std::cout<<" Input Neuron "<< current_event.target_neuron_id<<" Vm "<<target_neuron.get_membrane_potential();

        if (fired){

            std::cout<<"\n Spike ";

            int source_layer_idx = 0 ;

            int next_layer_idx = source_layer_idx + 1 ; 

            if (next_layer_idx < all_layers.size() ){

                const auto& current_layer_weights = all_weights[source_layer_idx];

                const int num_neurons_in_next_layer = all_layers[next_layer_idx].size();

                for (int j = 0 ; j < num_neurons_in_next_layer ; ++j ){

                    float effective_input_to_next_neuron = current_layer_weights[current_event.target_neuron_id][j];

                    event_queue.push({ current_simulation_time , NEURON_SPIKE_EVENT , source_layer_idx , current_event.target_neuron_id , next_layer_idx , j , effective_input_to_next_neuron });

                }


            }

    }

    std::cout << std::endl;

}

    else if (current_event.Type == NEURON_SPIKE_EVENT) {

                      std::cout << "NEURON_SPIKE_EVENT (from Layer " << current_event.source_neuron_layer_id
                      << " Neuron " << current_event.source_neuron_id
                      << " to Layer " << current_event.target_neuron_layer_id
                      << " Neuron " << current_event.target_neuron_id << ")\n";

                      LIFNeuron& target_neuron = all_layers[current_event.target_neuron_layer_id][current_event.target_neuron_id];

                      bool fired = target_neuron.update_membrane_and_check_spike(current_event.effective_current , current_simulation_time );

                      std::cout << "  Layer " << current_event.target_neuron_layer_id
                      << " Neuron " << current_event.target_neuron_id
                      << ": Vm = " << target_neuron.get_membrane_potential();

                        if (fired){
                            
                            std::cout<<" spike \n";

                            int source_layer_idx = current_event.target_neuron_layer_id;
                            int next_layer_idx = source_layer_idx + 1;

                            if (next_layer_idx < all_layers.size()){

                                const auto& current_layer_weights = all_weights[source_layer_idx];

                                const int num_neuron_in_next_layer = all_layers[next_layer_idx].size();

                                for (int j = 0 ; j < num_neuron_in_next_layer ; ++j ){

                                    float effective_input_to_next_neuron = current_layer_weights[current_event.target_neuron_id][j];

                                    event_queue.push({ current_simulation_time , NEURON_SPIKE_EVENT , source_layer_idx , current_event.target_neuron_id , next_layer_idx , j , effective_input_to_next_neuron });

                                }


                            }

                            else{

                                    std::cout<<"output layer spike";

                                }

                        }
                                    std::cout << std::endl;

                }

        }




    std::cout << "\n--- End of Event-Driven Simulation ---\n";
    std::cout << "Final Simulation Time: " << current_simulation_time << "s\n";

    return 0;



}

