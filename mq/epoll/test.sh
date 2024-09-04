#!/bin/bash

# Loop to send 20 different messages
for i in {1..20}
do
    echo "Hello Server $i" | nc 127.0.0.1 6000 &
done