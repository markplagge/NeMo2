//
// Created by Mark Plagge on 7/16/20.
//

#ifndef NEMOTNG_VIRTUALCOREREPORTS_H
#define NEMOTNG_VIRTUALCOREREPORTS_H
#include <string>
#include <vector>
#include "../nemo_globals.h"
namespace nemo {

	struct VirtualCoreReports {
		unsigned long vcore_id;
		std::vector<std::string> reports;
	};
	void add_nos_message(const std::shared_ptr<VirtualCoreReports>& report, double real_time, int model_id, int nos_message_type);
	void save_messages(const std::shared_ptr<VirtualCoreReports>&report);
	void print_messages(const std::shared_ptr<VirtualCoreReports>& report);
		extern std::string filename;
}
#endif//NEMOTNG_VIRTUALCOREREPORTS_H
