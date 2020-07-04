//
// Created by Mark Plagge on 6/23/20.
//

#include "test_utils.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
std::vector<int> split(const std::string &s, char delimiter){
std::vector<int> tokens;
std::string token;
std::istringstream token_stream(s);
while (std::getline(token_stream, token, delimiter)) {
tokens.push_back(stoi(token));
}
return tokens;
}