//
// Created by Mark Plagge on 11/1/19.
//
#include <string>
#include <utility>
#include "NemoNeuroCoreBase.h"
namespace nemo {
    namespace neuro_system {

/**
 * Manages a heartbeat message. If this LP has not sent a heartbeat message, send it, and set the
 * heartbeat status to true.
 * If cur_message->message_type is NEURON_SPIKE, then this sets heartbeat_sent to false.
 *
 * !Note: using the my_bf (set from the calling ROSS function to tw_bf *bf) here:
 * c0 = NS Tick update from some value. We loose whatever value was in previous neuro tick.
 * c1 = heartbeat_sent was set to true (implies NEURON_SPIKE message type)
 * c2 = heartbeat_sent was set to false (implies HEARTBEAT message type)
 * c3 =
 *
 * Heartbeat logic:
 * At time t + e, core C receives a spike message.
 * If C has not sent a heartbeat, send one scheduled for t + 1.
 * If C has sent a heartbeat, then just integrate.
 * Last neurosynaptic tick time is stored in previous neuro tick: t_p
 *
 * At time t + 1, core C receives a heartbeat message.
 * C sets current neurosynaptic tick to t + 1.
 * C runs the leak function for every nerusynaptic tick between last_leak_time and current_neuro tick.
 * C sets the
 * C runs fire/reset function
 */
        template<typename NEURO_WEIGHT_TYPE>
        void NemoNeuroCoreBase<NEURO_WEIGHT_TYPE>::forward_heartbeat_handler() {

            // Generic error checking:

            if (!heartbeat_sent && cur_message->message_type == HEARTBEAT) {
                tw_error(TW_LOC, "Got a heartbeat when no heartbeat was expected.\n");
            }
            auto heartbeat_rng = my_lp->rng->count;


            // current heartbeat management system

            // Spikes are where heartbeats are generated. If no heartbeat has been sent this tick, and this is a spike,
            // then we need to send a heartbeat scheduled for the end of this current epoch.
            if (cur_message->message_type ==
                NEURON_SPIKE) { // this if statement is a double check on the calling function
                if (heartbeat_sent && cur_message->intended_neuro_tick > current_neuro_tick) {

                    tw_error(TW_LOC, "Got a spike with an out of bounds tick.\n %s\n"

                                     "Current core tick: %li\n"
                                     "Current time: %Lf \n", this->cur_message->to_string().c_str(),
                             this->current_neuro_tick,
                             tw_now(this->my_lp));
                }
                evt_stat = BF_Event_Status::Spike_Rec;
                /**
                 * from tick 0->1:
                 * current_neuro_tick = previous_neuro_tick = 0;
                 * messages come in from t= 0.0...1 to t = 0.9;
                 * if we get a spike and current_neuro_tick is < t:
                 *  t = next_neuro_tick (gathered from the intended neuro tick in the message)
                 * if no heartbeat is sent:
                 *  heartbeat scheduled for t = 1
                 * --
                 * if message is a heartbeat:
                 * if heartbeat intended neuro tick == current_neuro_tick:
                 *  leak_needed_count = current_neuro_tick - last_leak_time
                 *  previous_neuro_tick = current_neuro_tick
                 *  last_leak_time = current_neuro_tick  <- this could be updated, but left in for possible different ways of calculating leak_needed_count
                 *  do leak, reset, fire funs.
                 *  fire messages are scheduled for current_neuro_tick + delay + JITTER (but this is handled by the implementation of this class)
                 * else:
                 *  This is an error condition.
                 *
                 */
                /**neurosynaptic tick manager: @todo: may want to move this to an external function, as the ticks might be different depending on the underlying model. */
                if (current_neuro_tick < cur_message->intended_neuro_tick) {
                    previous_neuro_tick = current_neuro_tick; //lossy operation - check for reverse computation errors
                    my_bf->c0 = 1; //big tick change
                    //this->evt_stat = BF_Event_Status ::NS_Tick_Update  & this->evt_stat; // evt stat update
                    this->evt_stat = add_evt_status(this->evt_stat, BF_Event_Status::NS_Tick_Update);
                    current_neuro_tick = cur_message->intended_neuro_tick;

                } else if (current_neuro_tick == cur_message->intended_neuro_tick) {
                    my_bf->c0 = 0;
                } else {
                    tw_error(TW_LOC, "Invalid tick times:\nMsg Data:\n %s \n current_neuro_tick: %d \n",
                             cur_message->to_string().c_str(), this->current_neuro_tick);
                }


                this->evt_stat = BF_Event_Status::Spike_Rec | this->evt_stat;
                if (!this->heartbeat_sent) {
                    my_bf->c1 = 1;
                    this->evt_stat = add_evt_status(this->evt_stat, BF_Event_Status::Heartbeat_Sent);
                    //this->evt_stat = BF_Event_Status::Heartbeat_Sent & this->evt_stat;
                    this->heartbeat_sent = true;
                    // send the heartbeat event
                    this->send_heartbeat();

                } else {// some error conditions:
                    if (cur_message->intended_neuro_tick != this->current_neuro_tick) {
                        tw_error(TW_LOC,
                                 "Got a spike intended for t %d, but heartbeat has been sent and LP is active at time %d.\n"
                                 "Details:\n"
                                 "CoreID: %i \n"
                                 "Message Data:\n"
                                 "source_core,dest_axon,intended_neuro_tick,nemo_event_status,"
                                 "random_call_count,debug_time %s\n", this->cur_message->intended_neuro_tick,
                                 this->current_neuro_tick, this->core_local_id, cur_message->to_string().c_str());

                    }
                }
            } else {
                evt_stat = BF_Event_Status::Heartbeat_Rec;
                //error check:
                //message is heartbeat - We need to call leak, fire, reset logic
                //but first we set the heartbeat event to false.
                my_bf->c2 = 1;
                this->heartbeat_sent = false;
                //at this point, we are at neuro_tick t. We need to process the leak/reset/fire
                //functions for neurons. Leaks will loop for (previous_leak_time -> 0.).
                //however, this is the heartbeat management only function.
                //Also, the current neuro tick needs to be incremented.
                this->evt_stat = add_evt_status(this->evt_stat, BF_Event_Status::NS_Tick_Update);
                my_bf->c0 = 1;
                my_bf->c4 = 1; // C4 leak counts are updated.
                this->leak_needed_count = this->current_neuro_tick - this->last_leak_time;
                this->previous_neuro_tick = this->current_neuro_tick;
                // leak counter is ready. Last leak time needs to be updated as well.
                this->last_leak_time = this->previous_neuro_tick;
            }

        }

/**
 * reverse_heartbeat_handler - reverse computation for heartbeat messages.
 */
        template<typename NEURO_WEIGHT_TYPE>
        void NemoNeuroCoreBase<NEURO_WEIGHT_TYPE>::reverse_heartbeat_handler() {


        }

        template<typename NEURO_WEIGHT_TYPE>
        void NemoNeuroCoreBase<NEURO_WEIGHT_TYPE>::send_heartbeat() {

            RNG_START(my_lp);

            auto now = tw_now(my_lp);
            //auto next_tick = get_next_neurosynaptic_tick(tw_now(my_lp));
            auto next_tick = 1.0;
            tw_event *heartbeat_event = tw_event_new(my_lp->gid, next_tick, my_lp);
            nemo_message *msg = (nemo_message *) tw_event_data(heartbeat_event);
            msg->intended_neuro_tick = next_tick;
            msg->message_type = HEARTBEAT;
            msg->debug_time = now;
            msg->nemo_event_status = as_integer(this->evt_stat);
            // Add some extra info to the messagE:
            msg->source_core = core_local_id;
            msg->dest_axon = -1;

            RNG_END(my_lp);

            tw_event_send(heartbeat_event);


        }

        template<typename NEURO_WEIGHT_TYPE>
        void NemoNeuroCoreBase<NEURO_WEIGHT_TYPE>::save_spike(nemo_message *m, long dest_core, long neuron_id) {

//    SpikeData s = {0};
//    s.source_core = core_local_id;
//    s.dest_axon = m->dest_axon;
//    s.source_neuro_tick = current_neuro_tick;
//    s.dest_neuro_tick = m->intended_neuro_tick;
//    s.dest_core = dest_core;
//    s.source_neuron = neuron_id;
//    s.tw_source_time = tw_now(my_lp);
//
//    this->spike_output->save_spike(s);
        }

        template<typename NEURO_WEIGHT_TYPE>
        void NemoNeuroCoreBase<NEURO_WEIGHT_TYPE>::core_init(tw_lp *lp) {
            for (int i = 0; i < NEURONS_PER_CORE; i++) {
                auto new_neu =
            }

        }

        template<typename NEURO_WEIGHT_TYPE>
        void NemoNeuroCoreBase<NEURO_WEIGHT_TYPE>::forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) {

        }

        template<typename NEURO_WEIGHT_TYPE>
        void NemoNeuroCoreBase<NEURO_WEIGHT_TYPE>::reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp) {

        }

        template<typename NEURO_WEIGHT_TYPE>
        void NemoNeuroCoreBase<NEURO_WEIGHT_TYPE>::core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp) {

        }

        template<typename NEURO_WEIGHT_TYPE>
        void NemoNeuroCoreBase<NEURO_WEIGHT_TYPE>::pre_run(tw_lp *lp) {

        }

        template<typename NEURO_WEIGHT_TYPE>
        void NemoNeuroCoreBase<NEURO_WEIGHT_TYPE>::core_finish(tw_lp *lp) {

        }

        template<typename NEURO_WEIGHT_TYPE>
        void NemoNeuroCoreBase<NEURO_WEIGHT_TYPE>::cleanup_output() {

        }


    }

}