//
// Created by Mark Plagge on 6/21/20.
//

#ifndef NEMOTNG_MAPPING_FUNCTIONS_H
#define NEMOTNG_MAPPING_FUNCTIONS_H
#include "BF_Event_Status.h"
#include <cstdint>
#include <neuro_os.h>
#include <ross.h>
#include <string>
#include <vector>
namespace nemo {
	/**
 * @defgroup mapping  Mapping Functions
 * helper functions for neuron mapping
 * @{
 */
	//Todo: implement these functions based on CODES mapping
	unsigned int get_core_id_from_gid(tw_lpid id);
	unsigned int get_local_id_from_gid(tw_lpid id);
	tw_peid nemo_map(tw_lpid gid);
	tw_lpid lp_typemapper(tw_lpid gid);
	inline uint64_t get_gid_from_core_local(unsigned long dest_core,unsigned long dest_axon) {
		return dest_core + 1;
//		if (g_tw_mynode == 0)
//			return (uint64_t)dest_core + 1;
//		else
//			return dest_core;
//		//return dest_core / g_tw_nlp;
	}
}
/**@}**/
#endif//NEMOTNG_MAPPING_FUNCTIONS_H
