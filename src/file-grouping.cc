/*
 * file-grouping.cc
 *
 *  Created on: Dec 9, 2012
 *      Author: yanli
 */

#include <vector>
#include <iostream>
#include "file-grouping.h"

using namespace std;

ostream& operator<<(ostream& os, vector<AccessGroup*> v_ags) {
  os << "{";
  for (size_t i = 0; i < v_ags.size(); ++i) {
      cout << *(v_ags[i]);
      if (i != v_ags.size() - 1)
        cout << ", ";
  }
  os << "}";

  return os;
}

std::ostream& operator<<(std::ostream& os, const AccessGroup& ag) {
  os << "{" << *(ag.brs_) << ", " << ag.times_ << "}";

  return os;
}

std::ostream& operator<<(std::ostream& os, const BlockRanges& brs) {
  os << "{";
  bool last_printed = false;
  for (BlockRanges::iterator i = brs.begin(); i != brs.end(); ++i) {
      if (last_printed)
        os << ", ";
      os << *i;
      last_printed = true;
  }
  os << "}";

  return os;
}

std::ostream& operator<<(std::ostream& os, const BlockRange& br) {
  return os << "(" << br.start_ << ", " << br.end_ << ")";
}

bool cmp_ag_ptr(AccessGroup* a, AccessGroup* b) {
  // reverse sort using ">"
  return a->times_ > b->times_;
}

Clusters::iterator find_br_in_clusters(Clusters* c, BlockRange a) {
  for (Clusters::iterator i = c->begin(); i != c->end(); ++i) {
      if ((*i)->find(a) != (*i)->end())
        return i;
  }
  return c->end();
}

void merge_clusters(Clusters* c, Clusters::iterator a, Clusters::iterator b) {
  // copy elements of b to a
  (*a)->insert((*b)->begin(), (*b)->end());
  // delete b from clusters c
  c->erase(b);
}

size_t block_range_size(BlockRange br) {
  return br.end_ - br.start_ + 1;
}

size_t block_ranges_size(BlockRanges* brs) {
  size_t result = 0;
  for (BlockRanges::iterator i = brs->begin(); i != brs->end(); ++i) {
      result += block_range_size(*i);
  }
  return result;
}

Clusters* agg_clustering(AccessGroups& ags, size_t cluster_max_size) {
  typedef vector<AccessGroup*> VAccessGroups;
  Clusters      *result = new Clusters();
  VAccessGroups  v_ags;
  AccessGroup   *ag;

  // Read all AGs and sort them
  while ((ag = ags.next()) != NULL) {
      v_ags.push_back(ag);
  }
  sort(v_ags.begin(), v_ags.end(), cmp_ag_ptr);

  // Process them one by one from the beginning
  for(VAccessGroups::iterator i = v_ags.begin(); i != v_ags.end(); ++i) {
      BlockRanges *pbrs = (*i)->brs_;
      BlockRanges::iterator pbrs_i = pbrs->begin();
      BlockRange a = *pbrs_i;
      BlockRange b = *(++pbrs_i);
      Clusters::iterator a_cluster = find_br_in_clusters(result, a);
      Clusters::iterator b_cluster = find_br_in_clusters(result, b);
      if (a_cluster != result->end() &&
          b_cluster != result->end() &&
          a_cluster != b_cluster) {
          if (block_ranges_size(*a_cluster) + block_ranges_size(*b_cluster)
              <= cluster_max_size) {
//                cout << "Merging a and b" << endl;
                merge_clusters(result, a_cluster, b_cluster);
          } else {
//            cout << "Can't merge " << **a_cluster << " and " << **b_cluster
//              << " due to size limit" << endl;
          }
      } else if (a_cluster != result->end() &&
          b_cluster == result->end() ) {
          if (block_ranges_size(*a_cluster) + block_range_size(b) <= cluster_max_size) {
//              cout << "Adding " << b << " to " << **a_cluster << endl;
              (*a_cluster)->insert(b);
          } else {
//           cout << "Can't add " << b << " to " << **a_cluster << " due to size limit."
//               << endl;
          }
      } else if (a_cluster == result->end() &&
          b_cluster != result->end()) {
          if (block_ranges_size(*b_cluster) + block_range_size(a) <= cluster_max_size) {
//              cout << "Adding " << a << " to " << **b_cluster << endl;
              (*b_cluster)->insert(a);
          } else {
//              cout << "Can't add " << a << " to " << **b_cluster << " due to size limit."
//                  << endl;
          }

      } else {
          // a_cluster == NULL && b_cluster == NULL
          // check each BRs to see if any still has space
          bool found_a_space = false;
          for (Clusters::iterator k = result->begin();
              k != result->end();
              ++k) {
              if (block_ranges_size(*k) + block_range_size(a)
                  + block_range_size(b) <= cluster_max_size) {
                (*k)->insert(a);
                (*k)->insert(b);
                found_a_space = true;
                break;
              }
          }
          if (!found_a_space) {
            // Create a new BRs that includes a and b
            // TODO: size limit check here
            BlockRanges *brs = new BlockRanges((*i)->brs_->begin(),
                (*i)->brs_->end());
  //          cout << "Creating a new cluster " << *brs << endl;
            result->push_back(brs);
          }
      }
  }

  return result;
}
