//
// Created by Mark Plagge on 6/18/20.
//

#ifndef NEMOTNG_NEMOCORESCHEDULER_H
#define NEMOTNG_NEMOCORESCHEDULER_H
#include "NemoNeuroCoreBase.h"
#include <neuro_os.h>
#include <configuru.hpp>
namespace nemo {
	namespace neuro_system {
		class NemoCoreScheduler : NemoNeuroCoreBase {
		protected:
			neuro_os::sim_proc::SimProcessQueue process_queue;
			double last_active_time = 0;
			std::vector <nemo::config::NemoModel> models;
			std::vector <nemo::config::ScheduledTask> task_list;
		public:
			void forward_scheduler_event(tw_bf *bf, nemo_message *m, tw_lp *lp);
			void reverse_scheduler_event(tw_bf *bf, nemo_message *m, tw_lp *lp);
			void queue_up_process(neuro_os::sim_proc::SimProc p);
			void set_models(const std::vector<nemo::config::NemoModel>& models);
			void set_task_list(const std::vector<nemo::config::ScheduledTask>& task_list);



		};
		// This class gets directly init'ed by ROSS through these functions:
		void sched_core_init(NemoCoreScheduler * s, tw_lp* lp);
		void sched_pre_run(NemoCoreScheduler * s, tw_lp* lp);
		void sched_forward_event(NemoCoreScheduler * s, tw_bf* bf, nemo_message* m, tw_lp* lp);
		void sched_reverse_event(NemoCoreScheduler * s, tw_bf* bf, nemo_message* m, tw_lp* lp);
		void sched_core_commit(NemoCoreScheduler * s, tw_bf* bf, nemo_message* m, tw_lp* lp);
		void sched_core_finish(NemoCoreScheduler * s, tw_lp* lp);
		tw_peid sched_map_linear(tw_lpid gid);
	}








}

#endif//NEMOTNG_NEMOCORESCHEDULER_H
