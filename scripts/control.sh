#!/bin/bash

reconfigure_times=(
	100 1000 10000
	)
send_times=(
	5000 50000 500000
	)
channels=(
	50
	)
switch_count=(
	2 3 4
	)
cluster_size=(
	2 3 4
	)
clusters_count=(
	2 3 4
	)

num_reconfigure=${#reconfigure_times[@]}
num_send=${#send_times[@]}
num_channels=${#channels[@]}
num_switches=${#switch_count[@]}
num_clusters=${#cluster_size[@]}
num_cluster_counts=${#clusters_count[@]}
total_run=$((num_reconfigure * num_send * num_channels * \
			 num_switches * num_clusters * num_cluster_counts))
num_proc=$(nproc)
num_instance=$((num_proc / 8))
echo $num_instance
for ((i=0; i<${total_run}; i++)); do
	r_index=$((i / (num_send * num_channels * num_switches * \
			   num_clusters * num_cluster_counts)))
	s_index=$(((i / (num_cluster_counts * num_clusters * num_switches *\
			   num_channels)) % num_send))
	c_index=$(((i / (num_cluster_counts * num_clusters * num_switches)) \
				% num_channels))
	sc_index=$(((i / (num_cluster_counts * num_clusters)) % num_switches))
	cs_index=$(((i / num_cluster_counts) % num_clusters))
	cc_index=$((i % num_cluster_counts))
	./instance.sh ${switch_count[$sc_index]} \
				  ${cluster_size[$cs_index]} \
				  ${clusters_count[$cc_index]} \
				  ${send_times[$s_index]} \
				  ${reconfigure_times[$r_index]} \
				  ${channels[$c_index]} &
	if [ $((i % num_instance)) -eq $((num_instance - 1)) ]; then
		wait
	fi
done
