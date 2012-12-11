from __future__ import print_function
import sys
import csv
from datetime import datetime, timedelta
from collections import namedtuple, deque

__author__ = 'yanli'

# consts
DATETIME_FORMAT = "%Y-%m-%d %H:%M:%S"

BlockAccess = namedtuple('BlockAccess', ['time', 'server', 'uid', 'rw', 'lba', 'blkcnt', 'duration'])
BlockRange = namedtuple('BlockRange', ['lba', 'blkcnt'])

class BlockAccesses:
    def __init__(self, csvfilename):
        csvfile = open(csvfilename, 'rb')
        dialect = csv.Sniffer().sniff(csvfile.read(1024))
        csvfile.seek(0)
        self.csvreader = csv.reader(csvfile)

    def __iter__(self):
        return self

    def next(self):
        row = self.csvreader.next()
        return BlockAccess(datetime.strptime(row[0], DATETIME_FORMAT),
                           row[1], int(row[2]), row[3], long(row[4]), long(row[5]), int(row[6]))

def breakDownAG(old_brs, new_brs):
    """Check and break down brs when overlapped"""
    overlap_found = False
    new_brs_to_check = deque(new_brs)
    result_new_brs = []
    result_old_brs = []
    while new_brs_to_check:
        # are we on second iteration?
        if len(result_old_brs) != 0:
            # If so, we should use the accessgroup from previous iteration
            old_brs = list(result_old_brs)
            result_old_brs = []
        # block range to check
        brtc = new_brs_to_check.pop()
        for old_br in old_brs:
            # ibrc can be empty if it's already covered by all other BRs in the AG
            if brtc == None:
                result_old_brs.append(old_br)
                continue
            # check if old_br and new_block_range_to_check have overlap
            old_br_start = old_br[0]
            old_br_end = old_br[1]
            brtc_start = brtc[0]
            brtc_end = brtc[1]
            #       oooo
            #  iiii
            if brtc_end < old_br_start:
                # no overlap, just keep the old_br
                result_old_brs.append(old_br)
            #       oooo
            #     iiii
            elif brtc_end >= old_br_start and brtc_start < old_br_start and brtc_end < old_br_end:
                overlap_found = True
                result_old_brs.append((old_br_start, brtc_end))
                result_old_brs.append((brtc_end+1, old_br_end))
                result_new_brs.append((old_br_start, brtc_end))
                brtc = (brtc_start, old_br_start - 1)
            #       oooo
            #     iiiiii
            elif brtc_end >= old_br_start and brtc_start < old_br_start and brtc_end == old_br_end:
                overlap_found = True
                result_old_brs.append(old_br)
                result_new_brs.append(old_br)
                brtc = (brtc_start, old_br_start - 1)
            #       oooo
            #     iiiiiiii
            elif brtc_end >= old_br_start and brtc_start < old_br_start and brtc_end > old_br_end:
                overlap_found = True
                result_old_brs.append(old_br)
                result_new_brs.append(old_br)
                brtc = (brtc_start, old_br_start - 1)
                new_brs_to_check.append((old_br_end + 1,brtc_end))
            #       oooo
            #       ii
            elif brtc_start == old_br_start and brtc_end < old_br_end:
                overlap_found = True
                result_old_brs.append(brtc)
                result_old_brs.append((brtc_end+1,old_br_end))
                result_new_brs.append(brtc)
                brtc = None
            #       oooo
            #       iiii
            elif brtc_start == old_br_start and brtc_end == old_br_end:
                result_old_brs.append(brtc)
                result_new_brs.append(brtc)
                brtc = None
            #       oooo
            #       iiiiii
            elif brtc_start == old_br_start and brtc_end > old_br_end:
                overlap_found = True
                result_old_brs.append(old_br)
                result_new_brs.append(old_br)
                brtc = (old_br_end + 1, brtc_end)
            #       oooo
            #        ii
            elif brtc_start > old_br_start and brtc_end < old_br_end:
                overlap_found = True
                result_old_brs.append((old_br_start, brtc_start - 1))
                result_old_brs.append(brtc)
                result_old_brs.append((brtc_end + 1, old_br_end))
                result_new_brs.append(brtc)
                brtc = None
            #       oooo
            #        iii
            elif brtc_start > old_br_start and brtc_end == old_br_end:
                overlap_found = True
                result_old_brs.append((old_br_start, brtc_start - 1))
                result_old_brs.append(brtc)
                result_new_brs.append(brtc)
                brtc = None
            #       oooo
            #        iiiii
            elif brtc_start > old_br_start and brtc_start < old_br_end and brtc_end > old_br_end:
                overlap_found = True
                result_old_brs.append((old_br_start, brtc_start - 1))
                result_old_brs.append((brtc_start, old_br_end))
                result_new_brs.append((brtc_start, old_br_end))
                brtc = (old_br_end + 1, brtc_end)
            #       oooo
            #           iiiii
            elif brtc_start > old_br_end:
                result_old_brs.append(old_br)

        if brtc:
            result_new_brs.append(brtc)

    return overlap_found, result_old_brs, result_new_brs


def addAG(accessgroups, incoming_ag):
    if incoming_ag in accessgroups:
        accessgroups[incoming_ag] += 1
        return
    elif not accessgroups:
        # ignore br of len 1
        if len(incoming_ag) > 1:
            accessgroups[incoming_ag] = 1
        return

    # no direct match of incoming_ag in accessgroups, have to check each elements
    # in accessgroups for overlapping
    for ag, times in accessgroups.items():
        overlapfound, brokendown_ag, brokendown_incoming_ag = breakDownAG(ag, incoming_ag)

        if overlapfound:
            del accessgroups[ag]
            for i in range(len(brokendown_ag)):
                for j in range(i + 1, len(brokendown_ag)):
                    n = frozenset([brokendown_ag[i], brokendown_ag[j]])
                    if n in accessgroups:
                        accessgroups[n] += times
                    else:
                        accessgroups[n] = times
            incoming_ag = brokendown_incoming_ag

    for i in range(len(brokendown_incoming_ag)):
#        n = brokendown_incoming_ag[i]
#        if n in accessgroups:
#            accessgroups[n] += 1
#        else:
#            accessgroups[n] = 1
        for j in range(i + 1, len(brokendown_incoming_ag)):
            n = frozenset([brokendown_incoming_ag[i], brokendown_incoming_ag[j]])
            if n in accessgroups:
                accessgroups[n] += 1
            else:
                accessgroups[n] = 1


def findAccessGroup(blockaccesses, windowsize):
    result = dict()
    buffer = deque()
    for new_access in blockaccesses:
        # Use a list instead of set because some tuples may appear more than once here
        new_br = (new_access.lba, new_access.lba + new_access.blkcnt - 1)
        # remove the block accesses that are too old
        while buffer and new_access.time - windowsize > buffer[0].time:
            buffer.popleft()

        addAG(result, frozenset([new_br]))
        for old_access in buffer:
            old_br = (old_access.lba, old_access.lba + old_access.blkcnt - 1)
            overlapfound, old_ags, new_ags = breakDownAG([old_br], [new_br])

            if not overlapfound:
                addAG(result, frozenset([old_br, new_br]))
            else:
                for new_ag in new_ags:
                    for old_ag in old_ags:
                        if new_ag != old_ag:
                            addAG(result, frozenset([new_ag, old_ag]))

        buffer.append(new_access)
    return result


if __name__=='__main__':
    result_ags = findAccessGroup(BlockAccesses(sys.argv[1]), timedelta(seconds = 2))
    for _ag in result_ags:
        ag = list(_ag)
        print(ag[0][0], ',', ag[0][1], ',', ag[1][0], ',', ag[1][1], ',', result_ags[_ag], sep='')