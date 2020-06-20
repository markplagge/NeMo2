//
// Created by Mark Plagge on 6/20/20.
//

#include "NemoNeuronTrueNorth.h"

/** TODO: Eventually replace this with generic macro and non-branching ABS code. */
#define IABS(a) (((a) < 0) ? (-a) : (a))//!< Typeless integer absolute value function
/** TODO: See if there is a non-branching version of the signum function, maybe in MAth libs and use that. */
#define SGN(x) ((x > 0) - (x < 0))    //!< Signum function
#define DT(x) !(x)                    //!<Kronecker Delta function.
#define BINCOMP(s, p) IABS((s)) >= (p)//!< binary comparison for conditional stochastic evaluation

//NemoNeuronGeneric::integrate(source_id);
/** \defgroup TN_Functions TN Functions
 * Leak, integrate fire functions
 * @{
 */

/**
 * Main integration function. Calls TN specific functions
 * @param source_id axon ID source for weight computations
 */
void nemo::neuro_system::NemoNeuronTrueNorth::integrate(unsigned int source_id) {
	auto st = this->ns;
	auto wt = st->synaptic_weight[st->axon_types[source_id]];
	if (wt) {
		if (st->weight_selection[st->axon_types[source_id]]) {// zero if this is
			// normal, else
			stochastic_integrate(wt);
		}
		else {
			st->membrane_potential += wt;
		}
	}
}
/**
 * Leak_n - leaks *n* number of times (backwards compatibility for non-loop based cores
 * @param n_leaks
 */
void nemo::neuro_system::NemoNeuronTrueNorth::leak_n(int n_leaks) {
	for (int i = 0; i < n_leaks; i++) {
		leak();
	}
}
/**
 * Primary leak
 */
void nemo::neuro_system::NemoNeuronTrueNorth::leak() {
	ns->drawn_random_number = tw_rand_integer(get_cur_lp()->rng, 0, ns->largest_random_value);
	ns->rng_count += 1;
	auto prev_volt = ns->membrane_potential;
	numeric_leak_calc(tw_now(get_cur_lp()));
	ringing(prev_volt);
	//leak logic
	//ring check
}

/**
 * Main fire function. Calls TN specific functions - override from base class
 * code adapted from TNFire in origin nemo
 * This code is called after leak - when a heartbeat message is received.
 * @return
 */
bool nemo::neuro_system::NemoNeuronTrueNorth::fire() {
	tw_lp* lp = this->get_cur_lp();
	/*fire status set not used since neuro core manages neurons */
	auto nj = tw_rand_integer(lp->rng, 0, 255);
	this->ns->rng_count += 1;


	auto will_fire = fire_floor_ceiling_reset();
	will_fire = (will_fire && fire_timing_check(tw_now(lp)));
	if (will_fire) {
		if (!ns->is_output_neuron){
			tn_fire();
		}else{
			// used to set the bf here --- should do that
		}
	}

	return will_fire;

}

//return NemoNeuronGeneric::fire();

void nemo::neuro_system::NemoNeuronTrueNorth::tn_fire(){
		// Managed by core - now just a function stub
		// Managed by core - now just a function stub

};
/**
 *  @brief  Checks to see if a neuron should fire.
 *  @todo check to see if this is needed, since it looks like just a simple if
 * statement is in order.
 *
 *  @param st neuron state
 *
 *  @return true if the neuron is ready to fire.
 */
bool nemo::neuro_system::NemoNeuronTrueNorth::should_fire() {
	auto threshold = ns->pos_threshold;
	return (ns->membrane_potential >= threshold && fire_timing_check(tw_now(this->get_cur_lp())));
}
bool nemo::neuro_system::NemoNeuronTrueNorth::over_underflow_check() {
	auto n = ns;
	int ceiling = 393216;
	int floor = -393216;
	bool spike = false;
	if (n->membrane_potential > ceiling) {
		spike = true;
		n->membrane_potential = ceiling;
	}
	else if (n->membrane_potential < floor) {
		n->membrane_potential = floor;
	}
	return spike;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCInconsistentNamingInspection"
/**
* @brief New firing system using underflow/overflow and reset.
* @return true if neuron is ready to fire. Membrane potential is set
* regardless.
*/
bool nemo::neuro_system::NemoNeuronTrueNorth::fire_floor_ceiling_reset() {
	auto lp = this->get_cur_lp();
	bool should_fire = false;
	ns->drawn_random_number = 0;
	auto Vrst = ns->reset_voltage;
	auto alpha = ns->pos_threshold;
	auto beta = ns->neg_threshold;
	short gamma = ns->reset_mode;
	if (ns->c)
		ns->drawn_random_number =
				(tw_rand_integer(lp->rng, 0, ns->largest_random_value));
	// check if neuron is overflowing/underflowing:
	if (over_underflow_check()) {
		return true;
	}
	else {
		auto Vj = ns->membrane_potential;
		if (Vj >= ns->pos_threshold + ns->drawn_random_number) {
			// reset:
			Vj = ((DT(gamma)) * Vrst) + ((DT(gamma - 1)) * (Vj - (alpha + ns->drawn_random_number))) + ((DT(gamma - 2)) * Vj);
			// volt_type mp = ns->membranePotential;
			should_fire = true;
		}
		else {
			auto rng_weight = beta + ns->drawn_random_number;
			if (Vj < (-1 * (beta * ns->kappa + rng_weight * (1 - ns->kappa)))) {
				// volt_type x = ns->membranePotential;
				// x = ((-1 * beta) * ns->kappa);
				//        volt_type s1,s2,s3,s4;
				//        s1 = (-1*beta) * ns->kappa;
				//        s2 = (-1*(DT(gamma))) * Vrst;
				//        s3 = (DT((gamma - 1)) * (ns->membranePotential + (beta +
				//        ns->drawnRandomNumber)));
				//        s4 = (DT((gamma - 2)) * ns->membranePotential) * (1 - ns->kappa);
				//		//x = s1 + (s2 + s3 + s4);

				auto gamma_wt = (-1 * (DT(gamma))) * Vrst;
				auto b = DT((gamma - 1));
				auto wt_x = (DT((gamma - 2))) * Vj;
				Vj = ((-1 * beta) * ns->kappa + (gamma_wt + (b * (Vj + rng_weight)) + wt_x) * (1 - ns->kappa));
			}
		}
		ns->membrane_potential = Vj;
	}
	return should_fire;
}
#pragma clang diagnostic pop
/**
 *  @brief  Neuron stochastic integration function - for use with stochastic
 * leaks and synapse messages.
 *
 *  @param weight weight of selected leak or synapse
 *  @param st     the neuron state
 */
void nemo::neuro_system::NemoNeuronTrueNorth::stochastic_integrate(int weight) {
	auto st = ns;
	if (BINCOMP(weight, st->drawn_random_number)) {
		st->membrane_potential += 1;
	}
}
/**
 *  @brief NumericLeakCalc - uses formula from the TrueNorth paper to calculate
 * leak.
 *  @details Will run $n$ times, where $n$ is the number of big-ticks that have
 * occured since
 *  the last integrate. Handles stochastic and regular integrations.
 *
 *  @TODO: self-firing neurons will not properly send messages currently - if
 * the leak is divergent, the flag needs to be set upon neuron init.
 *  @TODO: does not take into consideration thresholds. Positive thresholds
 * would fall under self-firing neurons, but negative thresholds should be
 * reset.
 *  @TODO: test leaking functions
 */

void nemo::neuro_system::NemoNeuronTrueNorth::numeric_leak_calc(tw_stime now) {
	if (ns->lambda==0)
		return;

	// calculate current time since last leak --- LEAK IS TERRIBLE FOR THIS:

	// then run the leak function until we've caught up:
	auto new_mp = ns->membrane_potential;
	auto lamb = ns->lambda;
	auto drawn_random = ns->drawn_random_number;
	auto c = ns->c;
	int64_t omega = ns->sigma_l*(1 - ns->epsilon) + SGN(ns->membrane_potential)*ns->sigma_l*ns->epsilon;

//    int64_t omega = ns->sigma_l * (1 - ns->epsilon) +
//        SGN(ns->membranePotential) * ns->sigma_l * ns->epsilon;

//    ns->membranePotential =
//        ns->membranePotential + (omega * ((1 - ns->c) * ns->lambda)) +
//            (ns->c & (BINCOMP(ns->lambda, ns->drawnRandomNumber)));
		//ns->membranePotential =
	new_mp += (omega*((1 - ns->c)*lamb)) + (c & (BINCOMP(lamb, drawn_random)));

	ns->membrane_potential = new_mp;

}


/** From Neuron Behavior Reference - checks to make sure that there is no
 "ringing".
 The specs state that the leak stores the voltage in a temporary variable. Here,
 we store the leak voltage in the membrane potential, and override it with a new
 value. */
void nemo::neuro_system::NemoNeuronTrueNorth::ringing(unsigned int old_voltage) {
	if (ns->epsilon && (SGN(ns->membrane_potential)!=SGN(old_voltage))) {
		ns->membrane_potential = 0;
	}
}

/** @} */



bool nemo::neuro_system::NemoNeuronTrueNorth::fire_timing_check(tw_stime now) {

	unsigned long current_big_tick = get_neurosynaptic_tick(now);
	unsigned long prevBigTick = get_neurosynaptic_tick(ns->last_fire);

	if (current_big_tick == 0) {//big tick is zero so we are at first
		ns->fire_count++;
		return true;
	}
	//check to see if we are on a new tick:
	if (current_big_tick > prevBigTick) {
		// we are in a new big tick. This is okay.
		ns->fire_count = 1;
		ns->last_fire = now;
		return true;
	}
	else if (current_big_tick == prevBigTick) {//fire request on the same tick - error condition 1
		ns->fire_count++;

		//#ifdef DEBUG
		//    tw_error(TW_LOC, "Neuron fire rate error. Current big tick: %lu \t FireCount: %i. Neuron Core: %i Local: %i ",
		//             currentBigTick, ns->firecount, ns->myCoreID, ns->myLocalID);
		//#endif
		return false;
	}
	else {
		//Unknown error state - BigTick < prevBigTick.
		//tw_error(TW_LOC, "Out of order big ticks! PrevActive: %lu, CurrentTick: %lu", prevBigTick, currentBigTick);
		//Reverse computation dude.
		/** @todo: Check this from original C code might be a bug? */
		return true;
	}
}

/**
 * \ingroup TN_RESET True North Reset
 * True North Leak, Integrate and Fire Functions
 * Reset function defs. Neuron reset functions will
 * change the neuron state without saving the previous state. All voltage state
 saving
 * must be handled in the neuron event function neuronReceiveMessage().
 * These functions operate based on the table presented in \cite Cass13_1000 .
 * Currently, there are three functions, one for linear reset (resetLinear()),
 * "normal" reset (resetNormal()), and non-reset (resetNone()).

 From the paper:
 | \f$𝛾_j\f$ | \f$𝜘_j\f$| Reset Mode               |     Positive Reset     |
 Negative Reset    |
 |----|----|--------------------------|:----------------------:|:---------------------:|
 | 0  | 0  | normal                   |          \f$R_j\f$         |
 \f$-R_j\f$        |
 | 0  | 1  | normal - neg saturation  |          \f$R_j\f$         |
 \f$-𝛽_j\f$        |
 | 1  | 0  | Linear                   | \f$V_j - (𝛼_j  + 𝜂_j)\f$ | \f$V_j + (𝛽_j
 + 𝜂_j)\f$ |
 | 1  | 1  | linear -neg saturation   |  \f$V_j - (𝛼_j,+ 𝜂_j)\f$ |
 \f$-𝛽_j\f$        |
 | 2  | 0  | non-reset                |          \f$V_j\f$         |
 \f$V_j\f$         |
 | 2  | 1  | non-reset net saturation |          \f$V_j\f$         |
 \f$-𝛽_j\f$        |

 * @todo: Check that reverse reset functions are needed, since previous voltage
 is stored in the neuron.
 * @{ */
/**
 * negative saturation reset function (common to all reset modes, called if
 * 𝛾 is true. Simply sets the value of the membrane potential to $-𝛽_j$.
**/
void nemo::neuro_system::NemoNeuronTrueNorth::neg_threshold_reset() {
	ns->membrane_potential = -ns->neg_threshold;
}
/**
 * Normal reset function
 */
void nemo::neuro_system::NemoNeuronTrueNorth::reset_normal() {

	if (ns->membrane_potential < ns->neg_threshold) {
		if (ns->kappa)
			neg_threshold_reset();
		else
			ns->membrane_potential = -(ns->reset_voltage);
	} else {
		ns->membrane_potential = ns->reset_voltage;  // set current voltage to \f$R\f$.
	}
}
/**
 *   Linear reset mode - ignores \f$R\f$, and sets the membrane potential
 *  to the difference between the threshold and the potential. *
 */
void nemo::neuro_system::NemoNeuronTrueNorth::reset_linear() {
	if (ns->membrane_potential < ns->neg_threshold) {
		if (ns->kappa)
			neg_threshold_reset();
		else {
			ns->membrane_potential =
					ns->membrane_potential - (ns->neg_threshold + ns->drawn_random_number);
		}
	} else {
		ns->membrane_potential =
				ns->membrane_potential - (ns->pos_threshold + ns->drawn_random_number);
	}
}

/**
 *   non-reset handler function - does non-reset style reset. Interestingly,
 *  even non-reset functions follow the negative saturation parameter from the
 * paper.
 */
void nemo::neuro_system::NemoNeuronTrueNorth::reset_none() {
	if (ns->kappa && ns->membrane_potential < ns->neg_threshold) {
		neg_threshold_reset();
	}
}
/** @}  */


void nemo::neuro_system::NemoNeuronTrueNorth::reset() {
	//no reset directly called
}

