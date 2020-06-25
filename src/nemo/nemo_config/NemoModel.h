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
		};

	}
 }
VISITABLE_STRUCT(nemo::config::NemoModel, id, needed_cores, model_file_path, spike_file_path, requested_time, benchmark_model_name);




#endif//NEMOTNG_NEMOMODEL_H
