from kuksa_client.grpc import VSSClient

DIGITAL_AUTO_IP = '127.0.0.1'

def getCurrentValue(vapi):
    try:
        with VSSClient(DIGITAL_AUTO_IP, 55555) as client:
            # print("getCurrentValue ----     ")
            current_values = client.get_current_values([vapi])
            if current_values[vapi] is not None:
                value = current_values[vapi].value
                # print(current_values[vapi].value)
                timestamp = current_values[vapi].timestamp 
                # print(current_values[vapi].value)
                # print(f"Value: {value}, Timestamp: {timestamp}")
                return value, str(timestamp)
            else:
                # print("getCurrentValue is None")
                return "nullstring"  # Ensure a string is always returned
        if client is None:
            print("getCurrentValue ----     ")
            return "nullstring"
    except Exception as e:
            # Catch any other exceptions
            print(f"An unexpected error occurred: {e}")
            return None

def getVssCurrentValue(vapi):
    return getCurrentValue(vapi)
    

def getTargetValue(vapi):
    try:
        with VSSClient(DIGITAL_AUTO_IP, 55555) as client:
            current_values = client.get_target_values([vapi])
            if current_values[vapi] is not None:
                value = current_values[vapi].value
                return value
            else:
                return "nullstring"  # Ensure a string is always returned
        if client is None:
            print("getCurrentValue ----     ")
            return "nullstring"
    except Exception as e:
        # Catch any other exceptions
        print(f"An unexpected error occurred: {e}")
        return None

def getVssTargetValue(vapi):
    return getTargetValue(vapi)
    

# def getVssTargetValue():
#     return getTargetValue("Vehicle.TextToSpeech")

# getVssCurrentValue("Vehicle.TextToSpeech")
# getVssTargetValue("Vehicle.TextToSpeech")
