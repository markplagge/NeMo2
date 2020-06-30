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
			//char vx[] = {"*********************************************************"};
			std::string vx(80, '*');
			constexpr int c1 = 2;
			constexpr int c2 = 40;
			constexpr int c3 = 39;
			constexpr int c4 = 5;

			constexpr char tbstop[] = "\t";
			extern int VERBOSE;
			const std::string line(40, '-');

			std::ostream& bold_on(std::ostream& os) {
				return os << "\e[1m";
			}

			std::ostream& bold_off(std::ostream& os) {
				return os << "\e[0m";
			}

			enum class ColorCode : int {
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

				x_blu_27 = 27,
				x_br_mag_89 = 89,
				x_red_197 = 197,
				x_red_196 = 196,
				x_nice_wt = 230,
				cust_c1 = 208,
				cust_c2 = 209,
				cust_c3 = 39,
				cust_c4 = 75

			};
			using namespace std;
			std::stringstream ss;

			template<ColorCode color, typename printable>
			std::string print_as_color(printable const& value) {
				std::stringstream sstr;
				sstr << "\033[1;" << static_cast<int>(color) << "m" << value << "\033[0m";
				return sstr.str();
			}
			template<ColorCode color>
			std::string print_as_color(std::stringstream const& value) {
				std::stringstream sstr;
				sstr << "\033[1;" << static_cast<int>(color) << "m" << value.str() << "\033[0m";
				return sstr.str();
			}

			template<typename printable>
			std::string pr_extd(printable const& value, ColorCode cc) {
				std::stringstream sstr;
				char pinkish[] = {0x1b, '[', '3', '8', ';', '5', ';', '2', '1', '2', 'm', 0};
				char init[] = {0x1b, '[', '3', '8', ';', '5', ';'};
				char end[] = {'m', 0};
				char normal[] = {0x1b, '[', '0', ';', '3', '9', 'm', 0};
				sstr << init << static_cast<int>(cc) << end << value << normal;
				return sstr.str();
			}
			template<typename printable>
			std::string pr_extd(printable const& value, int cc) {
				std::stringstream sstr;
				char pinkish[] = {0x1b, '[', '3', '8', ';', '5', ';', '2', '1', '2', 'm', 0};
				char init[] = {0x1b, '[', '3', '8', ';', '5', ';'};
				char end[] = {'m', 0};
				char normal[] = {0x1b, '[', '0', ';', '3', '9', 'm', 0};
				sstr << init << static_cast<int>(cc) << end << value << normal;
				return sstr.str();
			}

			template<int color, typename printable>
			std::string print_as_color(printable value) {
				return pr_extd(value, color);
			}
			template<typename printable>
			std::string pr_emph(printable const& value) {
				//return print_as_color<ColorCode::magenta>(value);
				//return pr_extd(value, ColorCode::x_nice_wt);
				return pr_extd(value, 206);
			}

			//auto rowstar = print_as_color<ColorCode::bright_blue>("*");
			auto rowstar = print_as_color<ColorCode::cust_c3>("*");
			auto star_line = print_as_color<ColorCode::cust_c4>(vx);

			template<typename T>
			void print_vec_nlines(int total, T vecname) {
				ss << rowstar << " " << print_as_color<ColorCode::magenta>(vecname);
				ss << print_as_color<ColorCode::red>(total) << endl;
			}

			template<typename T>
			void print_config_element(T const& elm) {
				ss << setfill(' ') << setw(c1) << rowstar << setw(c2) << " " << setw(c3) << elm << left << setw(c4) << rowstar << endl;
			}
			template<typename T>
			void print_vector_limit(std::vector<T> elms) {
				int chk = 0;
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
				ss << setfill(' ') << setw(c1) << rowstar << setw(c2) << desc << setw(c3) << elm << left << setw(c4) << rowstar << endl;
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
				if (VERBOSE <= 0) {
					VERBOSE = 0;
				}

				//ss << print_as_color<ansi_color_code::bright_blue>(vx) << endl;

				//ss << setfill('*') << setw(22) << rowstar << print_as_color<ColorCode::green>(" NeMo 2 - The Next Generation ") << setw(17) << right << "" << endl;
				//ss << setfill('*') << setw(25) << "" << print_as_color<134>(" NeMo 2 - The Next Generation ") << setw(25) << right << "" << endl;
				auto scolor = print_as_color<ColorCode::cust_c3>(std::string(25, '*'));
				std::stringstream nemo_name;
				//134 too?
				nemo_name << bold_on << print_as_color<ColorCode::x_red_196>(" NeMo ") << bold_on << print_as_color<ColorCode::cust_c1>(" 2") << bold_off << print_as_color<254>(" -") << print_as_color<ColorCode::cust_c2>(" The Next Generation ");
				ss << scolor << nemo_name.str() << scolor << endl;
				print_config_element<char>(' ');
				if (VERBOSE) {
					pr_e("List print limited to ", VERBOSE);
				}
			}
			void start_sim_ftr() {
				ss << print_as_color<ColorCode::bright_blue>(vx) << endl;
				std::cout << ss.str();
				ss.clear();
			}
			void end_sim_hdr() {
				ss << print_as_color<ColorCode::bright_blue>(vx) << endl;
				print_config_element<char>(' ');
			}
			void end_sim_ftr() {
				ss << print_as_color<ColorCode::bright_blue>(vx) << endl;
				std::cout << ss.str();
				ss.clear();
			}

			template<>
			void pr_e(std::string desc, config::ScheduledTask elm) {
				std::stringstream elx;
				elx << elm;

				pr_e(desc, elx.str());
			}
			template<>
			void print_config_element<config::NemoModel>(config::NemoModel const& elm) {
				ss << rowstar << elm << rowstar << endl;
			}
			template<typename PRINTABLE>
			void snprint_hdr_col(char line_buffer[200], PRINTABLE m) {
				stringstream hdr;
				hdr << m;

				snprintf(line_buffer, 199, "%s%s%20s%s%38s\n", line_buffer, rowstar.c_str(), " ",
						 hdr.str().c_str(), rowstar.c_str());
			}

			template<typename HDR_T>
			std::string pr_loc_h(HDR_T hdr_msg, int n) {
				char str_buffer[200] = {'\0'};
				std::stringstream sb;

				if (n >= 0) {
					std::stringstream hdr_addon;
					hdr_addon << n << " " << hdr_msg;
					snprint_hdr_col(str_buffer, print_as_color<123>(hdr_addon.str()));
				}
				else {
					snprint_hdr_col(str_buffer, hdr_msg);
				}

				sb << print_as_color<ColorCode::cyan>(vx) << endl
				   << str_buffer;
				return sb.str();
			}
			template<typename HDR_T>
			std::string pr_loc_h(HDR_T hdr_msg) {
				return pr_loc_h(hdr_msg, -1, "");
			}
		}// namespace p
	}
#endif//NEMOTNG_NEMO_FORMATTED_PRINT_H
