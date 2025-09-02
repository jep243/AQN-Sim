import argparse
import os
import sys
from queue import Empty
from queue import Queue
from threading import Thread
import subprocess
import time
from read_data import RunData
import pickle

# Define arguments
parser = argparse.ArgumentParser(description="Script for running ns3 batches")
parser.add_argument("input_dir", help="The file containing run parameters.")
parser.add_argument("output_file", help="The directory for output files.")
parser.add_argument("threads", default=10, help="Number of threads used.")
args = parser.parse_args()

# Verify input dir
input_dir = args.input_dir
output_file = args.output_file
if not os.path.isdir(input_dir):
	print(f"Invalid input dir: {input_dir}")
	sys.exit()

# Get the number of threads and define the worker function
if not args.threads.isdigit():
	print(f"Invalid number of threads: {args.threads}")
	sys.exit()
num_workers = int(args.threads)

# Get all the input files
file_queue = Queue()
for filename in os.listdir(input_dir):
	file_path = os.path.join(input_dir, filename)
	if os.path.isfile(file_path):
		file_queue.put(file_path)

output_queue = Queue()
def worker():
	global file_queue
	global output_queue
	running = True
	while running:
		try:
			file_path = file_queue.get(block=False)
			file_name, ext = os.path.splitext(file_path)
			out_path = file_name + ".bin"
			output_queue.put(out_path)
			print(f"Started encoding {file_path}")
			p = subprocess.Popen(['python', "read_data.py", file_path, out_path]) 
			while p.poll() is None:
				time.sleep(1)
			print(f"Finished encoding {file_path}")
		except Empty:
			running = False
			break
	print("Thread Exiting")

# Start the threads
threads = []
for _ in range(num_workers):
	t = Thread(target=worker)
	t.start()
	threads.append(t)
for t in threads:
	t.join()

# Write out the final data
print("Writing Data to File")
out_data = []
while output_queue:
	try:
		path = output_queue.get(block=False)
		if os.path.isfile(path):
			with open(path, "rb") as f:
				data = pickle.load(f)
				out_data.append(data)
	except Empty:
		break
with open(output_file, "wb") as f:
	pickle.dump(out_data, f)
print("Finished")
