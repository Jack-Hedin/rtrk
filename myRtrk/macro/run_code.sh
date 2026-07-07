#! /bin/bash

export USER="$(id -u -n)"
export LOGNAME=${USER}
export HOME=/sphenix/u/${LOGNAME}
 
source /opt/sphenix/core/bin/sphenix_setup.sh -n
#source /opt/sphenix/core/bin/sphenix_setup.sh -n ana.515
export MYINSTALL=~/install
source /opt/sphenix/core/bin/setup_local.sh $MYINSTALL
 
# print the environment - needed for debugging
printenv

#printf -v SEG "%06d" $1
SEG=$1

echo "Processing segment: $SEG"

# Correct ROOT call with all quotes escaped
root -b -q "/sphenix/user/abrahma/rtrk/macro/Fun4All_JetAna.C(-1, $SEG, \"dst_calo_cluster.list\",\"dst_global.list\",\"dst_tracks.list\",\"dst_truth_jet.list\",\"dst_truth.list\")"

echo all done
