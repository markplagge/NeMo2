//
// Created by Mark Plagge on 7/16/20.
//

#include "VirtualCoreReports.h"
#include <sstream>
#include <iostream>
#include <fstream>
namespace nemo {
	std::string filename = std::string("_") + std::to_string(g_tw_mynode) + "vcore_report.rpt";;
#define X(a) #a,
	static const char* nemo_message_type_strings[] = {NEMO_MESSAGE_TYPES};
#undef X

	void add_nos_message(const std::shared_ptr<VirtualCoreReports>& report, double real_time, int model_id, int nos_message_type) {
		std::stringstream s;
		s << "NOS MESSAGE " << nemo_message_type_strings[nos_message_type] << " AT " << real_time << " MID: " << model_id;
		report->reports.push_back(s.str());
	}
	void print_messages(const std::shared_ptr<VirtualCoreReports>& report) {
		for (const auto& report_t : report->reports) {
			std::cout << report_t << "\n";
		}
	}
	void save_messages(const std::shared_ptr<VirtualCoreReports>&report){
		std::string my_filename = std::to_string(report->vcore_id) + filename;
		std::ofstream myfile;
		myfile.open(my_filename);
		for (const auto& report_t : report->reports) {
			myfile << report << "\n";
		}
		myfile.close();
	}
}

