/*
 * mcsvlib.cc
 *
 *  Created on: Dec 9, 2012
 *      Author: yanli
 */

#include "mcsvlib.h"

std::istream& operator>>(std::istream& str,CSVRow& data)
{
    data.readNextRow(str);
    return str;
}
