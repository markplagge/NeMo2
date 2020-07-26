//
// Created by Mark Plagge on 7/26/20.
//

#include "NemoCoreDebugOutput.h"
namespace nemo{

	/*
 * Case Sensitive Implementation of ends_with()
 * It checks if the string 'mainStr' ends with given string 'toMatch'
 */
	bool ends_with(const std::string & main_str, const std::string & to_match){
		if(main_str.size() >= to_match.size() && main_str.compare(main_str.size() - to_match.size(), to_match.size(), to_match) == 0)
			return true;
		else
			return false;
	}


//	template<typename T>
//	void NemoCoreDebugOutput<T>::save_spike_record(int model_id, int task_id, bool use_impl) {
//		if(use_impl) {
//			this->underlying()->i_save_spike_record(model_id, task_id);
//		}else{
//			this->output_file << "SPIKE,"<< model_id << "," << task_id <<"\n";
//		}
//	}
//	template<typename T>
//	void NemoCoreDebugOutput<T>::start_debug_io(const std::string& filename,const std::string& optional_header,
//												const char *in_core_type){
//		this->debug_filename = filename;
//		if (!this->output_file.is_open()) {
//			output_file.open(debug_filename);
//		}
//		this->core_type = in_core_type;
//		if(optional_header.empty()){
//			this->underlying()->i_start_debug_io();
//		}else{
//			output_file << optional_header;
//			if (!ends_with(optional_header, "\n")){
//				output_file << "\n";
//			}
//		}
//	}

//	template<typename T>
//	void NemoCoreDebugOutput<T>::end_debug_io() {
//		this->underlying()->i_end_debug_io();
//		if(this->output_file.is_open()){
//			output_file.close();
//		}
//	}

//	template<typename T>
//	template<typename PROC_EVENT_TYPE>
//	void NemoCoreDebugOutput<T>::save_control_event(PROC_EVENT_TYPE event) {
//
//
//		this->underlying()->i_save_control_event(event);
//	}
//	template<typename T>
//	void NemoCoreDebugOutput<T>::save_input_spike_range(int model_id, int task_id, int num_spikes_sent) {
//		this->underlying()->i_save_input_spike_range(model_id, task_id, num_spikes_sent);
//	}

}