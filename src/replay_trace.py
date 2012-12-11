__author__ = 'yanli'

# Args
# 1: cluster check window (in seconds)
# 2: cluster count
# 3: csv trace file name
# 4: (optional) cluster file name, without which the even divier will be used as the baseline

import sys
import filegrouping
from datetime import timedelta

replay_check_window = timedelta(seconds = int(sys.argv[1]))
cluster_count = int(sys.argv[2])

if len(sys.argv) < 5:
    print "using even divider as baseline"
    baseline_mode = True
else:
    baseline_mode = False

lba_low = 86048
lba_high = 28308287
baseline_cluster_size = (lba_high - lba_low + 1) / cluster_count

check_window_count = 0
total_clusters_accessed = 0
lba_cluster_accessed = []
for blockaccess in filegrouping.BlockAccesses(sys.argv[3]):
    if baseline_mode:
        if not 'window_start_time' in locals():
            window_start_time = blockaccess.time
        else:
            if blockaccess.time - window_start_time > replay_check_window:
                check_window_count += 1
                window_start_time = blockaccess.time
                lba_cluster_accessed = []
            lba_cluster = (blockaccess.lba - lba_low) / baseline_cluster_size
            lba_end_cluster = (blockaccess.lba + blockaccess.blkcnt - 1 - lba_low) / baseline_cluster_size
            if not lba_cluster in lba_cluster_accessed:
                total_clusters_accessed += 1
                lba_cluster_accessed.append(lba_cluster)
            if not lba_end_cluster in lba_cluster_accessed:
                total_clusters_accessed += 1
                lba_cluster_accessed.append(lba_end_cluster)


print "check window size:", sys.argv[1], "seconds, cluster count:", sys.argv[2], ", average clusters accessed in a check window:", 1.0 * total_clusters_accessed / check_window_count