//
// Created by Mark Plagge on 3/19/20.
//

#ifndef NEMOTNG_HEARTBEATTESTCORE_H
#define NEMOTNG_HEARTBEATTESTCORE_H


#include "NemoNeuroCoreBase.h"
namespace nemo{
    namespace neuro_system {
        using namespace Eigen;
        class HeartbeatTestCore : INeuroCoreBase {
        public:
            HeartbeatTestCore(const std::shared_ptr<NemoCoreOutput> &spikeOutput, long lastActiveTime,
                              long currentNeuroTick, long previousNeuroTick, long lastLeakTime, long leakNeededCount,
                              bool heartbeatSent, int coreLocalId, tw_lp *myLp, tw_bf *myBf,
                              const std::shared_ptr<nemo_message> &curMessage, unsigned long curRngCount,
                              BF_Event_Status evtStat, int outputMode);

            HeartbeatTestCore(int coreLocalId, tw_lp *myLp, tw_bf *myBf, BF_Event_Status evtStat, int outputMode);

            HeartbeatTestCore();


            void core_init(tw_lp *lp)  override;

            void forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) override;

            void reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp) override;

            void core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp) override;

            void pre_run(tw_lp *lp) override;

            void core_finish(tw_lp *lp) override;

            ~HeartbeatTestCore() override;

            /**
             * nc:
             *
             */
            Matrix<int,Dynamic,Dynamic> neuron_connectivity;
            Matrix<int,Dynamic,Dynamic> neuron_weights;
            Vector<int,Dynamic> thresholds;
            Vector<int,Dynamic> current_membrane_pots;

        };
    }
}

#endif //NEMOTNG_HEARTBEATTESTCORE_H
