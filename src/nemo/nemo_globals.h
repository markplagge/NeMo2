//
// Created by Mark Plagge on 11/1/19.
//

#ifndef NEMOTNG_NEMO_GLOBALS_H
#define NEMOTNG_NEMO_GLOBALS_H
//#include "../../external/eigen/Eigen/Dense"
#include <ross.h>
#include <cstdint>

#include <vector>
#include <string>
#include "BF_Event_Status.h"
namespace nemo
{

#define RNG_START(lp) auto rng_count = lp->rng->count
#define RNG_END(lp) msg->random_call_count = (lp->rng->count - rng_count)
/** @defgroup global_macros Global Macro helper functions  *{ */
/**
 * JITTER(rng) -> macro for adding a jitter value to sent messages.
 * If there are 4096 cores, then there will be a max message population of:
 * (4096 * 256) between ticks $Tc_1$, $Tc_2$. The jitter is there to ensure that
 * all of the messages for a particular tick do not collide.
 *
 *
 */
#define JITTER_SCALE 1000
#define JITTER(rng) tw_rand_unif (rng) / JITTER_SCALE
/** @} */

/** @defgroup time_helpers
 * Helper functions and macros for ROSS timing
 * @{
 */
#define LITTLE_TICK = (double)1 / 1000
#define BIG_TICK = 1

	/**
	 *  gets the current neruosynaptic tick. With double time values, is really just floor(now)
	 * @param now
	 * @return
	 */
unsigned long get_neurosynaptic_tick (double now);

/** @todo: use this macro rather than calling yet another function and write more macros for timing */
#define GET_NEUROSYNAPTIC_TICK(now) long (now)
/**
 * Get next neuro tick - Returns the next tick for scheduling
 * @param now
 * @return
 */
unsigned long get_next_neurosynaptic_tick (double now);

/**
 * lt_offset - This is the value of the next little tick. Use this when creating events in the
 * tw_offset.
 */
#define lt_offset(rng) = JITTER (rng) + LITTLE_TICK
/**
 * bt_offset - This is offset for the next big tick. Can use this when creating events in the tw_offset for
 * the next big tick event.
 */
#define bt_offset(rng) JITTER (rng) + BIG_TICK

/**@} */

/** @defgroup types Typedef Vars
 * Typedefs to ensure proper types for the neuron parameters/mapping calculations
 * @{  */

using ne_weight_t = uint32_t;
using ne_stats_t = int_least64_t;
using ne_mapping_t = uint32_t;
using ne_neuro_param_t = int32_t;
using ne_id_type = int_fast64_t;  //!< id type is used for local mapping functions - there should be $n$ of them depending on CORE_SIZE
using ne_size_type = int_fast32_t;//!< nesize_type holds sizes of the sim - core size, neurons per core, etc.

typedef int32_t nemo_volt_type;   //!< volt_type stores voltage values for membrane potential calculations
typedef int64_t nemo_weight_type; //!< seperate type for synaptic weights.
typedef int32_t nemo_thresh_type; //!< Type for weights internal to the neurons.
typedef uint16_t nemo_random_type;//!< Type for random values in neuron models.

typedef uint64_t stat_type;

/**@}*/
template<typename... Args>
::std::string
string_format (const ::std::string &format, Args... args);

typedef enum CoreTypes {
	NO_CORE_TYPE,
  TN,
  LIF
} core_types;

core_types get_core_enum_from_json(std::string core_type);


#define NEMO_MESSAGE_TYPES \
	X(NEURON_SPIKE) \
	X(HEARTBEAT) \
	X(NOS_LOAD_MODEL) \
	X(NOS_TICK) \
	X(NOS_START) \
	X(NOS_STOP) \
	X(NOS_STATUS)


#define X(a) a,
	enum nemo_message_type { NEMO_MESSAGE_TYPES } ;
#undef X


//enum nemo_message_type {
//	NEURON_SPIKE = 1,
//	HEARTBEAT = 2,
//	NOS_LOAD_MODEL = 4,
//	NOS_TICK = 8,
//	NOS_START = 16,
//	NOS_STOP = 32,
//	NOS_STATUS = 64
//};
/** @} */


//enum class BF_Event_Status : uint32_t; {
//  None = 0x00,                  //! No state changes happened
//  Heartbeat_Sent = (1u << 1),   //! a heartbeat message was sent
//  Spike_Sent = (1u << 2),       //a spike message was sent
//  Output_Spike_Sent = (1u << 3),//! a spike message to an output layer was sent
//  Heartbeat_Rec = (1u << 4),    //! A heartbeat was received
//  Spike_Rec = (1u << 5),        //! A spike message was received
//  NS_Tick_Update = (1u << 6),   //! We updated the neurosynaptic tick value
//  Leak_Update = (1u << 7),      //! We updated the membrane potentials through a leak
//  FR_Update = (1u << 8),        //! We updated the membrane potentials through fire/reset computations
//
//  Process_Stopped = (1u << 9),		//! We have stopped the currently running process
//  New_Process_Started = (1u << 10)  //! We have started a new process
//
//
//};

/** @defgroup global_help Global Helpers.
 * Global helper functions / classes which are used throughout NeMo @{ */

/**
 * crtp helper / basis class. Helps keep static polymorphism function boilerplate code managable.
 * @tparam T
 */
template<typename T>
struct crtp {
  T &underlying ()
  {
	return static_cast<T &> (*this);
  }

  T const &
  underlying () const
  {
	return static_cast<T const &> (*this);
  }
};

/** Gives us the BINCOMP (binary comparison) function used for stochastic weight modes.
 * Takes the absolute value of the first value, and compares it to the seocnd. */
template<typename T1, typename T2>
bool bincomp (T1 val1, T2 val2)
{
  return abs (val1) >= val2;
}

/** Kroniker Delta Function for TrueNorth */
template<typename T1>
constexpr auto
dt (T1 x)
{
  return !(x);
}

/** SGN Function */
template<typename T1>
constexpr auto
sgn (T1 x)
{
  return ((x > 0) - (x < 0));
}
#define NEMO_MAX_CHAR_DATA_SIZE 927408
/**
 * Main message data structure.
 *
 */
typedef struct NemoMessage {

	int message_type;
	uint32_t nemo_event_status;

		int source_core;
		int dest_axon;
		unsigned long intended_neuro_tick;

		unsigned int random_call_count;
		double debug_time;


		int model_id;
		//char update_message[NEMO_MAX_CHAR_DATA_SIZE];



} nemo_message;

// Helper functon for BF logic:

inline uint64_t
get_gid_from_core_local (ne_id_type dest_core, ne_id_type dest_axon)
{
  //currently, cores are GIDs since this is a strict linear map
  return (uint64_t)dest_core + 1;
}
///**
// * 2D Array helper template. Matrix is a 2D array using STD::Array
// * @tparam T
// * @tparam ROW
// * @tparam COL
// */
//    template<class T, size_t ROW, size_t COL>
//    using Matrix = std::array<std::array<T, COL>, ROW>;

/**
 * 2D Matrix helper template. Matrix as a vector. Vectors are as fast as a C heap array,
 * so no reason not to use them.
 * @tparam T type of the data in the matrix
 */
//template<class T, typename ROW, typename COL>
//using Matrix = std::vector<std::vector<T>>(ROW, std::vector<T>(COL));

//
//    template<class T>
//    class Matrix {
//        unsigned m_rowSize;
//        unsigned m_colSize;
//        ::std::vector<::std::vector<T>> matrix;
//    public:
//        Matrix(unsigned, unsigned, T);
//
//        Matrix(const char *);
//
//        Matrix(const Matrix &);
//
//        ~Matrix();
//
//    };

extern char *SPIKE_OUTPUT_FILENAME;
extern int SPIKE_OUTPUT_MODE;
extern int OUTPUT_MODE;

//@todo: Move this to a config file that will be set up by CMAKE - or make these run-time-options
#define THREADED_WRITER 1
extern ::std::vector<core_types> core_type_map;

/** @} */

/** @defgroup CTMS Compile-Time Model Settings:
 * Model settings - define the size of arrays, number of outputs per neuron, etc..
 * @{
 */
#define MDL_SET(X)
// LIF Core settings:
constexpr int LIF_NEURONS_PER_CORE = 256;
constexpr int LIF_NUM_OUTPUTS = 256;

/** @} */
/**
 * @defgroup tn_const TrueNorth Neuron Limitations
 * Contains TrueNorth network constants
 * @{
 */
constexpr int NEURONS_PER_TN_CORE = 256;
constexpr int WEIGHTS_PER_TN_NEURON = 4;
constexpr int MAX_OUTPUT_PER_TN_NEURON = 1;
/** @} */



}// namespace nemo

#endif//NEMOTNG_NEMO_GLOBALS_H
