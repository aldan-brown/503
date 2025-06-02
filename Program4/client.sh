#!/bin/sh
g++ Client.cpp -o client

chmod 755 client

# Define buffer combinations
buf_combinations="15 100
30 50
60 25
100 15"

# Define test types
test_types="1 2 3"

# Clear output file
> output.txt

# Run combinations
echo "$buf_combinations" | while read nbufs bufsize
do
    for type in $test_types
    do
      for run in 1 2 3 4 5
      do
         echo "Running: csslab8.uwb.edu 04171 20000 $nbufs $bufsize $type" >> output.txt
         ./client csslab8.uwb.edu 04171 20000 $nbufs $bufsize $type | grep "Test(" >> output.txt
         done
    done
done