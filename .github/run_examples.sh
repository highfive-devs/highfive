#! /usr/bin/env bash

set -eu

if [[ $# -eq 0 ]]
then
  examples_dir="."
elif [[ $# -eq 1 ]]
then
  examples_dir="$1"
else
  echo "Usage: $0 [EXAMPLES_DIR]"
  exit -1
fi

for f in "${examples_dir}"/*_bin
do
  if [[ "${f}" == *"swmr_"* ]]
  then
    continue
  fi

  echo "-- ${f}"
  if [[ "${f}" == *"parallel_"* ]]
  then
    mpiexec -np 2 "${f}"
  else
    "${f}"
  fi
done

for f in "${examples_dir}"/swmr_*_bin
do
  echo "-- ${f}"
  "${f}" &
done

wait
