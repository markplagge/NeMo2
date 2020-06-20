//
// Created by Mark Plagge on 6/19/20.
//

#ifndef NEMOTNG_NEMO_FORMATTED_PRINT_H
#define NEMOTNG_NEMO_FORMATTED_PRINT_H

#include <vector>
#include <iostream>
//#include "nemo_formatted_print.h"

using std::boolalpha;
using std::cout;
using std::endl;

#include <iomanip>
#include <sstream>
#include <string>
	namespace nemo {
		namespace p {
			char vx[] = {"*********************************************************"};
			constexpr char tbstop[] = "\t";
			extern int VERBOSE;
			const std::string line(40,'-');
			enum class ansi_color_code : int {
				black = 30,
				red = 31,
				green = 32,
				yellow = 33,
				blue = 34,
				magenta = 35,
				cyan = 36,
				white = 37,
				bright_black = 90,
				bright_red = 91,
				bright_green = 92,
				bright_yellow = 93,
				bright_blue = 94,
				bright_magenta = 95,
				bright_cyan = 96,
				bright_white = 97,
			};
			using namespace std;
			std::stringstream  ss;


			template<typename printable>
			std::string print_as_color(printable const& value, ansi_color_code color) {
				std::stringstream sstr;
				sstr << "\033[1;" << static_cast<int>(color) << "m" << value << "\033[0m";
				return sstr.str();
			}
			template<ansi_color_code color, typename printable>
			std::string print_as_color(printable const& value) {
				std::stringstream sstr;
				sstr << "\033[1;" << static_cast<int>(color) << "m" << value << "\033[0m";
				return sstr.str();
			}

			template<typename printable>
			std::string print_as_color(printable const& value, ansi_color_code color, std::stringstream ss) {
				ss << "\033[1;" << static_cast<int>(color) << "m" << value << "\033[0m";
			};

			template<typename printable>
			std::string pr_emph(printable const& value){
				return print_as_color(value,ansi_color_code::magenta);
			}


			auto rowstar = print_as_color<ansi_color_code::bright_blue>("*");

			template<typename T>
			void print_config_element(T const& elm) {
				ss << setfill(' ') << setw(2) << rowstar << setw(30) << " " << setw(30) << elm << left << setw(2) << rowstar << endl;
			}
			template<typename T>
			void print_vector_limit(std::vector<T> elms){
				int chk =  0;
				for (auto& x : elms) {
					print_config_element(x);
					chk += 1;
					if  ((VERBOSE != 0) && (chk > VERBOSE)) {
						break;
					}
				}
			}
			template<typename T>
			void print_config_vector(std::vector<T> elms) {
				print_vector_limit(elms);
			}



			template<typename T>
			void pr_e(std::string desc, T elm) {
				ss << setfill(' ') << setw(2) << rowstar << setw(30) << desc << setw(30) << elm << left << setw(2) << rowstar << endl;
			}

//			template<typename T>
//			void pr_v(std::string desc, T elms) {
//
//				pr_e(desc, " ");
//				print_config_vector(elms);
//			}

			template<typename T, typename V>
			void pr_v(V desc, std::vector<T> elms){
				pr_e(desc, " ");
				print_config_vector(elms);
			}

			void start_sim_hdr() {
				if (VERBOSE <= 0){
					VERBOSE = 0;
				}

				//ss << print_as_color<ansi_color_code::bright_blue>(vx) << endl;

				ss << setfill('*') << setw(22) << rowstar << print_as_color<ansi_color_code::green>(" NeMo 2 - The Next Generation ") << setw(17) << right << "" << endl;
				print_config_element<char>(' ');
				if (VERBOSE) {
					pr_e("List print limited to ", VERBOSE);
				}

			}
			void start_sim_ftr() {
				ss << print_as_color<ansi_color_code::bright_blue>(vx) << endl;
				std::cout << ss.str();
				ss.clear();
			}
			void end_sim_hdr() {
				ss << print_as_color<ansi_color_code::bright_blue>(vx) << endl;
				print_config_element<char>(' ');
			}
			void end_sim_ftr() {
				ss << print_as_color<ansi_color_code::bright_blue>(vx) << endl;
				std::cout << ss.str();
				ss.clear();
			}


		}
}
#endif//NEMOTNG_NEMO_FORMATTED_PRINT_H
