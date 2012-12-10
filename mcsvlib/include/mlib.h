/*
 * mlib.h
 *
 *  Created on: Dec 9, 2012
 *      Author: yanli
 */

#ifndef MLIB_H_
#define MLIB_H_

#include <string>

int64_t str_to_int64(const std::string& str);

template <typename T>
T str_to(const std::string& str) {
  std::stringstream ss(str);
  T result;
  return ss >> result ? result : 0;
}

#endif /* MLIB_H_ */
