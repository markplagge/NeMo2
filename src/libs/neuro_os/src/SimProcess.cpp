//
// Created by Mark Plagge on 3/11/20.
//

#include "SimProcess.h"
#include <iostream>
namespace neuro_os { namespace sim_proc {



        SimProcess from_json_factory(const nlohmann::json &j ){
            auto pid = j.at("PID").get<int>();
            auto needed_cores = j.at("needed_cores").get<int>();
            auto needed_run_time = j.at("needed_run_time").get<int>();
            auto scheduled_start_time = j.at("scheduled_start_time").get<double>();

            auto p = SimProcess(pid,needed_cores,needed_run_time,scheduled_start_time);
            return p;
        }


		bool SimProcess::operator==(const SimProcess &rhs) const {
			return PID == rhs.PID &&
					needed_cores == rhs.needed_cores &&
					needed_run_time == rhs.needed_run_time &&
					scheduled_start_time == rhs.scheduled_start_time &&
					total_wait_time == rhs.total_wait_time &&
					total_run_time == rhs.total_run_time &&
					current_state == rhs.current_state;

		}


		bool SimProcess::operator!=(const SimProcess &rhs) const {
			return !(rhs == *this);
		}


		void to_json(json &j, const SimProcess &p) {
			j = json{
					{"PID",                  p.PID},
					{"needed_cores",         p.needed_cores},
					{"needed_run_time",      p.needed_run_time},
					{"scheduled_start_time", p.scheduled_start_time},
					{"total_wait_time",      p.total_wait_time},
					{"total_run_time",       p.total_run_time},
					{"current_state",        p.current_state},
			};
		};

		void from_json(const json &j,  SimProcess &p){
			j.at("PID").get_to(p.PID);
			j.at("needed_cores").get_to(p.needed_cores);
			j.at("needed_run_time").get_to(p.needed_run_time);
			j.at("scheduled_start_time").get_to(p.scheduled_start_time);

		}



		void from_json(const json &j, const SimProcess &p);


		SimProcess from_json_factory(const nlohmann::json &j );


		SimProcess::SimProcess(int pid, int neededCores, int neededRunTime, double scheduledStartTime):PID(pid), needed_cores(neededCores), needed_run_time(neededRunTime),
																										  scheduled_start_time(scheduledStartTime) {
			total_run_time = 0;
			total_wait_time = 0;
			current_state = WAITING;
		}

		std::ostream &operator<<(std::ostream &os, const SimProcess &process) {
			os << "PID: " << process.PID << " needed_cores: " << process.needed_cores << " needed_run_time: "
			   << process.needed_run_time << " scheduled_start_time: " << process.scheduled_start_time
			   << " total_wait_time: " << process.total_wait_time << " total_run_time: " << process.total_run_time
			   << " current_state: " << process.current_state;
			return os;
		}

		PROC_STATE SimProcess::get_current_state() const {
			return current_state;
		}


		void SimProcess::set_current_state(PROC_STATE currentState) {
			current_state = currentState;
		}




		int SimProcess::get_pid() const {
			return PID;
		}


		int SimProcess::get_needed_cores() const {
			return needed_cores;
		}


		int SimProcess::get_needed_run_time() const {
			return needed_run_time;
		}


		double SimProcess::get_scheduled_start_time() const {
			return scheduled_start_time;
		}


		void SimProcess::system_tick() {
			switch(current_state){
			case WAITING:
				++total_wait_time;
				break;
			case RUNNING:
				++total_run_time;
				if (total_run_time == needed_run_time){
					current_state = COMPLETE;
				}else if(total_run_time > needed_run_time){
					std::cerr << "Run time at " << total_run_time << " but only needed " << needed_run_time << "\n";
				}
				break;
			case COMPLETE:
				break;
			}
		}


		int SimProcess::get_total_wait_time() const {
			return total_wait_time;
		}


		int SimProcess::get_total_run_time() const {
			return total_run_time;
		}

//        template <class T>
//        void from_json(const json &j, const SimProcess &p){
//            j.at("PID").get_to(p.PID);
//            j.at("needed_cores").get_to(p.needed_cores);
//            j.at("needed_run_time").get_to(p.needed_run_time);
//            j.at("scheduled_start_time").get_to(p.scheduled_start_time);
//            j.at("neuron_state_system").get_to(p.neuron_state_system);
//        }

} }