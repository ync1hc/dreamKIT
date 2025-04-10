cmake_minimum_required(VERSION 3.10)
project(kuksa_databroker)

# Use C++11 standard.
set(CMAKE_CXX_STANDARD 11)

# Find the required packages.
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Explicitly locate the grpc_cpp_plugin (it should be installed in /usr/local/bin in our Docker build).
find_program(GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin HINTS /usr/local/bin)
if(NOT GRPC_CPP_PLUGIN_EXECUTABLE)
  message(FATAL_ERROR "grpc_cpp_plugin not found. Please ensure it is installed and accessible.")
endif()

# Set the PROTO_INCLUDE directory to the parent of the "kuksa" folder.
set(PROTO_INCLUDE ${PROJECT_SOURCE_DIR}/proto)

# Define the proto file paths.
set(TYPES_PROTO ${PROTO_INCLUDE}/kuksa/val/v1/types.proto)
set(VAL_PROTO   ${PROTO_INCLUDE}/kuksa/val/v1/val.proto)

# Create output directory for generated files.
set(GENERATED_DIR ${PROJECT_SOURCE_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

# Generate protobuf code for types.proto.
add_custom_command(
  OUTPUT ${GENERATED_DIR}/kuksa/val/v1/types.pb.cc ${GENERATED_DIR}/kuksa/val/v1/types.pb.h
  COMMAND ${Protobuf_PROTOC_EXECUTABLE}
          --cpp_out=${GENERATED_DIR}
          -I ${PROTO_INCLUDE}
          ${TYPES_PROTO}
  DEPENDS ${TYPES_PROTO}
  COMMENT "Generating C++ code from ${TYPES_PROTO}"
)

# Generate protobuf code for val.proto.
add_custom_command(
  OUTPUT ${GENERATED_DIR}/kuksa/val/v1/val.pb.cc ${GENERATED_DIR}/kuksa/val/v1/val.pb.h
  COMMAND ${Protobuf_PROTOC_EXECUTABLE}
          --cpp_out=${GENERATED_DIR}
          -I ${PROTO_INCLUDE}
          ${VAL_PROTO}
  DEPENDS ${VAL_PROTO}
  COMMENT "Generating C++ code from ${VAL_PROTO}"
)

# Generate gRPC code for val.proto.
add_custom_command(
  OUTPUT ${GENERATED_DIR}/kuksa/val/v1/val.grpc.pb.cc ${GENERATED_DIR}/kuksa/val/v1/val.grpc.pb.h
  COMMAND ${Protobuf_PROTOC_EXECUTABLE}
          --grpc_out=${GENERATED_DIR}
          --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
          -I ${PROTO_INCLUDE}
          ${VAL_PROTO}
  DEPENDS ${VAL_PROTO}
  COMMENT "Generating gRPC code from ${VAL_PROTO}"
)

# List all generated source files.
set(GENERATED_SRCS
    ${GENERATED_DIR}/kuksa/val/v1/types.pb.cc
    ${GENERATED_DIR}/kuksa/val/v1/val.pb.cc
    ${GENERATED_DIR}/kuksa/val/v1/val.grpc.pb.cc
)
set(GENERATED_HDRS
    ${GENERATED_DIR}/kuksa/val/v1/types.pb.h
    ${GENERATED_DIR}/kuksa/val/v1/val.pb.h
    ${GENERATED_DIR}/kuksa/val/v1/val.grpc.pb.h
)

# Define your client source file(s) (adjust as needed).
# In this example, we assume you have "client.cpp" in the project root.
set(CLIENT_SRCS   main.cpp
                  KuksaClient.cpp
)

# Create an executable target.
add_executable(KuksaDatabrokerClient ${CLIENT_SRCS} ${GENERATED_SRCS})

# Add include directories. Add GENERATED_DIR so that headers can be included using their full relative path.
target_include_directories(KuksaDatabrokerClient PRIVATE ${GENERATED_DIR} ${Protobuf_INCLUDE_DIRS})

# Link against gRPC and protobuf libraries.
target_link_libraries(KuksaDatabrokerClient PRIVATE gRPC::grpc++ protobuf::libprotobuf)
