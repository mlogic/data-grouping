/*
 * Process I/O traces and find correlated blocks
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

#include <iostream>
#include <fstream>
#include "file-grouping.h"
#include "mlib.h"
using namespace std;

int main(int argc, char *argv[])
{
  if (argc < 3) {
      cerr << "Usage: " << argv[0] << " input-csv-file max-limit" << endl;
      return 2;
  }

  fstream file(argv[1]);
  if (!file.good()) {
      cerr << "Can't open file " << argv[1] << endl;
      return 1;
  }

  size_t cluster_size_limit = str_to<size_t>(string(argv[2]));
  if (cluster_size_limit == 0) {
      cerr << "Invalid max-limit." << endl;
      return 1;
  }

  AccessGroups ag(file);
  Clusters *clusters;
  clusters = agg_clustering(ag, cluster_size_limit);

  if (NULL == clusters) {
      cerr << "Clustering failed." << endl;
      return 3;
  }

  // Output the result in CSV
  cout << "range_start, range_end, cluster_id" << endl;
  int cluster_count = 0;
  for (Clusters::iterator i = clusters->begin(); i!=clusters->end(); ++i) {
      for (BlockRanges::iterator j = (*i)->begin(); j!=(*i)->end(); ++j) {
          cout << j->start_ << "," << j->end_ << "," << cluster_count << endl;
      }
      ++cluster_count;
  }

  for_each(clusters->begin(), clusters->end(), free);
  free(clusters);
  return 0;
}
