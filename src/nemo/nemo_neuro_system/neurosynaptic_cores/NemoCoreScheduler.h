//
// Created by Mark Plagge on 6/18/20.
//

#ifndef NEMOTNG_NEMOCORESCHEDULER_H
#define NEMOTNG_NEMOCORESCHEDULER_H
#include "../../nemo_io/ModelFile.h"
#include "../../nemo_io/SpikeFile.h"
#include "NemoNeuroCoreBase.h"
#include <configuru.hpp>
#include <neuro_os.h>
#include <visit_struct/visit_struct.hpp>
#include <visit_struct/visit_struct_intrusive.hpp>
namespace nemo {
	namespace neuro_system {
		class NemoCoreScheduler : public NemoNeuroCoreBase {
		public:
			neuro_os::sim_proc::SimProcessQueue process_queue;
			double last_active_time = 0;

			std::vector<nemo::config::ScheduledTask> task_list;

			void set_models(const std::map<int, nemo::config::NemoModel>& models);
			void forward_scheduler_event(tw_bf *bf, nemo_message *m, tw_lp *lp);
			void reverse_scheduler_event(tw_bf *bf, nemo_message *m, tw_lp *lp);
			void queue_up_process(neuro_os::sim_proc::SimProc p);

			void set_task_list(const std::vector<nemo::config::ScheduledTask>& task_list);

			void init_process_models();

			void check_waiting_procs();
		};
		// This class gets directly init'ed by ROSS through these functions:
		void sched_core_init(NemoCoreScheduler * s, tw_lp* lp);
		//void sched_core_init(void* s, tw_lp* lp);
		void sched_pre_run(NemoCoreScheduler * s, tw_lp* lp);
		void sched_forward_event(NemoCoreScheduler * s, tw_bf* bf, nemo_message* m, tw_lp* lp);
		void sched_reverse_event(NemoCoreScheduler * s, tw_bf* bf, nemo_message* m, tw_lp* lp);
		void sched_core_commit(NemoCoreScheduler * s, tw_bf* bf, nemo_message* m, tw_lp* lp);
		void sched_core_finish(NemoCoreScheduler * s, tw_lp* lp);





	}








}

#endif//NEMOTNG_NEMOCORESCHEDULER_H
