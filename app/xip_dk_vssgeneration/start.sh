#!/bin/sh

echo "Start VSS Generation"
cd /app/vehicle_signal_specification

# Capture both stdout and stderr
output1=$(python vss-tools/vspec2json.py -e vss2dbc,dbc2vss,dbc -o /app/.dk/dk_manager/vssmapping/vssmapping_overlay.vspec --json-pretty ./spec/VehicleSignalSpecification.vspec /app/dk_vssgeneration/vss.json 2>&1)

# Debug: Show the captured output
echo "DEBUG: Output from VSS generation:"
echo "$output1"

# Check if the output contains the phrase "All done"
if echo "$output1" | grep -q "All done"; then
    echo "VSS GEN ALL DONE"
else
    echo "VSS GEN PROCESS FAILED OR INCOMPLETE"
fi

echo "End VSS Generation"
echo "===================================="
echo "Start VModel Generation"
cd /app/vehicle-model-generator

# Capture both stdout and stderr
output2=$(python src/velocitas/model_generator/cli.py /app/dk_vssgeneration/vss.json -I /app/vehicle_signal_specification/spec -u ../vehicle_signal_specification/spec/units.yaml -T /app/dk_vssgeneration/vehicle_gen 2>&1)

# Debug: Show the captured output
echo "DEBUG: Output from VModel generation:"
echo "$output2"

# Check if the output contains the phrase "All done"
if echo "$output2" | grep -q "All done"; then
    echo "VMODEL GEN ALL DONE"
else
    echo "VMODEL GEN PROCESS FAILED OR INCOMPLETE"
fi

echo "End VModel Generation"
