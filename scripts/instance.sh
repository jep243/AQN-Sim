#!/bin/bash

nodes_per_switch=$1
cluster_size=$2
num_clusters=$3

qubits=7
q_error=0
c_error=0
skew=0
max_tx_queue=10000
max_prop=$(((((nodes_per_switch + 1) + (2 * (nodes_per_switch)) + \
		    (5 *((cluster_size / 2) + 1))) * 3 * 5) + (5 * 2)))
packet_processing=350
control_tx_time=14
data_tx_time=30
packet_delay_padding=100
timeslot_padding=100
packet_delay=$(((6 * (packet_processing + control_tx_time)) \
			 + max_prop + packet_delay_padding))
timeslot=$((packet_delay + max_prop + data_tx_time + timeslot_padding))

send_time=$4
send_rng=$((send_time / 10))
reconfigure=$5
num_channels=$6

out_file="${7}/run_${nodes_per_switch}_\
${cluster_size}_${num_clusters}_${send_time}_${reconfigure}_${num_channels}.csv"
echo "Output File: ${out_file}"
echo "Starting run ${nodes_per_switch}_${cluster_size}_${num_clusters}_\
${send_time}_${reconfigure}_${num_channels}"

../../../ns3 run "\"sim --qubits=${qubits} --qerror=${q_error} \
			  --cerror=${c_error} --send-time=${send_time} \
			  --send-range=${send_rng} --skew=${skew} \
			  --reconfigure=${reconfigure} --timeslot=${timeslot} \
			  --packet-delay=${packet_delay} --max-tx-queue=${max_tx_queue} \
			  --num-channels=${num_channels} \
			  --nodes-per-switch=${nodes_per_switch} \
			  --cluster-size=${cluster_size} --num-clusters=${num_clusters} \
			  --debug=0\"" > $out_file

echo "Finished run ${nodes_per_switch}_${cluster_size}_${num_clusters}_\
${send_time}_${reconfigure}_${num_channels}"
