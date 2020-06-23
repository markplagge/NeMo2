//
// Created by Mark Plagge on 6/18/20.
//

#include "NemoCoreScheduler.h"

void nemo::neuro_system::NemoCoreScheduler::forward_scheduler_event(tw_bf* bf, nemo::nemo_message* m, tw_lp* lp) {
	//Scheduler events are only once per global tick
	//first queue up next scheduler event
	if (global_config->do_neuro_os) {
		auto sched_time = JITTER(lp->rng) + 1;
		struct tw_event* os_tick = tw_event_new(lp->gid, sched_time, my_lp);
		nemo_message* msg = (nemo_message*)tw_event_data(os_tick);
		msg->message_type = NOS_TICK;
		msg->debug_time = tw_now(lp);
		msg->random_call_count = lp->rng->count;
		tw_event_send(os_tick);
		//update state
		this->current_neuro_tick = floor(tw_now(lp));
		this->process_queue.system_tick();
	}
	//check the current list of processes:
}
void nemo::neuro_system::NemoCoreScheduler::reverse_scheduler_event(tw_bf* bf, nemo::nemo_message* m, tw_lp* lp){

}
void nemo::neuro_system::NemoCoreScheduler::queue_up_process(neuro_os::sim_proc::SimProc p){

}

void nemo::neuro_system::NemoCoreScheduler::set_task_list(const std::vector<nemo::config::ScheduledTask>& task_list)
{
	NemoCoreScheduler::task_list = task_list;
}
void nemo::neuro_system::NemoCoreScheduler::init_process_models() {

	test_map[32] = std::string("ASDF");
	auto l_task_list = global_config->scheduler_inputs;
	auto l_model_list = global_config->models;
	for (const auto& model : l_model_list) {
		auto model_id = model.id;

		models.emplace(model_id, model);
	}
	this->set_models(models);
	this->set_task_list(task_list);
	for (const auto& task : task_list) {
		auto start_time = task.start_time;
		auto task_id = task.task_id;

		neuro_os::sim_proc::SimProc proc;
		auto model = models[task_id];
		proc.needed_cores = model.needed_cores;
		proc.PID = model.id;
		proc.needed_run_time = model.requested_time;
		proc.scheduled_start_time = start_time;
		process_queue.enqueue(proc);

	}
}
void nemo::neuro_system::NemoCoreScheduler::check_waiting_procs(){

}

void nemo::neuro_system::NemoCoreScheduler::set_models(const std::map<int, nemo::config::NemoModel>& models) {
	NemoCoreScheduler::models = models;
}

void nemo::neuro_system::sched_core_init(NemoCoreScheduler *s, tw_lp* lp){
	new (s) NemoCoreScheduler();
	s->my_lp = lp;
	auto sched_time =  JITTER(lp->rng) + 1;
	struct tw_event* os_tick = tw_event_new(lp->gid, sched_time, lp);
	auto* msg = (nemo_message*)tw_event_data(os_tick);
	msg->message_type = NOS_TICK;
	msg->debug_time = tw_now(lp);
	msg->random_call_count = lp->rng->count;
	tw_event_send(os_tick);
	s->init_process_models();

}
void nemo::neuro_system::sched_pre_run(nemo::neuro_system::NemoCoreScheduler* s, tw_lp* lp){

}
/**
 * on forward events call the parent class
 * @param s
 * @param bf
 * @param m
 * @param lp
 */
void nemo::neuro_system::sched_forward_event(nemo::neuro_system::NemoCoreScheduler* s, tw_bf* bf, nemo::nemo_message* m, tw_lp* lp){
	s->forward_scheduler_event(bf,m,lp);
}
void nemo::neuro_system::sched_reverse_event(nemo::neuro_system::NemoCoreScheduler* s, tw_bf* bf, nemo::nemo_message* m, tw_lp* lp){
	s->reverse_scheduler_event(bf,m,lp);
}
void nemo::neuro_system::sched_core_commit(nemo::neuro_system::NemoCoreScheduler* s, tw_bf* bf, nemo::nemo_message* m, tw_lp* lp){
	s->core_commit(bf,m,lp);
}
void nemo::neuro_system::sched_core_finish(nemo::neuro_system::NemoCoreScheduler* s, tw_lp* lp){
	s->core_finish(lp);
}
