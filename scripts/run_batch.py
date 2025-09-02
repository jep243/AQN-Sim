import argparse
import os
import sys
from dataclasses import dataclass
from queue import Queue
from threading import Thread
import subprocess
import time

# Define arguments
parser = argparse.ArgumentParser(description="Script for running ns3 batches")
parser.add_argument("input_file", help="The file containing run parameters.")
parser.add_argument("output_dir", help="The directory for output files.")
parser.add_argument("threads", default=10, help="Number of threads used.")
args = parser.parse_args()

# Verify and read in input file
input_file = args.input_file
if not os.path.isfile(input_file):
	print(f"Invalid input file: {input_file}")
	sys.exit()

@dataclass
class RunConfig:
	nodes_per_switch: int
	cluster_size: int
	num_clusters: int
	send_time: int
	reconfigure_time: int
	num_channels: int

configs = Queue()
with open(input_file, "r") as f:
	lines = f.readlines()
	for line in lines:
		name, _ = os.path.splitext(line)
		values = name.split("_")
		configs.put(RunConfig(int(values[1]),
							  int(values[2]),
							  int(values[3]),
							  int(values[4]),
							  int(values[5]),
							  int(values[6])))

# Verify the output directory
output_dir = args.output_dir
if not os.path.isdir(output_dir):
	print(f"Invalid output directory: {output_dir}")
	sys.exit()

# Get the number of threads and define the worker function
if not args.threads.isdigit():
	print(f"Invalid number of threads: {args.threads}")
	sys.exit()
num_workers = int(args.threads)

def worker():
	global configs
	global output_dir
	running = True
	while running:
		if configs.empty():
			running = False
			continue
		config_data = configs.get()
		p = subprocess.Popen(["./instance.sh", 
							  str(config_data.nodes_per_switch),
							  str(config_data.cluster_size),
							  str(config_data.num_clusters),
							  str(config_data.send_time),
							  str(config_data.reconfigure_time),
							  str(config_data.num_channels),
							  output_dir])
		while p.poll() is None:
			time.sleep(1)

# Start the threads
threads = []
for _ in range(num_workers):
	t = Thread(target=worker)
	t.start()
for t in threads:
	t.join()
