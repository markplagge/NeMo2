//
// Created by Mark Plagge on 11/1/19.
//

#ifndef NEMOTNG_NEMOCOREOUTPUT_H
#define NEMOTNG_NEMOCOREOUTPUT_H

#include <fstream>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <visit_struct/visit_struct.hpp>
#include <configuru.hpp>
namespace nemo {
	/** @defgroup nemoio Nemo IO
 * Nemo IO System -  Defines data structurs and functions for saving and loading files
 * @{
 */

	template<typename T>
	struct DataFunctions {

		std::string
		to_csv() {
			T& underlying = static_cast<T&>(*this);
			return underlying.csv_data();
		}
		std::string
		get_header() {
			T& underlying = static_cast<T&>(*this);
			return underlying.csv_header();
		}
	};
	template<typename IMPL>
	struct JsonDataFunctions{
		std::string to_json(){
			IMPL& underlying = static_cast<IMPL&>(*this);
			return underlying.get_json();
		}
	};
	struct NemoSpikeData : DataFunctions<NemoSpikeData> {
		long source_neuron;
		long source_core;
		long dest_core;
		long dest_axon;
		double sched_spike_time;
		double cur_time;
		bool interchip;
		std::string
		csv_header() {
			return std::string("source_neuron,source_core,dest_core,dest_axon,sched_spike_time,cur_time\n");
		}
		std::string
		csv_data() {

			std::stringstream data_srm;
			data_srm << source_neuron << "," << source_core << "," << dest_core << "," << dest_axon << "," << sched_spike_time << "," << cur_time << "\n";
			std::string datarep = data_srm.str();
			return datarep;
		}
	};
	struct NemoMembraneData : DataFunctions<NemoMembraneData> {
		long source_core;
		long source_neuron;
		double membrane_pot;
		double cur_time;
		std::string
		csv_header() {
			return std::string("source_core,source_neuron,membrane_pot,cur_time");
		}
		std::string csv_data() {
			std::stringstream data_srm;
			data_srm << source_core << "," << source_neuron << "," << membrane_pot << "," << cur_time << "\n";
			return data_srm.str();
		}
	};

	struct NemoTNNeuronStats {
		long neuron_id;
		unsigned long spike_sent_count;
		unsigned long spike_recv_count;
		unsigned int core_dest;
		unsigned int neuron_dest;
		std::vector<double> active_times;
		std::vector<int> active_time_msg_rcv;
		NemoTNNeuronStats(long neuron_id, unsigned int core_dest, unsigned int neuron_dest) : neuron_id(neuron_id), core_dest(core_dest), neuron_dest(neuron_dest) {}
		NemoTNNeuronStats(long neuron_id, unsigned long spike_sent_count, unsigned long spike_recv_count, unsigned int core_dest, unsigned int neuron_dest, const std::vector<double>& active_times);


	};
	struct NemoDebugRecord {
		long core_id;
		std::vector<NemoTNNeuronStats> neurons;
		void set_neurons(const std::vector<NemoTNNeuronStats>& neurons);
		explicit NemoDebugRecord(long core_id);
		NemoDebugRecord(NemoDebugRecord *old) {
			this->neurons = old->neurons;
			this->core_id = old->core_id;
		}
	};
	struct NemoDebugJSONHandler{
		std::string filename;
		std::vector<std::shared_ptr<NemoDebugRecord>> core_records;
		NemoDebugJSONHandler();
		explicit NemoDebugJSONHandler(const std::string& filename);
		NemoDebugJSONHandler(const std::string& filename, unsigned int num_cores) : filename(filename) {
			core_records.reserve(num_cores);
		}
		void write_data();
		std::vector<NemoDebugRecord> get_core_records();
	};



	class NemoOutputHandler {
	protected:
		std::vector<NemoMembraneData> mem_data;
		std::vector<NemoSpikeData> spike_data;

	public:
		virtual void open_comms() = 0;
		virtual void close_comms() = 0;
		virtual void write() = 0;

		void add_data(NemoSpikeData m) {
			spike_data.push_back(m);
		}
		void add_data(NemoMembraneData m) {
			mem_data.push_back(m);
		}
	};




	class NemoPosixOut : public NemoOutputHandler {

		int sp_out_hdr = 0;
		int mp_out_hdr = 0;
		int rank{};
		std::string header;
		int prewrite_buffer = 1024;
		std::ofstream sp_f_out;
		std::ofstream mp_f_out;
	protected:
		std::string filename;
		std::string spike_out;
		std::string mpot_out;

	public:
		NemoPosixOut(std::string filename, int rank);
		NemoPosixOut(std::string spike_fn, std::string mpot_fn, int rank);

		NemoPosixOut();
		//NeMoPOSIXOut(const std::string &filename, int rank, int prewriteBuffer);

		void open_comms() override;
		void close_comms() override;
		void write() override;
	};



	class NemoCoreOutput {
	public:
		long core_id;
		NemoOutputHandler* output_handler;

		NemoCoreOutput(long core_id, NemoOutputHandler* output_handler);

		void save_spike(long source_neuron, long dest_core, long dest_axon, double sched_spike_time, double cur_time) const;
		void save_spike(long source_neuron, long dest_core, long dest_axon, double sched_spike_time, double cur_time, bool is_interchip) const;
		friend std::ostream& operator<<(std::ostream& os, const NemoCoreOutput& output);

		void 	save_membrane_pot(long source_neuron, double membrane_pot, double cur_time) const;
		void	save_spike_data(NemoSpikeData d);
		void	save_membrane_data(NemoMembraneData d);
	};
	/** @} */
}// namespace nemo
VISITABLE_STRUCT(nemo::NemoTNNeuronStats, neuron_id, spike_sent_count,spike_recv_count,core_dest,neuron_dest,active_times,active_time_msg_rcv);
VISITABLE_STRUCT(nemo::NemoDebugRecord, core_id, neurons );
#endif//NEMOTNG_NEMOCOREOUTPUT_H
