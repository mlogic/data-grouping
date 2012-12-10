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
