#include "KuksaClient.hpp"
#include <fstream>

namespace KuksaClient {

//------------------------------------------------------------------------------
// ConfigParser implementation
//------------------------------------------------------------------------------
bool ConfigParser::parse(const std::string &filename, Config &config) {
  std::ifstream configFile(filename);
  if (!configFile.is_open()) {
    std::cerr << "Unable to open config file: " << filename << std::endl;
    return false;
  }
  try {
    json j;
    configFile >> j;
    config.serverURI = j["broker"]["serverURI"].get<std::string>();
    config.debug = j.value("debug", false);
    if (j.find("signal") != j.end() && j["signal"].is_array()) {
      for (auto &item : j["signal"]) {
        if (item.find("path") != item.end())
          config.signalPaths.push_back(item["path"].get<std::string>());
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "Failed to parse " << filename << ": " << e.what() << std::endl;
    return false;
  }
  return true;
}

//------------------------------------------------------------------------------
// DataBrokerClient implementation
//------------------------------------------------------------------------------
DataBrokerClient::DataBrokerClient()
  : channel_(nullptr), stub_(nullptr) { }

void DataBrokerClient::Connect(const std::string &serverURI) {
  channel_ = grpc::CreateChannel(serverURI, grpc::InsecureChannelCredentials());
  stub_ = kuksa::val::v1::VAL::NewStub(channel_);
  std::cout << "Connected to " << serverURI << std::endl;
}

void DataBrokerClient::GetValue(const std::string &entryPath) {
  if (!stub_) {
    std::cout << "Client not connected. Aborting GetValue()." << std::endl;
    return;
  }
  kuksa::val::v1::GetRequest request;
  auto *entryReq = request.add_entries();
  entryReq->set_path(entryPath);
  entryReq->set_view(kuksa::val::v1::VIEW_CURRENT_VALUE);

  kuksa::val::v1::GetResponse response;
  grpc::ClientContext context;
  grpc::Status status = stub_->Get(&context, request, &response);
  if (!status.ok()) {
    std::cout << "Get() RPC failed: " << status.error_message() << std::endl;
    return;
  }
  if (response.error().code() != 0) {
    std::cout << "Get() global error: " << response.error().message() << std::endl;
    return;
  }
  if (response.entries_size() == 0) {
    std::cout << "Get(): No entries returned for \"" << entryPath << "\"." << std::endl;
    return;
  }
  const auto &data_entry = response.entries(0);
  std::cout << "Get(): Value for \"" << entryPath << "\": " 
            << DataPointToString(data_entry.value()) << std::endl;
}

void DataBrokerClient::StreamedUpdate(const std::string &entryPath, float newValue) {
  if (!stub_) {
    std::cout << "Client not connected. Aborting StreamedUpdate()." << std::endl;
    return;
  }
  grpc::ClientContext context;
  auto stream = stub_->StreamedUpdate(&context);
  kuksa::val::v1::StreamedUpdateRequest request;
  auto *update = request.add_updates();
  kuksa::val::v1::DataEntry* data_entry = update->mutable_entry();
  data_entry->set_path(entryPath);
  data_entry->mutable_value()->set_float_(newValue);
  update->add_fields(kuksa::val::v1::FIELD_VALUE);

  if (!stream->Write(request)) {
    std::cout << "StreamedUpdate: Failed to write request." << std::endl;
    return;
  }
  stream->WritesDone();
  kuksa::val::v1::StreamedUpdateResponse response;
  while (stream->Read(&response)) {
    if (response.error().code() != 0) {
      std::cout << "StreamedUpdate response error: " << response.error().message() << std::endl;
    } else {
      std::cout << "StreamedUpdate: Received a response." << std::endl;
    }
  }
  grpc::Status status = stream->Finish();
  if (!status.ok()) {
    std::cout << "StreamedUpdate RPC failed: " << status.error_message() << std::endl;
  } else {
    std::cout << "StreamedUpdate: Completed successfully." << std::endl;
  }
}

void DataBrokerClient::Subscribe(const std::string &entryPath) {
  if (!stub_) {
    std::cout << "Client not connected. Aborting Subscribe()." << std::endl;
    return;
  }
  kuksa::val::v1::SubscribeRequest request;
  auto *subEntry = request.add_entries();
  subEntry->set_path(entryPath);
  subEntry->set_view(kuksa::val::v1::VIEW_ALL);
  subEntry->add_fields(kuksa::val::v1::FIELD_VALUE);

  grpc::ClientContext context;
  auto reader = stub_->Subscribe(&context, request);
  std::cout << "Subscribe: Listening on \"" << entryPath << "\"." << std::endl;
  kuksa::val::v1::SubscribeResponse response;
  int updateCount = 0;
  while (reader->Read(&response)) {
    updateCount++;
    std::cout << "Subscribe: Received update #" << updateCount 
              << " for \"" << entryPath << "\"" << std::endl;
    for (int i = 0; i < response.updates_size(); i++) {
      const auto &update = response.updates(i);
      std::cout << "   Update: Path: " << update.entry().path()
                << " Value: " << DataPointToString(update.entry().value()) << std::endl;
    }
  }
  grpc::Status status = reader->Finish();
  if (!status.ok()) {
    std::cout << "Subscribe RPC failed: " << status.error_message() << std::endl;
  } else {
    std::cout << "Subscribe: Stream finished for \"" << entryPath << "\"." << std::endl;
  }
}

void DataBrokerClient::GetServerInfo() {
  if (!stub_) {
    std::cout << "Client not connected. Aborting GetServerInfo()." << std::endl;
    return;
  }
  kuksa::val::v1::GetServerInfoRequest request;
  kuksa::val::v1::GetServerInfoResponse response;
  grpc::ClientContext context;
  grpc::Status status = stub_->GetServerInfo(&context, request, &response);
  if (!status.ok()) {
    std::cout << "GetServerInfo RPC failed: " << status.error_message() << std::endl;
    return;
  }
  std::cout << "Server Info:" << std::endl;
  std::cout << "  Name:    " << response.name() << std::endl;
  std::cout << "  Version: " << response.version() << std::endl;
}

//------------------------------------------------------------------------------
// Helper functions for DataBrokerClient
//------------------------------------------------------------------------------
std::string DataBrokerClient::DataPointToString(const kuksa::val::v1::Datapoint &dp) {
  std::ostringstream oss;
  switch (dp.value_case()) {
    case kuksa::val::v1::Datapoint::kString:
      oss << dp.string();
      break;
    case kuksa::val::v1::Datapoint::kBool:
      oss << (dp.bool_() ? "true" : "false");
      break;
    case kuksa::val::v1::Datapoint::kInt32:
      oss << dp.int32();
      break;
    case kuksa::val::v1::Datapoint::kInt64:
      oss << dp.int64();
      break;
    case kuksa::val::v1::Datapoint::kUint32:
      oss << dp.uint32();
      break;
    case kuksa::val::v1::Datapoint::kUint64:
      oss << dp.uint64();
      break;
    case kuksa::val::v1::Datapoint::kFloat:
      oss << dp.float_();
      break;
    case kuksa::val::v1::Datapoint::kDouble:
      oss << dp.double_();
      break;
    default:
      oss << "unknown or unset";
      break;
  }
  return oss.str();
}

void DataBrokerClient::setValueImpl(kuksa::val::v1::Datapoint *dp, float value) {
  dp->set_float_(value);
}

void DataBrokerClient::setValueImpl(kuksa::val::v1::Datapoint *dp, double value) {
  dp->set_double_(value);
}

void DataBrokerClient::setValueImpl(kuksa::val::v1::Datapoint *dp, int32_t value) {
  dp->set_int32(value);
}

void DataBrokerClient::setValueImpl(kuksa::val::v1::Datapoint *dp, int64_t value) {
  dp->set_int64(value);
}

void DataBrokerClient::setValueImpl(kuksa::val::v1::Datapoint *dp, uint32_t value) {
  dp->set_uint32(value);
}

void DataBrokerClient::setValueImpl(kuksa::val::v1::Datapoint *dp, uint64_t value) {
  dp->set_uint64(value);
}

void DataBrokerClient::setValueImpl(kuksa::val::v1::Datapoint *dp, bool value) {
  dp->set_bool_(value);
}

void DataBrokerClient::setValueImpl(kuksa::val::v1::Datapoint *dp, const std::string &value) {
  dp->set_string(value);
}

//------------------------------------------------------------------------------
// SubscriptionManager implementation
//------------------------------------------------------------------------------
SubscriptionManager::SubscriptionManager(DataBrokerClient &client,
                                         const std::vector<std::string> &paths)
  : client_(client), signalPaths_(paths) {}

void SubscriptionManager::startSubscriptions() {
  for (const auto &path : signalPaths_) {
    threads_.emplace_back([this, path]() {
      client_.Subscribe(path);
    });
  }
}

void SubscriptionManager::joinAll() {
  for (auto &t : threads_) {
    if (t.joinable())
      t.join();
  }
}

void SubscriptionManager::detachAll() {
  for (auto &t : threads_) {
    if (t.joinable())
      t.detach();
  }
}

} // namespace KuksaClient
