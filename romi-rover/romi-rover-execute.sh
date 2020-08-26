#!/bin/bash
# Execute the romi rover, first checking the port configuration.
# Usage: romi-rover-execute.sh <path-to-config-file.json>

echo "config file: $1";
./rcdiscover $1
./rclaunch -C $1


