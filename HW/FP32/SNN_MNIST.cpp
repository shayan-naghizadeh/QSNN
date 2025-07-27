#include <iostream> 
#include<vector>
#include<queue>
#include<functional>



class LIFNeuron {
    public:
    float memberane_potential;
    bool has_fired_in_current_steps;

    float threshold_voltage;
    float leak_reversal_potential;
    float fixed_leak_amount_per_step;

    enum ResetMechanism { SUBTRACT_THERSHOLD, REST_TO_ZERO };
    ResetMechanism current_reset_mechanism;

    LIFNeuron (float v_th=0.5, float e_l=0, float fixed_leak_amt=0.2 , ResetMechanism rest_mech=REST_TO_ZERO):
    memberane_potential(0.0f),
    has_fired_in_current_steps(false),
    threshold_voltage(v_th),
    leak_reversal_potential(e_l),
    fixed_leak_amount_per_step(fixed_leak_amt),
    current_reset_mechanism(rest_mech){}


    bool update_membrane_and_check_spike(float input_current){
        has_fired_in_current_steps = false;

        if (memberane_potential > leak_reversal_potential){
            memberane_potential -= fixed_leak_amount_per_step;
            if (memberane_potential < leak_reversal_potential){
                memberane_potential = leak_reversal_potential;
            }
        }else if (memberane_potential < leak_reversal_potential){
            memberane_potential += fixed_leak_amount_per_step;
            if(memberane_potential < leak_reversal_potential){
                memberane_potential = leak_reversal_potential;
            }
        }

        memberane_potential += input_current;

        if (memberane_potential > threshold_voltage){
            has_fired_in_current_steps = true;

            if (current_reset_mechanism == SUBTRACT_THERSHOLD){
                memberane_potential -= threshold_voltage;
            }else if (memberane_potential == REST_TO_ZERO){
                memberane_potential = 0.0f;
            }
        }
        return has_fired_in_current_steps;
    }


    // bool update(float input_current){
    //     //first leakage second check for spike
    //     has_fired_in_current_steps = false;

    //     if (memberane_potential > leak_reversal_potential){

    //         memberane_potential -= fixed_leak_amount_per_step;

    //         if (memberane_potential < leak_reversal_potential){

    //             memberane_potential = leak_reversal_potential;
    //         }
    //     }else if (memberane_potential < leak_reversal_potential){

    //         memberane_potential += fixed_leak_amount_per_step;
        
    //         if (memberane_potential > leak_reversal_potential){

    //             memberane_potential = leak_reversal_potential;
    //         }
    //     }



    //     memberane_potential += input_current;

    //     if ( memberane_potential >= threshold_voltage){
    //         has_fired_in_current_steps = true;

    //         if (current_reset_mechanism == SUBTRACT_THERSHOLD){

    //             memberane_potential -= threshold_voltage;

    //         }   else if(current_reset_mechanism == REST_TO_ZERO){

    //             memberane_potential = 0.0f;
    //         }

    //     }
    //     return has_fired_in_current_steps;
    // }

    float get_membrane_potential() const {

        return memberane_potential;
    }

    };


enum EventType{
    INPUT_SPIKE_EVENT,
    NEURON_SPIKE_EVENT
};

struct SpikeEvent {

    double scheduled_time;
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
LIFNeuron::ResetMechanism reset_mech = LIFNeuron::REST_TO_ZERO;


std::vector<std::vector<LIFNeuron>> all_layers;

all_layers.emplace_back();
for ( int i = 0; i < num_input_neurons ; ++i) {
    all_layers[0].emplace_back( v_threshold , e_leak , fixed_leak_per_step , reset_mech);
}


all_layers.emplace_back();
for ( int i = 0 ; i < num_hidden_neuron ; ++i ) {
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

double simulation_end_time = 2.0;

std::cout<<"---starting simulation---\n";






std::vector<LIFNeuron> input_layer;

std::vector<LIFNeuron> output_layer;
 
for (int i=0; i<num_input_neurons; ++i){
    input_layer.emplace_back(v_threshold,e_leak,fixed_leak_per_step,reset_mech);
}

for (int i=0; i<num_output_neurons; ++i){
    output_layer.emplace_back(v_threshold,e_leak,fixed_leak_per_step,reset_mech);
}

std::vector<std::vector<float>> weights_io(num_input_neurons,std::vector<float>(num_output_neurons));

weights_io[0][0]=0.6f;
weights_io[1][0]=0.1f;
weights_io[2][0]=0.5f;

weights_io[0][1]=0.2f;
weights_io[1][1]=0.7f;
weights_io[2][1]=0.3f;

int timestamp = 10;

std::cout<<"---starting test---\n";

for (int step = 0 ; step < timestamp ; ++step){

    std::cout<<"\nthis is "<< step<<" step";

    std::vector<float> input_to_output_layer_current(num_output_neurons,0.0f);

    std::vector<float> external_inputs(num_input_neurons,0.0f);

    if (step == 2){
        external_inputs[0] = 1.2f;
    }
    if (step == 5){
        external_inputs[1] = 1.5f;
    }
    if (step == 7){
        external_inputs[2] = 0.8f;
    }

    // std::cout<<"\nInput layer updates\n";
    std::cout<<"\n-----------------start input layer--------------------\n";

    for (int i=0 ; i < num_input_neurons ; ++i){

        bool fired = input_layer[i].update(external_inputs[i]);

        std::cout<<"Input neuron " << i+1 << ": "<< input_layer[i].get_membrane_potential();

        if(fired){

            std::cout<<"\nSPIKE ";

            for (int j = 0 ; j < num_output_neurons ; ++j){
                input_to_output_layer_current[j] += weights_io[i][j];
            }
        }
        std::cout<<std::endl;
    }

    std::cout<<"\n--------------------end of input layer-----------------\n";

    std::cout<<"\n-----------------start second layer--------------------\n";

    for (int i = 0 ; i < num_output_neurons ; ++i){

        bool fired = output_layer[i].update(input_to_output_layer_current[i]);

        std::cout<< "output neuron "<<i<<" Vm "<<output_layer[i].get_membrane_potential()<<std::endl;

        if(fired){
            std::cout<<"\nSPIKE\n";
        }

    }

}
std::cout<<"--end of simulation--";

    return 0 ;

}

