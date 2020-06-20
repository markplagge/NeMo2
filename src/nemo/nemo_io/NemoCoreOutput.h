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

namespace nemo
{
/** @defgroup nemoio Nemo IO
 * Nemo IO System -  Defines data structurs and functions for saving and loading files
 * @{
 */

template<typename T>
struct DataFunctions {

  std::string
  to_csv ()
  {
	T &underlying = static_cast<T &> (*this);
	return underlying.csv_data ();
  }
  std::string
  get_header ()
  {
	T &underlying = static_cast<T &> (*this);
	return underlying.csv_header ();
  }
};
struct NemoSpikeData: DataFunctions<NemoSpikeData> {
  long source_neuron;
  long source_core;
  long dest_core;
  long dest_axon;
  double sched_spike_time;
  double cur_time;
  std::string
  csv_header ()
  {
	return std::string ("source_neuron,source_core,dest_core,dest_axon,sched_spike_time,cur_time\n");
  }
  std::string
  csv_data ()
  {

	std::stringstream data_srm;
	data_srm << source_neuron << "," << source_core << "," << dest_core << "," << dest_axon << "," << sched_spike_time << "," << cur_time << "\n";
	std::string datarep = data_srm.str ();
	return datarep;
  }
};
struct NemoMembraneData: DataFunctions<NemoSpikeData> {
  long source_core;
  long source_neuron;
  double membrane_pot;
  double cur_time;
  std::string
  csv_header ()
  {
	return std::string ("source_core,source_neuron,membrane_pot,cur_time");
  }
  std::string
  csv_data ()
  {
	std::stringstream data_srm;
	data_srm << source_core << "," << source_neuron << "," << membrane_pot << "," << cur_time << "\n";
	return data_srm.str ();
  }
};

class NemoOutputHandler {
 protected:
  std::vector<NemoMembraneData> mem_data;
  std::vector<NemoSpikeData> spike_data;

 public:
  virtual void
  open_comms () = 0;
  virtual void
  close_comms () = 0;
  virtual void
  write () = 0;

  void
  add_data (NemoSpikeData m)
  {
	spike_data.push_back (m);
  }
  void
  add_data (NemoMembraneData m)
  {
	mem_data.push_back (m);
  }
};

class NeMoPOSIXOut: public NemoOutputHandler {
  std::string filename;
  std::string spike_out;
  std::string mpot_out;
  int sp_out_hdr = 0;
  int mp_out_hdr = 0;
  int rank{};
  std::string header;
  int prewrite_buffer;
  std::ofstream sp_f_out;
  std::ofstream mp_f_out;

 public:
  NeMoPOSIXOut (std::string filename, int rank);

  NeMoPOSIXOut ();
  //NeMoPOSIXOut(const std::string &filename, int rank, int prewriteBuffer);

  void
  open_comms () override;
  void
  close_comms () override;
  void
  write () override;
};

class NemoCoreOutput {
 public:
  long core_id;
  NemoOutputHandler *output_handler;

  NemoCoreOutput (long coreId, NemoOutputHandler *outputHandler);

  void
  save_spike (long source_neuron, long dest_core, long dest_axon, double sched_spike_time, double cur_time) const;

  friend std::ostream &
  operator<< (std::ostream &os, const NemoCoreOutput &output);

  void
  save_membrane_pot (long source_neuron, double membrane_pot, double cur_time) const;
  void
  save_spike_data (NemoSpikeData d);
  void
  save_membrane_data (NemoMembraneData d);
};
/** @} */
}// namespace nemo

#endif//NEMOTNG_NEMOCOREOUTPUT_H
