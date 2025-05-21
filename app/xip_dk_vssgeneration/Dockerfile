FROM python:3.12-alpine

WORKDIR /app/

# Copy files into the container
COPY vehicle_signal_specification /app/vehicle_signal_specification
COPY vehicle-model-generator /app/vehicle-model-generator
COPY start.sh /app/

# Set execute permission for the script
RUN chmod +x /app/start.sh

# Install the necessary Python packages
RUN pip install pyyaml anytree deprecation graphql-core stringcase setuptools
RUN pip install -r /app/vehicle-model-generator/requirements.txt

# Set var env
ENV PYTHONPATH=$PYTHONPATH:/app/vehicle-model-generator/src/

# Execute the script
CMD ["/app/start.sh"]
