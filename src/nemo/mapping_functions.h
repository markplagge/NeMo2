//
// Created by Mark Plagge on 6/21/20.
//

#ifndef NEMOTNG_MAPPING_FUNCTIONS_H
#define NEMOTNG_MAPPING_FUNCTIONS_H
#include <ross.h>
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
}
/**@}**/
#endif//NEMOTNG_MAPPING_FUNCTIONS_H
