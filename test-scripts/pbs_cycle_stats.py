# config: utf-8

# Copyright (C) 2003-2020 Altair Engineering, Inc. All rights reserved.
# Copyright notice does not imply publication.
#
# ALTAIR ENGINEERING INC. Proprietary and Confidential. Contains Trade Secret
# Information. Not for use or disclosure outside of Licensee's organization.
# The software and information contained herein may only be used internally and
# is provided on a non-exclusive, non-transferable basis. License may not
# sublicense, sell, lend, assign, rent, distribute, publicly display or
# publicly perform the software or other information provided herein,
# nor is Licensee permitted to decompile, reverse engineer, or
# disassemble the software. Usage of the software and other information
# provided by Altair(or its resellers) is only as explicitly stated in the
# applicable end user license agreement between Altair and Licensee.
# In the absence of such agreement, the Altair standard end user
# license agreement terms shall govern.

import argparse
import os
import time
from collections import OrderedDict

import numpy as np


def cvt_sec_to_str(sec):
    """
    Convert seconds[.microseconds] into a form like 20h 30m 50s
    """
    if isinstance(sec, str):
        sec = float(sec)

    if sec == 0:
        return '0s'

    rest = int(sec)
    microseconds = sec - rest
    days = rest / 86400
    rest %= 86400
    hours = rest / 3600
    rest %= 3600
    mins = rest / 60
    rest %= 60

    str_time = ''
    if days:
        str_time += '%dd ' % (days)
    if hours:
        str_time += '%dh ' % (hours)
    if mins:
        str_time += '%dm ' % (mins)
    if rest:
        str_time += '%ds ' % (rest)
    if microseconds:
        ms_str = '\u00B5' + 's'
        str_time += '%d %s' % (microseconds * 1000000, ms_str)

    if str_time[-1] == ' ':
        str_time = str_time[:-1]
    return str_time


def output_stats(input_arr, perc_list, type_str, is_time=False):
    """
    Print stats based on a numpy array
    Print the mean, median, min, and max, and requested percentiles
    """
    stats = OrderedDict()
    stats['Max'] = np.percentile(input_arr, 100)
    for p in perc_list:
        stats[str(p) + 'p'] = np.percentile(input_arr, p,
                                            interpolation='nearest')

    stats['Mean'] = np.mean(input_arr)
    stats['Median'] = np.percentile(input_arr, 50, interpolation='nearest')
    stats['Min'] = np.percentile(input_arr, 0)

    for k in stats:
        if is_time is True:
            strp = cvt_sec_to_str(stats[k])

        else:
            strp = str(int(stats[k]))
        # cvt_sec_to_str() can return a unicode character
        print(k + ' ' + type_str + ': ' + str(strp))
    print('')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('--schedlogs', '-s', metavar='DIR',
                        default='/var/spool/pbs/sched_logs',
                        help="Path to sched logs")
    parser.add_argument('--perc', '-p', nargs='+', default=[90, 75, 25],
                        help='percentials to print')
    parser.add_argument('--stats', action='store_true',
                        help='print extra cycle stats')
    parser.add_argument('--long', '-l', metavar='DURATION',
                        default='1200', type=int,
                        help='print start time of cycle '
                        'if it is over this duration')
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='print verbose cycle stats.  Used with --stats')
    parser.add_argument('--start', metavar='DATE', default=0,
                        help='YYYYMMDD for the start date')
    parser.add_argument('--end', metavar='DATE', default=99999999,
                        help='YYYYMMDD for the end date')
    args = parser.parse_args()

    schedlog_path = args.schedlogs
    perc = list(map(int, args.perc))
    perc.sort(reverse=True)
    stats = args.stats
    verbose = args.verbose
    long_time = args.long
    start_date = int(args.start)
    end_date = int(args.end)

    if not os.path.isdir(schedlog_path):
        print('Schedlog path is not a directory')
        exit(1)

    cycle_times = []
    num_consider_jobs = []
    num_preempt_jobs = []
    num_run_jobs = []
    num_top_jobs = []

    consider = 0
    preempt = 0
    run = 0
    top = 0

    for root, _, files in os.walk(schedlog_path):
        # Remove all non PBS log files and sort them to make sure we go through
        # the logs in chronological order
        files = [x for x in files if x.isdigit()]
        files = list(map(int, files))
        files = [x for x in files if x >= start_date and x <= end_date]
        files.sort()

        # initialize start_time and end_time here so they do not go out of
        # scope when we change files.  We want to handle the case of cycles
        # spanning files
        start_time = 0
        end_time = 0
        start_time_str = ''
        consider_jobs = {}
        for f in files:
            f = str(f)
            relpath = os.path.join(root, f)
            with open(relpath, 'r') as fd:
                for line in fd:
                    if 'Starting Scheduling Cycle' in line:
                        timestamp = line.split(";", 1)[0]
                        if '.' in timestamp:
                            dt_obj = time.strptime(
                                timestamp, '%m/%d/%Y %H:%M:%S.%f')
                        else:
                            dt_obj = time.strptime(
                                timestamp, '%m/%d/%Y %H:%M:%S')
                        start_time = time.mktime(dt_obj)
                        start_time_str = timestamp
                        if stats:
                            consider = 0
                            preempt = 0
                            run = 0
                            top = 0
                    elif 'Leaving Scheduling Cycle' in line:
                        timestamp = line.split(';', 1)[0]
                        if '.' in timestamp:
                            # timestamp contains microseconds
                            dt_obj = time.strptime(
                                timestamp, '%m/%d/%Y %H:%M:%S.%f')
                        else:
                            dt_obj = time.strptime(
                                timestamp, '%m/%d/%Y %H:%M:%S')
                        end_time = time.mktime(dt_obj)

                        # First file starts in the middle of the cycle
                        if start_time == 0:
                            continue
                        elif end_time < start_time:
                            print('File: %s, line: %s, end time %s < ' \
                                'start time %s, ignoring cycle' % (
                                    filename, line, end_time, start_time))
                            continue
                        ct = end_time - start_time
                        cycle_times.append(ct)
                        if ct > long_time:
                            print('Long Cycle: %s Start: %s End: %s' % \
                                (cvt_sec_to_str(ct), start_time_str, timestamp))
                        if stats is True:
                            num_consider_jobs.append(consider)
                            num_preempt_jobs.append(preempt)
                            num_run_jobs.append(run)
                            num_top_jobs.append(top)
                    elif stats is True:
                        if 'Considering job to run' in line:
                            consider += 1
                            sp = line.split(';')
                            if sp[4] in consider_jobs:
                                consider_jobs[sp[4]] += 1
                            else:
                                consider_jobs[sp[4]] = 1
                        elif 'Job preempted by' in line:
                            preempt += 1
                        elif 'Job run' in line:
                            run += 1
                        elif 'Job is a top job' in line:
                            top += 1

    print('Total cycles: ' + str(len(cycle_times)))
    cycle_times_arr = np.array(cycle_times)
    output_stats(cycle_times_arr, perc, 'cycle time', True)

    if stats is True:
        print('Total Jobs Considered: %d' % (len(consider_jobs)))
        print('Total jobs run %d' % sum(num_run_jobs))
        not_imm_run = [x for x in consider_jobs if consider_jobs[x] > 1]
        print('Jobs not run immediately: %d' % (len(not_imm_run)))

        print('Average scheduler stats per cycle:')

        consider_jobs_arr = np.array(num_consider_jobs)
        preempt_jobs_arr = np.array(num_preempt_jobs)
        run_jobs_arr = np.array(num_run_jobs)
        top_jobs_arr = np.array(num_top_jobs)

        if verbose is True:
            output_stats(consider_jobs_arr, perc, 'jobs considered')
            output_stats(preempt_jobs_arr, perc, 'jobs preempted')
            output_stats(top_jobs_arr, perc, 'top jobs')
            output_stats(run_jobs_arr, perc, 'jobs run')

        else:
            m = int(np.mean(consider_jobs_arr) + .5)
            print('Jobs considered: %d' % (m))

            m = int(np.mean(preempt_jobs_arr) + .5)
            print('Jobs preempted: %d' % (m))

            m = int(np.mean(top_jobs_arr) + .5)
            print('Top jobs: %d' % (m))

            m = int(np.mean(run_jobs_arr) + .5)
            print('Jobs run: %d' % (m))

