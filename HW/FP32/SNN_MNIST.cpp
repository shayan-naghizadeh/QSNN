#include <iostream> 


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





    bool update(float input_current){
        //first leakage second check for spike
        has_fired_in_current_steps = false;

        if (memberane_potential > leak_reversal_potential){

            memberane_potential -= fixed_leak_amount_per_step;

            if (memberane_potential < leak_reversal_potential){

                memberane_potential = leak_reversal_potential;
            }
        }else if (memberane_potential < leak_reversal_potential){

            memberane_potential += fixed_leak_amount_per_step;
        
            if (memberane_potential > leak_reversal_potential){

                memberane_potential = leak_reversal_potential;
            }
        }



        memberane_potential += input_current;

        if ( memberane_potential >= threshold_voltage){
            has_fired_in_current_steps = true;

            if (current_reset_mechanism == SUBTRACT_THERSHOLD){

                memberane_potential -= threshold_voltage;

            }   else if(current_reset_mechanism == REST_TO_ZERO){

                memberane_potential = 0.0f;
            }

        }
        return has_fired_in_current_steps;
    }

    float get_membrane_potential() const {

        return memberane_potential;
    }

    };

int main(){


LIFNeuron firstNeuron;
std::cout<<firstNeuron.get_membrane_potential();
std::cout<<"\n"<<firstNeuron.update(0.40);
std::cout<<"\n"<<firstNeuron.get_membrane_potential();
std::cout<<"\n"<<firstNeuron.update(0);
std::cout<<"\n"<<firstNeuron.get_membrane_potential();

    return 0 ;

}

