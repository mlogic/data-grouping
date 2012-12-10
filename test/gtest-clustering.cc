/*
 * Process I/O traces and find correlated blocks, test cases
 *
 * Copyright (c) 2012, University of California, Santa Cruz, CA, USA.
 * Developers:
 *  Yan Li <yanli@cs.ucsc.edu>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Storage Systems Research Center, the
 *       University of California, nor the names of its contributors
 *       may be used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <fstream>
#include "gtest/gtest.h"
#include "mcsvlib.h"
#include "file-grouping.h"

using namespace std;

TEST(FileGroupingTest, TestBlockRange) {
  BlockRange a(1, 3);
  BlockRange b(1, 3);
  BlockRange c(1, 4);
  BlockRange d(2, 1);

  ASSERT_EQ(a, b);
  ASSERT_NE(b, c);
  ASSERT_LE(a, c);
  ASSERT_LE(a, d);
  ASSERT_TRUE(a <= d);
  ASSERT_TRUE(d >= a);
  ASSERT_FALSE(a < b);
  ASSERT_TRUE(a <= b);
  ASSERT_TRUE(a >= b);
  ASSERT_FALSE(a > b);
}

TEST(FileGroupingTest, TestBlockRanges) {
  BlockRanges a, b;
  a.insert(BlockRange(1,2));
  a.insert(BlockRange(7,8));
  b.insert(BlockRange(7,8));
  b.insert(BlockRange(1,2));

  ASSERT_EQ(a,b);
}

TEST(FileGroupingTest, TestAccessGroups) {
  ifstream file("test-access-groups.txt");
  ASSERT_TRUE(file.good());
  AccessGroups ags(file);
  const int input_items = 2;
  // expected BlockRanges
  int64_t expected_brs[][4] = {
      {6160416,6160423,6299160,6299167},
      {6160424,6160431,6160408,6160415}
  };
  int expected_ag_times[] = { 1 , 39 };
  AccessGroup* expected_ag[2];
  int i;
  for(i = 0; i < input_items; ++i) {
      BlockRanges *brs = new BlockRanges();
      brs->insert(BlockRange(expected_brs[i][0], expected_brs[i][1]));
      brs->insert(BlockRange(expected_brs[i][2], expected_brs[i][3]));
      expected_ag[i] = new AccessGroup(brs, expected_ag_times[i]);
  }

  AccessGroup* ag;
  i = 0;
  while ((ag = ags.next()) != NULL) {
      ASSERT_EQ((*expected_ag[i]), (*ag));
      delete ag;
      ++i;
  }
  ASSERT_EQ(2, i);

  for(i = 0; i < input_items; ++i) {
      delete expected_ag[i];
  }
}

TEST(FileGroupingTest, ClusteringStopWhenReachingClusterSizeLimit) {
  ifstream file("test-clustering-input.txt");
  ASSERT_TRUE(file.good());
  AccessGroups ag(file);
  Clusters *cluster;
  cluster = agg_clustering(ag, 6);
  ASSERT_TRUE(cluster);
  vector<BlockRanges*> v_cluster(cluster->begin(), cluster->end());
  sort(v_cluster.begin(), v_cluster.end());

  BlockRanges expected[2];
  expected[0].insert(BlockRange(1,2));
  expected[0].insert(BlockRange(3,4));
  expected[0].insert(BlockRange(7,8));
  expected[1].insert(BlockRange(5,6));
  expected[1].insert(BlockRange(10,11));

  ASSERT_EQ((size_t)2, v_cluster.size());
  ASSERT_EQ(expected[0], *v_cluster[0]);
  ASSERT_EQ(expected[1], *v_cluster[1]);

  for_each(cluster->begin(), cluster->end(), free);
  free(cluster);
}

TEST(FileGroupingTest, ClusteringWithoutClusterSizeLimit) {
  ifstream file("test-clustering-input.txt");
  ASSERT_TRUE(file.good());
  AccessGroups ag(file);
  Clusters *cluster;
  cluster = agg_clustering(ag, 100);
  ASSERT_TRUE(cluster);

  BlockRanges expected;
  expected.insert(BlockRange(1,2));
  expected.insert(BlockRange(3,4));
  expected.insert(BlockRange(7,8));
  expected.insert(BlockRange(5,6));
  expected.insert(BlockRange(10,11));

  ASSERT_EQ((size_t)1, cluster->size());
  ASSERT_EQ(expected, **(cluster->begin()));

  for_each(cluster->begin(), cluster->end(), free);
  free(cluster);
}
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
