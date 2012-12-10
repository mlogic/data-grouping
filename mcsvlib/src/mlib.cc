/*
 * mlib.cc
 *
 *  Created on: Dec 9, 2012
 *      Author: yanli
 */

#include <sstream>
#include "mlib.h"

int64_t str_to_int64(const std::string& str) {
  int64_t result;
  std::stringstream ss(str);
  ss >> result;
  return result;
}


//TODO: << of a vector
//TODO: << of a set
