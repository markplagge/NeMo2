//
// Created by Mark Plagge on 3/19/20.
//

#include <tiffio.h>
#include "HeartbeatTestCore.h"
#include <Eigen/src/Core/Array.h>
#include <Eigen/src/Core/EigenBase.h>

void nemo::neuro_system::HeartbeatTestCore::forward_event(tw_bf *bf, nemo::nemo_message *m, tw_lp *lp) {
    evt_stat = BF_Event_Status::None;
    cur_message = ::std::make_shared<nemo::nemo_message>(*m);
    my_lp = lp;
    my_bf = bf;
    if (m->message_type == HEARTBEAT){
        forward_heartbeat_handler();
    }
    if(m->message_type == NEURON_SPIKE){
        //current_membrane_pots(m->dest_axon) = (current_membrane_pots(m->dest_axon) * (neuron_connectivity(m->dest_axon)) * neuron_weights)
        auto mc = neuron_connectivity.row(m->dest_axon) * neuron_weights(m->dest_axon);
        current_membrane_pots.row(m->dest_axon) += mc;
    }
    if (evt_stat == BF_Event_Status::Heartbeat_Rec){
        //heartbeat_recvd
        for(int i = 0; i < neurons_per_core; i ++){
            if(current_membrane_pots(i) > thresholds(i)){
                current_membrane_pots(i) = 0;
                auto evt = tw_event_new(output(i),0.0001,lp);
                nemo_message *m = (nemo_message *) tw_event_data(evt);
                m->message_type = NEURON_SPIKE;
                m->source_core = core_id;
                m->intended_neuro_tick = tw_now(lp) + 0.0001;
                tw_event_send(evt);
            }
        }
    }

}

void nemo::neuro_system::HeartbeatTestCore::reverse_event(tw_bf *bf, nemo::nemo_message *m, tw_lp *lp) {
//freaking reverse not implemented
}

void nemo::neuro_system::HeartbeatTestCore::core_commit(tw_bf *bf, nemo::nemo_message *m, tw_lp *lp) {

}

void nemo::neuro_system::HeartbeatTestCore::pre_run(tw_lp *lp) {

}

void nemo::neuro_system::HeartbeatTestCore::core_finish(tw_lp *lp) {

}

void nemo::neuro_system::HeartbeatTestCore::core_init(tw_lp *lp) {
    for(int i = 0; i < LIF_NEURONS_PER_CORE; i ++){
        neuron_connectivity(i) = 1;
        thresholds(i) = 1;
        current_membrane_pots(i) = 0;
        for(int j = 0; j < LIF_NEURONS_PER_CORE; j ++){
            if(i == j)
                neuron_weights(i,j) = 1;
            else
                neuron_weights(i,j) = 0;
        }
    }
    tw_event * e = tw_event_new(lp->gid, 1,lp);
    auto *m = (nemo_message *)tw_event_data(e);
    m->message_type = NEURON_SPIKE;
    m->debug_time = 0;
    m->dest_axon = 0;
    m->intended_neuro_tick = 1;
    m->source_core = get_core_id_from_gid(lp->gid);
    m->nemo_event_status = 0;
    tw_event_send(e);

}

nemo::neuro_system::HeartbeatTestCore::HeartbeatTestCore(const std::shared_ptr<NemoCoreOutput> &spikeOutput,
                                                         long lastActiveTime, long currentNeuroTick,
                                                         long previousNeuroTick, long lastLeakTime,
                                                         long leakNeededCount, bool heartbeatSent, int coreLocalId,
                                                         tw_lp *myLp, tw_bf *myBf,
                                                         const std::shared_ptr<nemo_message> &curMessage,
                                                         unsigned long curRngCount, nemo::BF_Event_Status evtStat,
                                                         int outputMode) : INeuroCoreBase(spikeOutput, lastActiveTime,
                                                                                          currentNeuroTick,
                                                                                          previousNeuroTick,
                                                                                          lastLeakTime, leakNeededCount,
                                                                                          heartbeatSent, coreLocalId,
                                                                                          myLp, myBf, curMessage,
                                                                                          curRngCount, evtStat,
                                                                                          outputMode) {}

nemo::neuro_system::HeartbeatTestCore::HeartbeatTestCore(int coreLocalId, tw_lp *myLp, tw_bf *myBf,
                                                         nemo::BF_Event_Status evtStat, int outputMode)
        : INeuroCoreBase(coreLocalId, myLp, myBf, evtStat, outputMode) {}



nemo::neuro_system::HeartbeatTestCore::~HeartbeatTestCore() = default;

nemo::neuro_system::HeartbeatTestCore::HeartbeatTestCore() {
    using namespace Eigen;
     MatrixXi nc = Eigen::MatrixXi::Constant(neurons_per_core,neurons_per_core,1);
     MatrixXi nv =Eigen::MatrixXi::Constant(neurons_per_core,neurons_per_core,1);
     Array<int,Dynamic,1> th(neurons_per_core);
     VectorXi tb = VectorXi::Constant(neurons_per_core,0);
     neuron_connectivity = nc;
     neuron_weights = nv;
     thresholds = th;
     current_membrane_pots = tb;


}
