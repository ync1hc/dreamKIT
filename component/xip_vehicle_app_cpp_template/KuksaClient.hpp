#ifndef KUKSA_CLIENT_HPP
#define KUKSA_CLIENT_HPP

#include <vector>
#include <string>
#include <thread>
#include <memory>
#include <iostream>
#include <sstream>

// gRPC and Protobuf headers.
#include <grpcpp/grpcpp.h>
#include "kuksa/val/v1/val.grpc.pb.h"
#include "kuksa/val/v1/types.pb.h"

// Include the nlohmann/json header
#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace KuksaClient {

//------------------------------------------------------------------------------
// Configuration structure & ConfigParser
//------------------------------------------------------------------------------
struct Config {
  std::string serverURI;
  bool debug = false;
  std::vector<std::string> signalPaths;
};

class ConfigParser {
public:
  // Parses the JSON file and loads the configuration into config.
  // Returns true if parsing was successful.
  static bool parse(const std::string &filename, Config &config);
};

//------------------------------------------------------------------------------
// DataBrokerClient: Provides basic RPC methods to interact with the server.
//------------------------------------------------------------------------------
class DataBrokerClient {
public:
  DataBrokerClient();

  // Connect to the server using the provided URI.
  void Connect(const std::string &serverURI);

  // Get the current value for an entry.
  void GetValue(const std::string &entryPath);

  // Templated SetValue() function. It supports various types.
  template<typename T>
  void SetValue(const std::string &entryPath, const T &newValue) {
    if (!stub_) {
      std::cout << "Client not connected. Aborting SetValue()." << std::endl;
      return;
    }
    kuksa::val::v1::SetRequest request;
    auto* update = request.add_updates();
    kuksa::val::v1::DataEntry* data_entry = update->mutable_entry();
    data_entry->set_path(entryPath);

    // Call the helper overload to set the value.
    setValueImpl(data_entry->mutable_value(), newValue);

    update->add_fields(kuksa::val::v1::FIELD_VALUE);

    kuksa::val::v1::SetResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->Set(&context, request, &response);
    if (!status.ok()) {
      std::cout << "Set() RPC failed: " << status.error_message() << std::endl;
      return;
    }
    if (response.error().code() != 0) {
      std::cout << "Set() global error: " << response.error().message() << std::endl;
    } else {
      std::cout << "SetValue(): Updated \"" << entryPath << "\" to " 
                << DataPointToString(data_entry->value()) << std::endl;
    }
  }

  // A streaming update example.
  void StreamedUpdate(const std::string &entryPath, float newValue);

  // Subscribe to changes for an entry.
  void Subscribe(const std::string &entryPath);

  // Get server info.
  void GetServerInfo();

private:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<kuksa::val::v1::VAL::Stub> stub_;

  // Helper to convert Datapoint to string.
  static std::string DataPointToString(const kuksa::val::v1::Datapoint &dp);

  // Overloaded helper functions to set a value in a Datapoint message.
  static void setValueImpl(kuksa::val::v1::Datapoint *dp, float value);
  static void setValueImpl(kuksa::val::v1::Datapoint *dp, double value);
  static void setValueImpl(kuksa::val::v1::Datapoint *dp, int32_t value);
  static void setValueImpl(kuksa::val::v1::Datapoint *dp, int64_t value);
  static void setValueImpl(kuksa::val::v1::Datapoint *dp, uint32_t value);
  static void setValueImpl(kuksa::val::v1::Datapoint *dp, uint64_t value);
  static void setValueImpl(kuksa::val::v1::Datapoint *dp, bool value);
  static void setValueImpl(kuksa::val::v1::Datapoint *dp, const std::string &value);
};

//------------------------------------------------------------------------------
// SubscriptionManager: Manages subscription threads for given signal paths.
//------------------------------------------------------------------------------
class SubscriptionManager {
public:
  SubscriptionManager(DataBrokerClient &client, const std::vector<std::string> &paths);
  void startSubscriptions();
  void joinAll();
  void detachAll();

private:
  DataBrokerClient &client_;
  std::vector<std::string> signalPaths_;
  std::vector<std::thread> threads_;
};

} // namespace KuksaClient

#endif // KUKSA_CLIENT_HPP
