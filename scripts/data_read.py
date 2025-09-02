import numpy as np
import matplotlib.pyplot as plt
import os
import random
import pandas as pd
from tqdm import tqdm
import pickle

class RunData:

    def __init__(self, file_path):
		name, ext = os.path.splitext(file_path)
        tokens = name.split("_")
        self.nodes_per_switch = int(tokens[1])
        self.cluster_size = int(tokens[2])
        self.num_clusters = int(tokens[3])
        self.send_time = int(tokens[4])
        self.reconfigure = int(tokens[5])
        self.channels = int(tokens[6])
        self.c_error = 0
		self.q_error = 0
		if len(tokens) >= 9:
			self.c_error = int(tokens[7])
			self.q_error = int(tokens[8])
		self.drop_count = 0
        self.ids = []
        self.sent_by_id = {}
        self.total_sent = 0
        self.total_finished = 0
        self.tx_full_columns = ["app_id", "rx_qubits", "tx_qubits", "time"]
        self.tx_full = pd.DataFrame(columns = self.tx_full_columns)
        self.collision_count = 0
        self.packet_columns = ["id", "sender", "protocol", "nacks", "awks", "rx_q_time", "tx_q_time", "total_time", "app_id"]
        self.packet_data = pd.DataFrame(columns = self.packet_columns)
        with open(file_path, "r") as f:
            lines = f.readlines()
            for line in tqdm(lines):
                values = line.split(",")
                if values[0] == "TxFull":
                    id = int(values[1])
                    rx_q = int(values[2])
                    tx_q = int(values[3])
                    time = int(values[4])
                    new_row = pd.DataFrame([[id, rx_q, tx_q, time]], columns=self.tx_full_columns)
                    self.tx_full = pd.concat([self.tx_full, new_row], ignore_index=True)
                    if not id in self.ids:
                        self.ids.append(id)
                elif "TotalSent" in values[0]:
                    t = values[0].split(":")
                    count = int(t[1])
                    id = int(values[1])
                    self.sent_by_id[id] = count
                    self.total_sent += count
                    if not id in self.ids:
                        self.ids.append(id)
                elif values[0] == "CollisionCount":
                    self.collision_count = int(values[1])
                elif values[0] == "DropCount":
                    self.drop_count = int(values[1])
                elif values[0].isdigit():
                    if not len(values) == 9:
                        print(file_path)
                        print(values)
                    id = int(values[0])
                    sender = values[1] == "1"
                    protocol = int(values[2])
                    nacks = int(values[3])
                    awks = int(values[4])
                    rx_q_time = int(values[5])
                    tx_q_time = int(values[6])
                    total_time = int(values[7])
                    app_id = int(values[8])
                    new_row = pd.DataFrame([[id, sender, protocol, nacks, awks, 
											 rx_q_time, tx_q_time, total_time, 
											 app_id]],
                                           columns=self.packet_columns)
                    self.packet_data = pd.concat([self.packet_data, new_row], 
												  ignore_index=True)
                    self.total_finished += 1
                else:
                    print(f"Found unknown type: {values[0]}")

# Read in all the files
all_data = []
directory = "output/e3/"
for filename in os.listdir(directory):
    file_path = os.path.join(directory, filename)
    if os.path.isfile(file_path):
        print(f"Running {file_path}")
        all_data.append(RunData(file_path))
        print(f"Finished {file_path}")

# Write the data objects to binary file
output_file = "output/e3.bin"
with open(output_file, "wb") as f:
    pickle.dump(all_data, f)

# Load data
#with open('file.bin', 'rb') as f:
#    loaded_objects = pickle.load(f)
