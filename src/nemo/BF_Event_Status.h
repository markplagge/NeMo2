//
// Created by Mark Plagge on 7/2/20.
//

#ifndef NEMOTNG_BF_EVENT_STATUS_H
#define NEMOTNG_BF_EVENT_STATUS_H

#include <cstdint>
#include <type_traits>
/** @defgroup bf_evt BF_EventStatus
 * Event Status enum / bitfield group. This group contains elements that manage the
 * enum / flags for event status created in NeMo.
 * @{
 */

/**
 * BF_Event_Status - A descriptive specialized replacement for tw_bf.
 * Flags the program flow when handling events.
 */
namespace nemo {
	enum class BF_Event_Status : uint32_t {
		None = 0x00,                  //! No state changes happened
		Heartbeat_Sent = (1u << 1),   //! a heartbeat message was sent
		Spike_Sent = (1u << 2),       //a spike message was sent
		Output_Spike_Sent = (1u << 3),//! a spike message to an output layer was sent
		Heartbeat_Rec = (1u << 4),    //! A heartbeat was received
		Spike_Rec = (1u << 5),        //! A spike message was received
		NS_Tick_Update = (1u << 6),   //! We updated the neurosynaptic tick value
		Leak_Update = (1u << 7),      //! We updated the membrane potentials through a leak
		FR_Update = (1u << 8),        //! We updated the membrane potentials through fire/reset computations
		Spike_Sent_Interchip = (1u << 9), //! Interchip spike sent
		Process_Stopped = (1u << 10),    //! We have stopped the currently running process
		New_Process_Started = (1u << 11)//! We have started a new process
	};


	template<typename Enumeration>
	auto as_integer (Enumeration const value) -> typename std::underlying_type<Enumeration>::type
	{
		return static_cast<typename ::std::underlying_type<Enumeration>::type> (value);
	}

	inline BF_Event_Status
	operator|(BF_Event_Status a, BF_Event_Status b){
		return static_cast<BF_Event_Status> (as_integer (a) | as_integer (b));
	}


	inline BF_Event_Status
	operator& (BF_Event_Status a, BF_Event_Status b)
	{
		return static_cast<BF_Event_Status> (as_integer (a) & as_integer (b));
	}

	template<typename E>
	inline bool	in_the (unsigned int a, E b)
	{
		return bool (a & as_integer (b));
	}

	inline bool	in_the (BF_Event_Status a, BF_Event_Status b)
	{
		return as_integer (a & b);
	}

	inline BF_Event_Status & operator|= (BF_Event_Status &a, BF_Event_Status b)
	{
		a = static_cast<BF_Event_Status> (
				as_integer (a) | as_integer (b));
		return a;
	}

	inline bool	is_hb_sent (BF_Event_Status event_status)
	{
		return as_integer (event_status & BF_Event_Status::Heartbeat_Sent);
	}

	inline bool	is_spike_sent (BF_Event_Status event_status)
	{
		return as_integer (event_status & BF_Event_Status::Spike_Sent);
	}

	inline bool	is_spike_recv (BF_Event_Status event_status)
	{
		return as_integer (event_status & BF_Event_Status::Spike_Rec);
	}

	inline bool	is_output_spike_sent (BF_Event_Status event_status)
	{
		return as_integer (event_status & BF_Event_Status::Output_Spike_Sent);
	}

	inline bool	is_heartbeat_rec (BF_Event_Status event_status)
	{
		return as_integer (event_status & BF_Event_Status::Heartbeat_Rec);
	}

	inline bool	is_nstick_updated (BF_Event_Status event_status)
	{
		return as_integer (event_status & BF_Event_Status::NS_Tick_Update);
	}


	template<typename EST>
	inline bool	in_event (BF_Event_Status status, EST event)
	{
		return as_integer (status & event);
	}

	inline BF_Event_Status	add_event_condition (BF_Event_Status event_status, BF_Event_Status new_event)
	{
		//no check for accuracy, could add it here.
		return event_status | new_event;
	}

	inline BF_Event_Status	add_event_condiditon (BF_Event_Status new_event)
	{
		return new_event;
	}


	template<typename NEW_EVT>	inline BF_Event_Status
	add_evt_status (BF_Event_Status event_status, NEW_EVT new_event)
	{
		return event_status | new_event;
	}

	template<typename... NEW_EVT>	inline BF_Event_Status
	add_evt_status (BF_Event_Status event_status, NEW_EVT... new_event)
	{
		return event_status | add_evt_status (new_event...);
	}
/** @} */



}
#endif//NEMOTNG_BF_EVENT_STATUS_H
