#!/bin/bash

qubits=7
q_error=0
c_error=0
skew=0
max_tx_queue=10000
max_prop=110
packet_processing=350
control_tx_time=14
data_tx_time=30
packet_delay_padding=100
timeslot_padding=100
packet_delay=$(((6 * (packet_processing + control_tx_time)) \
			 + max_prop + packet_delay_padding))
timeslot=$((packet_delay + max_prop + data_tx_time + timeslot_padding))

send_time=100000000
send_rng=$((send_time / 10))
reconfigure=100
num_channels=40

nodes_per_switch=3
cluster_size=3
num_clusters=4

debug_level=${1:-0}
use_gdb=${2:-0}

run_string="sim --qubits=${qubits} --qerror=${q_error} \
		    --cerror=${c_error} --send-time=${send_time} \
			--send-range=${send_rng} --skew=${skew} \
			--reconfigure=${reconfigure} --timeslot=${timeslot} \
			--packet-delay=${packet_delay} --max-tx-queue=${max_tx_queue} \
			--num-channels=${num_channels} \
			--nodes-per-switch=${nodes_per_switch} \
			--cluster-size=${cluster_size} --num-clusters=${num_clusters} \
			--debug=${debug_level}"

if [ $use_gdb -eq 1 ]; then
	../../../ns3 run "$run_string" --gdb
else
	../../../ns3 run "$run_string"
fi
