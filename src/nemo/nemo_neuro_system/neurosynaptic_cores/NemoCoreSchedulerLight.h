//
// Created by Mark Plagge on 7/25/20.
//

#ifndef NEMOTNG_NEMOCORESCHEDULERLIGHT_H
#define NEMOTNG_NEMOCORESCHEDULERLIGHT_H
#define RNG_START(lp) auto rng_count = lp->rng->count
#define RNG_END(lp) msg->random_call_count = (lp->rng->count - rng_count)
#include "../../../libs/neuro_os/src/NemoNosScheduler.h"
#include "../../mapping_functions.h"
#include "../../nemo_config/NemoConfig.h"
#include "../../nemo_globals.h"
#include "../../nemo_io/ModelFile.h"
#include "../../nemo_io/NemoCoreOutput.h"
#include "../../nemo_io/SpikeFile.h"
#include <configuru.hpp>
#include <neuro_os.h>
#include <ross.h>
#include <vector>
#include <visit_struct/visit_struct.hpp>
#include <visit_struct/visit_struct_intrusive.hpp>
#include "../../nemo_io/NemoCoreDebugOutput.h"
namespace nemo {
	extern config::NemoConfig* global_config;
	namespace neuro_system {
		using namespace neuro_os;

		/**
		 * NemoCoreSchedulerLight is the NOS based scheduler. Based on neuromorphic core, but
		 * uses the external pybind11 bindings to manage scheduler arbiter logic
		 * @ingroup nemo_cores
		 */
		class NemoCoreSchedulerLight : public NemoCoreDebugOutput<NemoCoreSchedulerLight> {
			/* Debug fields */
			std::ofstream debug_log;
			double last_active_time = 0;
			unsigned int current_scheduler_time = 0;
			unsigned long _dbg_dest_gid = -1;


			/* end debug fields
			 * Start of scheduler specific items */
			/* config flags and settings and state management */
			tw_lp *my_lp;
			tw_bf *my_bf;
			long rng_count = 0;
			unsigned int num_cores_in_sim;
			std::map<int, ModelFile> model_files;
			std::map<int, SpikeFile> spike_files;


			std::unique_ptr<neuro_os::NemoNosScheduler> main_arbiter;


			/* Arbiter management */
			void scheduler_tick(tw_bf *bf, nemo_message *m);
			void send_start_messages(const std::vector<ProcEvent>& starts);
			void send_stop_messages(const std::vector<ProcEvent>& stops);
			void send_input_spikes_to_cores(int model_id,int task_id);
			void init_models_and_scheduler();
			neuro_os::NemoNosScheduler *nos_scheduler;
			/*Reverse Arbiter Management */
			void rev_scheduler_tick(tw_bf *bf, nemo_message *m);
			void rev_send_start_messages();
			void rev_send_stop_messages();
			void rev_send_input_spikes_to_cores();
			void send_scheduler_tick();

			/*helper functions */
			void send_nos_control_message(nemo_message_type message_type, double offset, unsigned int dest_gid, int task_id, int model_id);




		public:
			/*static scheduler interfaces to ROSS */
			static void sched_core_init(void * s, tw_lp* lp);
			static void sched_pre_run(void * s, tw_lp* lp);
			static void sched_forward_event(void * s, tw_bf* bf, nemo_message* m, tw_lp* lp);
			static void sched_reverse_event(void * s, tw_bf* bf, nemo_message* m, tw_lp* lp);
			static void sched_core_commit(void * s, tw_bf* bf, nemo_message* m, tw_lp* lp);
			static void sched_core_finish(void * s, tw_lp* lp);

			unsigned long get_dbg_dest_gid() const;
			void set_dbg_dest_gid(unsigned long dbg_dest_gid);

			/* debug functions */
			void i_save_spike_record(int model_id, int task_id);
			template <typename EVT_TYPE>
			void i_save_control_event(EVT_TYPE event);
			void i_start_debug_io();
			void i_end_debug_io();
			void i_save_input_spike_range(int model_id, int task_id, int num_spikes_sent);
			/* end debug functions */

		};

	}
}


#endif//NEMOTNG_NEMOCORESCHEDULERLIGHT_H