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

#ifndef FILE_GROUPING_H_
#define FILE_GROUPING_H_

#include <list>
#include <set>
#include "mlib.h"
#include "mcsvlib.h"

/**
 * A Range of blocks like (1, 4), which means blocks 1, 2, 3, 4.
 */
class BlockRange {
public:
  BlockRange(int64_t start, int64_t end) :
    start_(start), end_(end) {}
  int64_t start_;
  int64_t end_;
  bool operator==(const BlockRange& b) const {
    return (start_ == b.start_) && (end_ == b.end_);
  }
  bool operator!=(const BlockRange& b) const {
    return !((*this) == b);
  }
  bool operator<(const BlockRange& b) const {
    if (start_ < b.start_)
      return true;
    if (start_ > b.start_)
      return false;
    // now a.start_ == b.start_
    if (end_ < b.end_)
      return true;
    else
      return false;
  }
  bool operator>(const BlockRange& b) const {
    if (start_ > b.start_)
      return true;
    if (start_ < b.start_)
      return false;
    // now a.start_ == b.start_
    if (end_ > b.end_)
      return true;
    else
      return false;
  }
  bool operator<=(const BlockRange& b) const {
    return (*this) < b || (*this) == b;
  }
  bool operator>=(const BlockRange& b) const {
    return (*this) > b || (*this) == b;
  }
};
std::ostream& operator<<(std::ostream& os, const BlockRange& br);


/**
 * BlockRanges is a set of BlockRanges
 */
typedef std::set<BlockRange> BlockRanges;
std::ostream& operator<<(std::ostream& os, const BlockRanges& brs);

/**
 * AccessGroup is BlockRanges plus AccessTimes.
 */
class AccessGroup {
public:
  AccessGroup(BlockRanges* brs, int times) : brs_(brs), times_(times) {};
  ~AccessGroup() { if (brs_) delete brs_; }
  bool operator==(const AccessGroup& b) const {
    return (*brs_) == *(b.brs_) && times_ == b.times_;
  }

  BlockRanges* brs_;
  int times_;
};

std::ostream& operator<<(std::ostream& os, const AccessGroup& ag);

class AccessGroups {
public:
  AccessGroups(std::istream& str) : csviter(str) {};
  // Returns the next AccessGroup from the istream, returns NULL if no
  // more available.
  AccessGroup* next() {
    if (csviter == CSVIterator())
      return NULL;

    int64_t a_start = str_to_int64((*csviter)[0]);
    int64_t a_end   = str_to_int64((*csviter)[1]);
    int64_t b_start = str_to_int64((*csviter)[2]);
    int64_t b_end   = str_to_int64((*csviter)[3]);
    int access_times = str_to_int64((*csviter)[4]);
    BlockRanges *brs = new BlockRanges();
    brs->insert(BlockRange(a_start, a_end));
    brs->insert(BlockRange(b_start, b_end));
    ++csviter;
    return new AccessGroup(brs, access_times);
  }
private:
  CSVIterator csviter;
};

typedef std::list<BlockRanges*> Clusters;
Clusters* agg_clustering(AccessGroups& ags, size_t cluster_max_size);

#endif /* FILE_GROUPING_H_ */
