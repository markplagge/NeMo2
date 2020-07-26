//
// Created by Mark Plagge on 7/26/20.
//

#ifndef NEMOTNG_NEMOCOREDEBUGOUTPUT_H
#define NEMOTNG_NEMOCOREDEBUGOUTPUT_H
#include "../nemo_globals.h"
#include <fstream>
#include <string>

#include <sstream>
namespace nemo {
	bool ends_with(const std::string & main_str, const std::string & to_match);
	template <typename T>
	struct NemoCoreDebugOutput : crtp<T> {
		std::string debug_filename;
		bool is_file_open=false;
		std::ofstream output_file;
		const char *core_type;

		/** save_spike_record - when a spike is sent from this core, save it to the file */
		void save_spike_record( int model_id, int task_id,bool use_impl){
			if(use_impl) {
				this->underlying().i_save_spike_record(model_id, task_id);
			}else{
				this->output_file << "SPIKE,"<< model_id << "," << task_id <<"\n";
			}
		}
		/** save_control_event - Saves a NOS control event to the debug log */



		template<typename PROC_EVENT_TYPE>
		void save_control_event(PROC_EVENT_TYPE event) {
			this->underlying().i_save_control_event(event);
		}
		void save_input_spike_range(int model_id, int task_id, int num_spikes_sent) {
			this->underlying().i_save_input_spike_range(model_id, task_id, num_spikes_sent);
		}

		/** starts up this core's debug system */
		void start_debug_io(const std::string& filename,const std::string& optional_header,const char *in_core_type){
			{
				this->debug_filename = filename;
				if (!this->output_file.is_open()) {
					output_file.open(filename);
				}
				this->core_type = in_core_type;
				if(optional_header.empty()){
					this->underlying().i_start_debug_io();
				}else{
					output_file << optional_header;
					if (!ends_with(optional_header, "\n")){
						output_file << "\n";
					}
				}
			}
		}
		/** ends this core's deubg system */
		void end_debug_io(){
			if(this->underlying().output_file.is_open()){
				output_file.close();
			}
		}

	};

	template <typename T>
	struct NemoCoreMessageDebugOutput : crtp<T> {


	};

	namespace util {
		template<typename ELE_T>
		std::string assemble_csv_elements(ELE_T element ){
			std::stringstream x;
			x << element;
			return x.str();
		}
		template<typename ELE_T, typename... ELE_ARGS>
		std::string assemble_csv_elements(ELE_T element, ELE_ARGS... element_args ){
			std::stringstream s;
			s << element;
			s << "," << assemble_csv_elements(element_args...);
			return s.str();

		}






	}//namespace util
}//namespace nemo

#endif//NEMOTNG_NEMOCOREDEBUGOUTPUT_H
