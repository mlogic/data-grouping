/*
 * Yan Li's CSV Lib, test cases
 *
 * Copyright (c) 2012, Yan Li <elliot.li@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Yan Li, nor the names of the project's
 *       contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL YAN LI
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include "gtest/gtest.h"
#include "mcsvlib.h"
#include <cstdlib>

using namespace std;

TEST(MCSVLibTest, TestReading) {
  ifstream       file("test-input-csv.txt");
  ASSERT_EQ(file.good(), true) << "Can't open test file." << endl;
  int expected_data[][5] = {
      {3510034,3510041,3504962,3504969,1},
      {542696,542711,19288484,19288491,2}
  };
  int i = 0;
  for(CSVIterator loop(file); loop != CSVIterator(); ++loop, ++i) {
      for(size_t j = 0; j < sizeof(expected_data[0])/sizeof(int); ++j) {
          ASSERT_EQ(
              atoi((*loop)[j].c_str()),
              expected_data[i][j]
          );
      }
  }
  ASSERT_EQ(i, 2);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

