//
// Created by Mark Plagge on 6/18/20.
//

#include "NemoCoreScheduler.h"
void nemo::neuro_system::NemoCoreScheduler::forward_scheduler_event(tw_bf* bf, nemo::nemo_message* m, tw_lp* lp) {

}
void nemo::neuro_system::NemoCoreScheduler::reverse_scheduler_event(tw_bf* bf, nemo::nemo_message* m, tw_lp* lp){

}
void nemo::neuro_system::NemoCoreScheduler::queue_up_process(neuro_os::sim_proc::SimProc p){

}
void nemo::neuro_system::NemoCoreScheduler::set_models(const std::vector<nemo::config::NemoModel>& models)
{
	NemoCoreScheduler::models = models;
}
void nemo::neuro_system::NemoCoreScheduler::set_task_list(const std::vector<nemo::config::ScheduledTask>& task_list)
{
	NemoCoreScheduler::task_list = task_list;
}

void nemo::neuro_system::sched_core_init(nemo::neuro_system::NemoCoreScheduler* s, tw_lp* lp)
{
}
void nemo::neuro_system::sched_pre_run(nemo::neuro_system::NemoCoreScheduler* s, tw_lp* lp)
{
}
void nemo::neuro_system::sched_forward_event(nemo::neuro_system::NemoCoreScheduler* s, tw_bf* bf, nemo::nemo_message* m, tw_lp* lp)
{
}
void nemo::neuro_system::sched_reverse_event(nemo::neuro_system::NemoCoreScheduler* s, tw_bf* bf, nemo::nemo_message* m, tw_lp* lp)
{
}
void nemo::neuro_system::sched_core_commit(nemo::neuro_system::NemoCoreScheduler* s, tw_bf* bf, nemo::nemo_message* m, tw_lp* lp)
{
}
void nemo::neuro_system::sched_core_finish(nemo::neuro_system::NemoCoreScheduler* s, tw_lp* lp)
{
}
tw_peid nemo::neuro_system::sched_map_linear(tw_lpid gid)
{
	return 0;
}
