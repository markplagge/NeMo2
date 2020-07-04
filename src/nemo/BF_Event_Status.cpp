//
// Created by Mark Plagge on 7/2/20.
//

#include "BF_Event_Status.h"
namespace nemo{
	constexpr bool event_in(BF_Event_Status a, BF_Event_Status b){
		return  (static_cast<std::underlying_type<BF_Event_Status>::type>(a) & static_cast<std::underlying_type<BF_Event_Status>::type>(b));
	}
}
