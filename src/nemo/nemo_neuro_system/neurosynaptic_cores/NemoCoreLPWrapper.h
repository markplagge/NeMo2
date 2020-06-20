//
// Created by Mark Plagge on 5/2/20.
//

#ifndef NEMOTNG_NEMOCORELPWRAPPER_H
#define NEMOTNG_NEMOCORELPWRAPPER_H
#include "../../nemo_globals.h"
#include "NemoNeuroCoreBase.h"

namespace nemo {
	namespace config {
		class NemoConfig;

	}
	extern config::NemoConfig* global_config;

	namespace neuro_system {

		class NemoCoreLPWrapper {

		public:
			static void core_init(NemoCoreLPWrapper* s, tw_lp* lp)
			{
				//determine the type of core we want through mapping
				s->set_core_id(s, lp);
			}

			static void pre_run(NemoCoreLPWrapper* s, tw_lp* lp)
			{
				s->core->pre_run(lp);
			}

			static void forward_event(NemoCoreLPWrapper* s, tw_bf* bf, nemo_message* m, tw_lp* lp)
			{
				s->core->forward_event(bf, m, lp);
			}

			static void reverse_event(NemoCoreLPWrapper* s, tw_bf* bf, nemo_message* m, tw_lp* lp)
			{
				s->core->reverse_event(bf, m, lp);
			}

			static void core_commit(NemoCoreLPWrapper* s, tw_bf* bf, nemo_message* m, tw_lp* lp)
			{
				s->core->core_commit(bf, m, lp);
			}

			static void core_finish(NemoCoreLPWrapper* s, tw_lp* lp)
			{
				s->core->core_finish(lp);
			}

			void set_core_id(NemoCoreLPWrapper* s, tw_lp* lp);
			NemoNeuroCoreBase* core;
			ne_id_type core_id;
		};

	}// namespace neuro_system
}// namespace nemo

#endif//NEMOTNG_NEMOCORELPWRAPPER_H
