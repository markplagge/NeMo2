//
// Created by Mark Plagge on 11/1/19.
//
#include "NemoNeuroCoreBase.h"
#include "../neuron_models/neuron_factory.h"
#include <string>
#include <utility>
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

		void NemoNeuroCoreBase::forward_heartbeat_handler() {
			evt_stat = BF_Event_Status::None;
			// Generic error checking:

			if (!heartbeat_sent && cur_message->message_type == HEARTBEAT) {
				tw_error(TW_LOC, "Got a heartbeat when no heartbeat was expected.\n");
			}
			auto heartbeat_rng = my_lp->rng->count;

			// current heartbeat management system

			// Spikes are where heartbeats are generated. If no heartbeat has been sent this tick, and this is a spike,
			// then we need to send a heartbeat scheduled for the end of this current epoch.
			if (cur_message->message_type == NEURON_SPIKE) {// this if statement is a double check on the calling function
				if (heartbeat_sent && cur_message->intended_neuro_tick > current_neuro_tick) {

					tw_error(TW_LOC, "Got a spike with an out of bounds tick.\n"
									 "Current core tick: %li\n"
									 "Current time: %Lf \n",
							 //this->cur_message->to_string().c_str(),
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
					previous_neuro_tick = current_neuro_tick;//lossy operation - check for reverse computation errors
					my_bf->c0 = 1;                           //big tick change
					//this->evt_stat = BF_Event_Status ::NS_Tick_Update  & this->evt_stat; // evt stat update
					this->evt_stat = add_evt_status(this->evt_stat, BF_Event_Status::NS_Tick_Update);
					current_neuro_tick = cur_message->intended_neuro_tick;
				}
				else if (current_neuro_tick == cur_message->intended_neuro_tick) {
					my_bf->c0 = 0;
				}
				else {
					tw_error(TW_LOC, "Invalid tick times: \n current_neuro_tick: %d \n",
							 this->current_neuro_tick);
				}

				this->evt_stat = BF_Event_Status::Spike_Rec | this->evt_stat;
				if (!this->heartbeat_sent) {
					my_bf->c1 = 1;
					this->evt_stat = add_evt_status(this->evt_stat, BF_Event_Status::Heartbeat_Sent);
					//this->evt_stat = BF_Event_Status::Heartbeat_Sent & this->evt_stat;
					this->heartbeat_sent = true;
					// send the heartbeat event
					this->send_heartbeat();
				}
				else {// some error conditions:
					if (cur_message->intended_neuro_tick != this->current_neuro_tick) {
						tw_error(TW_LOC,
								 "Got a spike intended for t %d, but heartbeat has been sent and LP is active at time %d.\n"
								 "Details:\n"
								 "CoreID: %i \n"
								 "Message Data:\n",

								 this->cur_message->intended_neuro_tick,
								 this->current_neuro_tick,
								 this->core_local_id);
					}
				}
			}
			else if (cur_message->message_type == HEARTBEAT){//Heartbeat message received
				evt_stat = BF_Event_Status::Heartbeat_Rec;
				//error check:
				//message is heartbeat - We need to call leak, fire, reset logic
				//but first we set the heartbeat event to false.
				my_bf->c2 = 1;
				this->heartbeat_sent = false;
				/**at this point, we are at neuro_tick t. We need to process the leak/reset/fire
	 * functions for neurons. Leaks will loop for (previous_leak_time -> 0.).
	 * however, this is the heartbeat management only function.
	 * Also, the current neuro tick needs to be incremented.
	 * Since forward event handler calls this function, here the state is updated
	 * to reflect the current simulation state. Once out of this function,
	 * the leak / fire / reset functions in the neurons are called.*/

				this->evt_stat = add_evt_status(this->evt_stat, BF_Event_Status::NS_Tick_Update);
				my_bf->c0 = 1;
				my_bf->c4 = 1;// C4 leak counts are updated.
				this->leak_needed_count = this->current_neuro_tick - this->last_leak_time;
				this->previous_neuro_tick = this->current_neuro_tick;
				// leak counter is ready. Last leak time needs to be updated as well.
				this->last_leak_time = this->previous_neuro_tick;
			}
		}

		/**
 * reverse_heartbeat_handler - reverse computation for heartbeat messages.
 */

		void NemoNeuroCoreBase::reverse_heartbeat_handler() {
		}

		void NemoNeuroCoreBase::send_heartbeat() {

			RNG_START(my_lp);

			auto now = tw_now(my_lp);
			//auto next_tick = get_next_neurosynaptic_tick(tw_now(my_lp));
			auto next_tick = 1.0;
			tw_event* heartbeat_event = tw_event_new(my_lp->gid, next_tick, my_lp);
			nemo_message* msg = (nemo_message*)tw_event_data(heartbeat_event);
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

		void NemoNeuroCoreBase::save_spike(nemo_message* m, long dest_core, long neuron_id, double current_time) {
			this->output_system->save_spike(m->source_core, dest_core, m->dest_axon, m->debug_time, current_time);
			this->output_system->output_handler->write();
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

		void NemoNeuroCoreBase::core_init(tw_lp* lp) {
			this->core_local_id = get_core_id_from_gid(lp->gid);
			this->has_self_firing_neuron = false;
			auto x = new NeMoPOSIXOut();
			auto out = new NemoCoreOutput(core_local_id, x);
			this->output_system = out;
			// Use the nemo global config to set up this core

		}

		void NemoNeuroCoreBase::forward_event(tw_bf* bf, nemo_message* m, tw_lp* lp) {
			this->cur_message = m;
			this->my_lp = lp;
			this->my_bf = bf;

			if (m->message_type == NOS_LOAD_MODEL) {
				//current_model = models[m->model_id];
				std::istringstream iss(m->update_message);
				std::string tdl_m = "CORE INIT";
				int check_nums = -1;
				this->current_model = this->models[m->model_id];
				this->init_current_model(m->update_message);
			}else {
				this->forward_heartbeat_handler();

				//				auto core_stat_cfg = configuru::parse_string(m->update_message,configuru::FORGIVING,"CORE LOAD");

				if (m->message_type == NEURON_SPIKE) {
					for (const auto& item : neuron_array) {
						item->integrate(m->dest_axon);
					}
				}
				if (m->message_type == HEARTBEAT) {
					//LEAK
					run_leaks();
					//Fire
					run_fires();
					//Reset
					run_resets();
				}
				else {
					tw_error(TW_LOC, "Error - got message %i in neuro core base", m->message_type);
				}
			}
		}

		void NemoNeuroCoreBase::reverse_event(tw_bf* bf, nemo_message* m, tw_lp* lp) {
		}

		void NemoNeuroCoreBase::core_commit(tw_bf* bf, nemo_message* m, tw_lp* lp) {
			if (save_spikes) {
				save_spike(m, core_local_id, m->dest_axon, tw_now(lp));
			}
		}

		void NemoNeuroCoreBase::pre_run(tw_lp* lp) {
		}

		void NemoNeuroCoreBase::core_finish(tw_lp* lp) {
		}

		void NemoNeuroCoreBase::cleanup_output() {
		}

		/**
 * run_leaks - iterates through the neurons and runs the leak function.
 */
		void NemoNeuroCoreBase::run_leaks() {
#ifdef OPEN_MP
#pragma omp parallel for
#endif
			for (int neuron_id = 0; neuron_id < global_config->neurons_per_core; neuron_id++) {
				for (int leak_num = 0; leak_num < leak_needed_count; leak_num++) {
					neuron_array[neuron_id]->leak();
				}
			}
		}
		void NemoNeuroCoreBase::run_fires() {
			int nid = 0;
			for (const auto& item : neuron_array) {

				auto did_fire = item->fire();
				if (did_fire) {
					this->evt_stat = add_evt_status(this->evt_stat, BF_Event_Status::Output_Spike_Sent);
					this->evt_stat = add_evt_status(this->evt_stat, BF_Event_Status::Spike_Sent);
					if(is_dest_interchip(nid)){
						/** @todo: add cross chip communication recording here */

					}

					if (not item->is_self_manage_spike_events()) {
						auto dest_gid = get_gid_from_core_local(neuron_dest_cores[nid],neuron_dest_axons[nid]);
						struct tw_event* spike = tw_event_new(dest_gid, get_neurosynaptic_tick(tw_now(my_lp)), my_lp);
						nemo_message* msg = (nemo_message*)tw_event_data(spike);
						msg->intended_neuro_tick = this->current_neuro_tick + 1;
						msg->message_type = NEURON_SPIKE;
						msg->nemo_event_status = as_integer(this->evt_stat);
						msg->source_core = this->core_local_id;
						msg->dest_axon = -1;
						msg->debug_time = tw_now(my_lp);
						tw_event_send(spike);
					}
					nid ++;

				}
			}
		}
		void NemoNeuroCoreBase::run_resets() {
			//! @todo: Need to enable openmp possibly for things
#ifdef OPEN_MP
#pragma omp parallel for
#endif
			for (const auto& neuron : neuron_array) {
				neuron->reset();
			}
		}
		bool NemoNeuroCoreBase::is_dest_interchip(int neuron_id) {
			if (global_config->total_chips  <= 1) {
				return false;
			}

			auto source_core = get_core_id_from_gid(this->my_lp->gid);
			auto dest_core = this->neuron_dest_cores[neuron_id];
			auto cores_per_chip = global_config->ns_cores_per_chip;

			unsigned int source_chip = source_core / cores_per_chip;
			unsigned int dest_chip = dest_core / cores_per_chip;
			return source_chip != dest_chip;
		}
		void NemoNeuroCoreBase::create_blank_neurons() {
			if(neurons_init){
				neuron_stack.push_back(neuron_array);
			}
			for(int i = 0; i < global_config->neurons_per_core; i ++){
				std::shared_ptr<NemoNeuronGeneric> neuron(get_new_neuron(this->my_core_type));
				this->neuron_array.push_back(neuron);
			}
		}
		void NemoNeuroCoreBase::init_current_model(std::string model_def) {

			//line-by-line init of neurons
			std::istringstream mdl_string(model_def);
			/** @todo: I know this is bad to parse the lines multiple times but I don't want to pass around configuru ojbects
			 */
			int check = -1;
			for (std::string line; std::getline(mdl_string, line); ){
				auto core_stat_cfg = configuru::parse_string(line.c_str(),configuru::FORGIVING,"CORE_INIT");
				if(check){
					auto new_core_type = get_core_enum_from_json((std::string)core_stat_cfg["type"]);
					this->my_core_type = new_core_type;
					create_blank_neurons();
					check ++;
				}
				auto neuron_id =(unsigned int) core_stat_cfg["localID"];

				this->neuron_array[neuron_id].get()->init_from_json_string(line);
			}




			}

			void NemoNeuroCoreBase::interrupt_running_model() {

			}
		void NemoNeuroCoreBase::resume_running_model() {

		}

	}// namespace neuro_system

}// namespace nemo