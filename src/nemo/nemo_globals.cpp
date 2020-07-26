//
// Created by Mark Plagge on 12/6/19.
//

#include "nemo_globals.h"
#include <cstdarg>// For va_start, etc.
#include <cstdint>
#include <fstream>
#include <memory>// For std::unique_ptr
#include <string>
#include <type_traits>
#include <vector>
namespace nemo
{
	core_types get_core_enum_from_json(std::string core_type){
		if (core_type.find("TN") != std::string::npos){
			return TN;
		}else if (core_type.find("LIF") != std::string::npos){
			return LIF;
		}else{
			return NO_CORE_TYPE;
		}
	}

	unsigned long
get_neurosynaptic_tick (double now)
{
  return (unsigned long)now;
}
/** Use this XARG wherever you need string representation of the message type */
nemo_message_type x;
#define X(a) #a,
	static const char *nemo_message_type_strings[] = { NEMO_MESSAGE_TYPES };
#undef X


unsigned long
get_next_neurosynaptic_tick (double now)
{
  return (unsigned long)now + 1;
}

using namespace std;

template<typename... Args>
string string_format (const ::std::string &format, Args... args)
{
  size_t size = snprintf (nullptr, 0, format.c_str (), args...) + 1;// Extra space for '\0'
  unique_ptr<char[]> buf (new char[size]);
  snprintf (buf.get (), size, format.c_str (), args...);
  return string (buf.get (), buf.get () + size - 1);// We don't want the '\0' inside
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCInconsistentNamingInspection"
#define X(a, b) b,
	static const char *ERR_FILETYPE[] = { ERR_FILETYPES };
#undef X
#define X(a, b) b,
	static const char *ERR_FILE_KIND[] = {ERR_FILE_KINDS };
#undef X






	template <typename STRING_LIKE>
	char * check_file_exist(STRING_LIKE path_to_file, int type, bool do_err,  char const *file, int line){
		std::string path_adapt(path_to_file);
		std::ifstream f(path_adapt.c_str());
		if (f.good()){
			const char * msg ={"\0"};
			return const_cast<char*>(msg);
		}else {
			const char * msg = ERR_FILE_KIND[0];
			if(do_err){
				tw_error(file,line,"%s %s", ERR_FILETYPE[type], ERR_FILE_KIND[0]);
			}
			return const_cast<char*>(msg);
		}
	}
	template <>
	char * check_file_exist<std::string>(std::string path_to_file, int type, bool do_err,  char const *file, int line) {
		return check_file_exist(path_to_file.c_str(), type,do_err,file,line);
	}
	template <>
	char * check_file_exist<char *>(char *path_to_file, int type, bool do_err,  char const *file, int line) {
		return check_file_exist(std::string(path_to_file), type,do_err,file,line);
	}
#pragma clang diagnostic pop

//::std::string
//NemoMessage::to_string ()
//{
//  ::std::string message_type_desc = "HEARTBEAT";
//  if (message_type == NEURON_SPIKE)
//  {
//	message_type_desc = "NEURON SPIKE";
//  }
//  ::std::string result = string_format ("type \tsource_core\tdest_axon\tintended_neuro_tick\tnemo_event_status\t"
//										"random_call_count\t debug_time\n"
//										"%s\t%i\t%i\t%i\t%i\t%i\t%f\n",
//										message_type_desc.c_str (), source_core, dest_axon, intended_neuro_tick,
//										nemo_event_status, random_call_count, debug_time);
//
//  return result;
//}
}// namespace nemo