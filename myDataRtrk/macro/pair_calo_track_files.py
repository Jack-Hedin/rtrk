#!/usr/bin/env python3
import sys
import re
import os

def get_run_segment(filepath):
    """
    Extract run and segment numbers from an sPHENIX DST filename.
    Example: ...-00053877-00045.root → run=53877, segment=45
    """
    match = re.search(r'-(\d+)-(\d+)\.root$', filepath)
    if match:
        run = int(match.group(1))
        segment = int(match.group(2))
        return run, segment
    return None

def read_files(list_file):
    """
    Read a list of DST files into a dictionary: { run: { segment: path } }
    """
    runs = {}
    with open(list_file, 'r') as f:
        for line in f:
            path = line.strip()
            if not path:
                continue
            key = get_run_segment(path)
            if key:
                run, segment = key
                runs.setdefault(run, {})[segment] = path
    return runs

def create_condor_filelist_1to1(track_list_file, calo_list_file):
    """
    Create a single output file where each line corresponds to:
    1 CALO file + 1 TRACK file with matching segment numbers (1:1 ratio)
    """
    print("Reading input lists...")

    track_runs = read_files(track_list_file)
    calo_runs = read_files(calo_list_file)

    all_runs = sorted(set(track_runs.keys()) | set(calo_runs.keys()))
    print(f"Found {len(all_runs)} runs total.\n")

    for run in all_runs:
        print(f"Processing run {run}...")

        if run not in track_runs or run not in calo_runs:
            print(f"⚠️ Missing CALO or TRACK files for run {run}. Skipping.")
            continue

        output_file = f"matched_filelist_run_{run}.txt"
        count = 0

        with open(output_file, "w") as out:
            # Get all segments that exist in BOTH track and calo files
            common_segments = sorted(set(track_runs[run].keys()) & set(calo_runs[run].keys()))
            
            print(f"Found {len(common_segments)} common segments for run {run}")
            
            for segment in common_segments:
                calo_path = calo_runs[run][segment]
                track_path = track_runs[run][segment]
                
                # Write 1:1 pair: calo_file track_file
                out.write(f"{calo_path} {track_path}\n")
                count += 1

        if count > 0:
            print(f"Created {output_file} with {count} 1:1 pairs.\n")
        else:
            os.remove(output_file)
            print(f"No matching segments found for run {run}. Skipping.\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 pair_calo_track_files.py <track_filelist.txt> <calo_filelist.txt>")
        sys.exit(1)

    track_list = sys.argv[1]
    calo_list = sys.argv[2]
    create_condor_filelist_1to1(track_list, calo_list)