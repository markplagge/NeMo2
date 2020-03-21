//
// Created by Mark Plagge on 2019-03-13.
//

#ifndef NEMO2_CORELP_H
#define NEMO2_CORELP_H
#include <string>
#include <iostream>
#include <vector>
#include "../nemo_neuro_system/neuron_models/NemoNeuronGeneric.h"
#include "../nemo_neuro_system/neurosynaptic_cores/NemoNeuroCoreBase.h"
#include "../nemo_globals.h"
//#include "../nemo_neuro_system/neurosynaptic_cores/NemoNeuroCoreBase.h"
namespace nemo {


//////////////////////
/**
 * CoreLP - This class is an adapter for ROSS to the C++ object system in NeMo2.
 * Each LP in ROSS is of type CoreLP - and each CoreLP contains one reference to a
 * neuromorphic core class.
 *
 * Hopefully this abstraction will not hurt performance.
 * This class breaks some OO rules, as it will need to talk to the mapping and configuation
 * parsing systems directly - can't pass it anything but what is expected from ROSS.
 *
 * Updated - Since we are getting elements passed to us through ROSS C-Style (pointers),
 * We will use function templates to manage this?
 *
 * A CoreLP contains a reference to the INeuroCoreBase class (*core), which is managed
 * here. Basic core logic is handled by this class.
 *
 * Basically, the CoreLP class is a holder for the functions that ROSS expects and the neuron object.
 * Logic that is not related to neurosynaptic core functionality is handled here, then control is handed off
 * to the inner INEuroCoreBase class.
 *
 * Rather than have ROSS pass around INeuroCoreBase objects directly, it will pass around
 */
    class CoreLP {
    public:


        static void event_trace(nemo_message *m, tw_lp *lp, char *buffer, int *collect_flag);


        /**
     * ROSS function for initializing this core LP. Creates a new instance of a neurosynaptic core.
     * Calls the instantiated
     * @param s
     * @param lp
     */
        static void core_init(CoreLP *s, tw_lp *lp) {
            //determine the type of core we want through mapping
            s->create_core(lp);
        }

        int core_creation_mode = 0;

        static void pre_run(CoreLP *s, tw_lp *lp) {
            s->core->pre_run(lp);
            // testing heartbeat code - remove once tested or enclose in a #
            tw_lp *test_core_lp = lp; //tw_getlocal_lp(0); // zero is the test core for our tests.
            //we want to set the weights of the core's neurons to 0 so that we can ignore spikes and generate heartbeats only
            auto heartbeat_schedule = s->heartbeat_schedule;
            ::std::sort(heartbeat_schedule.begin(), heartbeat_schedule.end());
            ::std::reverse(heartbeat_schedule.begin(), heartbeat_schedule.end());
            for (auto sched_time : heartbeat_schedule) {
                if (sched_time != 1) {
                    if (sched_time < 0) {
                        sched_time = 1;
                    }

                    double send_time = sched_time - 1;
                    send_time += JITTER(test_core_lp->rng);
                    tw_event *e = tw_event_new(0, send_time, test_core_lp);
                    auto *message = (nemo_message *) tw_event_data(e);
                    message->intended_neuro_tick = (int) sched_time;
                    message->source_core = -1;
                    message->dest_axon = 1;
                    message->debug_time = send_time;
                    ::std::cout << "Msg send time:  " << send_time << "\n";

                    tw_event_send(e);
                }
            }
        }

        static void forward_event(CoreLP *s, tw_bf *bf, nemo_message *m, tw_lp *lp) {
            s->core->forward_event(bf, m, lp);
        }

        static void reverse_event(CoreLP *s, tw_bf *bf, nemo_message *m, tw_lp *lp) {
            s->core->reverse_event(bf, m, lp);
        }

        static void core_commit(CoreLP *s, tw_bf *bf, nemo_message *m, tw_lp *lp) { s->core->core_commit(bf, m, lp); }

        static void core_finish(CoreLP *s, tw_lp *lp) {
            s->core->core_finish(lp);
            s->core->cleanup_output();
        };


        void create_core(tw_lp *lp);

        neuro_system::INeuroCoreBase *get_core() {
            return core;
        }

        void setCore(neuro_system::INeuroCoreBase *core);

        ::std::vector<int> heartbeat_schedule;
    private:
        neuro_system::INeuroCoreBase *core;
        int active;
        core_types core_type;
    };
}
#endif //NEMO2_CORELP_H
