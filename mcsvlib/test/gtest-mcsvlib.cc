/*
 * gtest-mcsvlib.cc
 *
 *  Created on: Dec 8, 2012
 *      Author: yanli
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

