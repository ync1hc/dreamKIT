import cantools
import json
import os

# Load the .dbc file
dbc_file_path = 'ModelCAN.dbc'
db = cantools.database.load_file(dbc_file_path)

# Define the name of the signal you're looking for and the default value for signals
# target_signal_name = 'DAS_hazardLightRequest'
target_signal_name = 'VCLEFT_turnSignalStatus1'

default_value = 0

# Load existing JSON file if it exists and is not empty; otherwise, start with an empty dictionary
json_file_path = 'dbc_default_values_test.json'
if os.path.exists(json_file_path) and os.path.getsize(json_file_path) > 0:
    with open(json_file_path, 'r') as json_file:
        dbc_defaults = json.load(json_file)
else:
    dbc_defaults = {}

# Search for the signal across all messages
message_found = None
for message in db.messages:
    if any(signal.name == target_signal_name for signal in message.signals):
        message_found = message
        break

# If the signal is found, add all signals from the message to dbc_defaults
if message_found:
    for signal in message_found.signals:
        dbc_defaults[signal.name] = default_value
else:
    print(f"Signal '{target_signal_name}' not found in any message in the .dbc file.")

# Write updated defaults to the JSON file
with open(json_file_path, 'w') as json_file:
    json.dump(dbc_defaults, json_file, indent=2)

print(f"Updated '{json_file_path}' with signals from the message containing '{target_signal_name}'.")
