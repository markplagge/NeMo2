//
// Created by Mark Plagge on 3/11/20.
//

#include "SimProcess.h"
#include <iostream>
namespace neuro_os { namespace sim_proc {


        template <class T>
		SimProcess<T> from_json_factory(const nlohmann::json& j)
		{
			auto pid = j.at("PID").get<int>();
			auto needed_cores = j.at("needed_cores").get<int>();
			auto needed_run_time = j.at("needed_run_time").get<int>();
			auto scheduled_start_time = j.at("scheduled_start_time").get<double>();

			auto p = SimProcess<T>(pid, needed_cores, needed_run_time, scheduled_start_time);
			return p;
		}

		template<class T>
		bool SimProcess<T>::operator==(const SimProcess& rhs) const
		{
			return PID == rhs.PID && needed_cores == rhs.needed_cores && needed_run_time == rhs.needed_run_time && scheduled_start_time == rhs.scheduled_start_time && total_wait_time == rhs.total_wait_time && total_run_time == rhs.total_run_time && current_state == rhs.current_state;
		}

		template<class T>
		bool SimProcess<T>::operator!=(const SimProcess& rhs) const
		{
			return !(rhs == *this);
		}

		template<class T>
		void to_json(json& j, const SimProcess<T>& p)
		{
			j = json{
					{"PID", p.PID},
					{"needed_cores", p.needed_cores},
					{"needed_run_time", p.needed_run_time},
					{"scheduled_start_time", p.scheduled_start_time},
					{"total_wait_time", p.total_wait_time},
					{"total_run_time", p.total_run_time},
					{"current_state", p.current_state},
			};
		};
		template<class T>
		void from_json(const json& j, SimProcess<T>& p)
		{
			j.at("PID").get_to(p.PID);
			j.at("needed_cores").get_to(p.needed_cores);
			j.at("needed_run_time").get_to(p.needed_run_time);
			j.at("scheduled_start_time").get_to(p.scheduled_start_time);
		}

		template<class T>
		void from_json(const json& j, const SimProcess<T>& p);

		template<class T>
		SimProcess<T> from_json_factory(const nlohmann::json& j);

		template<class T>
		SimProcess<T>::SimProcess(int pid, int neededCores, int neededRunTime, double scheduledStartTime) : PID(pid), needed_cores(neededCores), needed_run_time(neededRunTime),
																											scheduled_start_time(scheduledStartTime)
		{
			total_run_time = 0;
			total_wait_time = 0;
			current_state = WAITING;
		}
		template<class T>
		std::ostream& operator<<(std::ostream& os, const SimProcess<T>& process)
		{
			os << "PID: " << process.PID << " needed_cores: " << process.needed_cores << " needed_run_time: "
			   << process.needed_run_time << " scheduled_start_time: " << process.scheduled_start_time
			   << " total_wait_time: " << process.total_wait_time << " total_run_time: " << process.total_run_time
			   << " current_state: " << process.current_state;
			return os;
		}
		template<class T>
		PROC_STATE SimProcess<T>::get_current_state() const
		{
			return current_state;
		}

		template<class T>
		void SimProcess<T>::set_current_state(PROC_STATE currentState)
		{
			current_state = currentState;
		}

		template<class T>
		int SimProcess<T>::get_pid() const
		{
			return PID;
		}

		template<class T>
		int SimProcess<T>::get_needed_cores() const
		{
			return needed_cores;
		}

		template<class T>
		int SimProcess<T>::get_needed_run_time() const
		{
			return needed_run_time;
		}

		template<class T>
		double SimProcess<T>::get_scheduled_start_time() const
		{
			return scheduled_start_time;
		}

		template<class T>
		void SimProcess<T>::system_tick()
		{
			switch (current_state) {
			case WAITING:
				++total_wait_time;
				break;
			case RUNNING:
				++total_run_time;
				if (total_run_time == needed_run_time) {
					current_state = COMPLETE;
				}
				else if (total_run_time > needed_run_time) {
					std::cerr << "Run time at " << total_run_time << " but only needed " << needed_run_time << "\n";
				}
				break;
			case COMPLETE:
				break;
			}
		}

		template<class T>
		int SimProcess<T>::get_total_wait_time() const
		{
			return total_wait_time;
		}

		template<class T>
		int SimProcess<T>::get_total_run_time() const
		{
			return total_run_time;
		}

		//        template <class T>
		//        void from_json(const json &j, const SimProcess<T> &p){
		//            j.at("PID").get_to(p.PID);
		//            j.at("needed_cores").get_to(p.needed_cores);
		//            j.at("needed_run_time").get_to(p.needed_run_time);
		//            j.at("scheduled_start_time").get_to(p.scheduled_start_time);
		//            j.at("neuron_state_system").get_to(p.neuron_state_system);
		//        }

	} }