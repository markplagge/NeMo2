//
// Created by Mark Plagge on 6/19/20.
//

#ifndef NEMOTNG_NEMOCOREDEFS_H
#define NEMOTNG_NEMOCOREDEFS_H
#include "NemoCoreSchedulerLight.h"
#include "NemoCoreScheduler.h"
#include <ross.h>
namespace nemo {
	namespace neuro_system {


		class NemoCoreScheduler;
		class NemoCoreSchedulerLight;
//		void sched_core_init(NemoCoreScheduler* s, tw_lp* lp);
//		void sched_pre_run(NemoCoreScheduler* s, tw_lp* lp);
//		void sched_forward_event(NemoCoreScheduler* s, tw_bf* bf, nemo_message* m, tw_lp* lp);
//		void sched_reverse_event(NemoCoreScheduler* s, tw_bf* bf, nemo_message* m, tw_lp* lp);
//		void sched_core_commit(NemoCoreScheduler* s, tw_bf* bf, nemo_message* m, tw_lp* lp);
//		void sched_core_finish(NemoCoreScheduler* s, tw_lp* lp);
		tw_peid sched_map_linear(tw_lpid gid);
	}// namespace neuro_system
}// namespace nemo
#endif//NEMOTNG_NEMOCOREDEFS_H
