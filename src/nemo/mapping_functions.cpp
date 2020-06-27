//
// Created by Mark Plagge on 3/25/20.
//
#include "nemo_globals.h"
namespace nemo {
	/**
 * Cores are arranged as follows:
 * Core 0 - GID 0 - Non Simulation Core - Scheduling core that manages running models
 * Core 1 - GID 1 and so on are sim cores.
 * Nemo operates using linear mapping, so this is a basic linear map.
 * @param id the GID of the requesting core LP
 * @return
 */
	unsigned int get_core_id_from_gid(tw_lpid id) {
		return id;
	}
	/**
 * Local ID. DO NOT USE - cruft from nemo1
 * @param id
 * @return
 */
	unsigned int get_local_id_from_gid(tw_lpid id) {
		return 1;
	}

	/**
 * As explained in get_core_id_from_gid, gid 0 is a control core, and the
 * rest are simulated cores
 * @param gid
 * @return the LP type for the GID.
 */
	tw_lpid lp_typemapper(tw_lpid gid) {
		if (gid == 0) {
			return 0;
		}
		else {
			return 1;
		}
	}
	tw_peid nemo_map(tw_lpid gid) {
		return (tw_peid)gid / g_tw_nlp;
	}
}// namespace nemo
