//
// Created by Mark Plagge on 12/6/19.
//

#include "nemo_globals.h"
#include <cstdarg>// For va_start, etc.
#include <cstdint>
#include <memory>// For std::unique_ptr
#include <string>
#include <type_traits>
#include <vector>
namespace nemo
{
unsigned long
get_neurosynaptic_tick (double now)
{
  return (unsigned long)now;
}

unsigned long
get_next_neurosynaptic_tick (double now)
{
  return (unsigned long)now + 1;
}

using namespace std;

template<typename... Args>
string
string_format (const ::std::string &format, Args... args)
{
  size_t size = snprintf (nullptr, 0, format.c_str (), args...) + 1;// Extra space for '\0'
  unique_ptr<char[]> buf (new char[size]);
  snprintf (buf.get (), size, format.c_str (), args...);
  return string (buf.get (), buf.get () + size - 1);// We don't want the '\0' inside
}

::std::string
Nemo_Message::to_string ()
{
  ::std::string message_type_desc = "HEARTBEAT";
  if (message_type == NEURON_SPIKE)
  {
	message_type_desc = "NEURON SPIKE";
  }
  ::std::string result = string_format ("type \tsource_core\tdest_axon\tintended_neuro_tick\tnemo_event_status\t"
										"random_call_count\t debug_time\n"
										"%s\t%i\t%i\t%i\t%i\t%i\t%f\n",
										message_type_desc.c_str (), source_core, dest_axon, intended_neuro_tick,
										nemo_event_status, random_call_count, debug_time);

  return result;
}
}// namespace nemo