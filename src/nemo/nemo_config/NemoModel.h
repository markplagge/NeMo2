//
// Created by Mark Plagge on 6/23/20.
//

#ifndef NEMOTNG_NEMOMODEL_H
#define NEMOTNG_NEMOMODEL_H
#include <string>
#include <visit_struct/visit_struct.hpp>
#include <visit_struct/visit_struct_intrusive.hpp>
/**
 * NeMoModel -
 * Struct containing parameters for a particular model, file paths for the model configs / input spikes,
 * a string that determines if it is a benchmark network and what network it is.
 *
 * NemoCoreScheduler takes a vector of these, and uses them to manage the simulation.
 */
 namespace nemo {
	namespace config {
		struct NemoModel {

			int id;
			int needed_cores;
			std::string model_file_path;
			std::string spike_file_path;
			double requested_time;
			std::string benchmark_model_name;

			std::string to_string() const;
			friend std::ostream& operator<<(std::ostream& os, const NemoModel& model);

			/**
			 * is_continuous - If a scheduled task has a requested_time value of less than zero,
			 * then it is a model that should run forever. This is different than a model with
			 * requested time of 0, which means that we don't know how long the model will run for.
			 * @see has_known_end_time()
			 * @return true if the model should run forever
			 */
			bool is_continuous(){
				return (requested_time < 0);
			}

			/**
			 * has_known_end_time() : if a scheduled task has a requested_time of > 0, then
			 * it will run for that many ticks. If the requested_time is zero, then the process
			 * doesn't know how long it might need to run for.
			 *
			 * Not yet implemented.
			 * @return
			 */
			bool has_known_end_time(){
				return (requested_time == 0);
			}
		};

	}
 }
VISITABLE_STRUCT(nemo::config::NemoModel, id, needed_cores, model_file_path, spike_file_path, requested_time, benchmark_model_name);




#endif//NEMOTNG_NEMOMODEL_H
