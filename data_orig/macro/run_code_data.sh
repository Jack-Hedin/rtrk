#! /bin/bash

export USER="$(id -u -n)"
export LOGNAME=${USER}
export HOME=/sphenix/u/${LOGNAME}
 
source /opt/sphenix/core/bin/sphenix_setup.sh -n ana.515
export MYINSTALL=~/install
source /opt/sphenix/core/bin/setup_local.sh $MYINSTALL
 
# print the environment - needed for debugging
printenv

CALO_SEG=$1
echo "Processing segment: $CALO_SEG"


# Correct ROOT call with proper separation
root -b -q "/sphenix/user/abrahma/datartrk/macro/Fun4All_JetAna_Data.C(45000, $CALO_SEG, \"calo_filelist_run_53877.txt\",\"track_filelist_run_53877.txt\")"


echo all done


