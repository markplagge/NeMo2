from operator import itemgetter

from numba import jit
from joblib import Parallel, delayed
import numpy as np

event_list = """CIFAR 100,1,start_running,1"""
runs = open('rr_data.csv', 'r').read().split('\n')
r2 = set(runs)


def split_line(line):
    model_name, job_id, event, time = line.split(",")
    time = int(time)
    job_id = int(job_id)
    model_name = np.str(model_name)
    event = np.str(event)

    return [model_name, job_id, event, time]


r2 = [m for m in r2 if len(m) > 1]
r3 = map(split_line, list(r2))
r4 = []
for m in r3:
    r4.append(m)

r4 = sorted(r4,key=itemgetter(3))


def generate_command(line1, line2):
    model = line1[0]
    id = line1[1]
    start_time = line1[3]
    end_time = line2[3]

000 = {list: 4} ['CIFAR 100', 1, 'start_running', 1]
001 = {list: 4} ['CIFAR 100', 1, 'interrupted', 200]
002 = {list: 4} ['SAR', 2, 'start_running', 201]
003 = {list: 4} ['SAR', 2, 'interrupted', 400]
004 = {list: 4} ['Tonic', 3, 'start_running', 401]