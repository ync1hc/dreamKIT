#include "KuksaClient.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <functional>

// nlohmann/json (header-only)
#include <nlohmann/json.hpp>

// gRPC / Proto includes (only used here)
#include <grpcpp/grpcpp.h>
#include "kuksa/val/v1/val.grpc.pb.h"
#include "kuksa/val/v1/types.pb.h"

namespace KuksaClient {

using json = nlohmann::json;

//------------------------------------------------------------------------------
// ConfigParser Implementation
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
        config.serverURI = j.at("broker").at("serverURI").get<std::string>();
        config.debug = j.value("debug", false);
        if (j.contains("signal") && j["signal"].is_array()) {
            for (const auto &item : j["signal"]) {
                if (item.contains("path"))
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
// Helper functions: setValueImpl overloads
//------------------------------------------------------------------------------
void setValueImpl(kuksa::val::v1::Datapoint *dp, float value) {
    dp->set_float_(value);
}

void setValueImpl(kuksa::val::v1::Datapoint *dp, double value) {
    dp->set_double_(value);
}

void setValueImpl(kuksa::val::v1::Datapoint *dp, int32_t value) {
    dp->set_int32(value);
}

void setValueImpl(kuksa::val::v1::Datapoint *dp, int64_t value) {
    dp->set_int64(value);
}

void setValueImpl(kuksa::val::v1::Datapoint *dp, bool value) {
    dp->set_bool_(value);
}

void setValueImpl(kuksa::val::v1::Datapoint *dp, const std::string &value) {
    dp->set_string(value);
}

//------------------------------------------------------------------------------
// DataBrokerClient Implementation Using PIMPL (Private Implementation)
//------------------------------------------------------------------------------
class DataBrokerClient::Impl {
public:
    Impl() = default;
    ~Impl() = default;

    void Connect(const std::string &serverURI) {
        channel_ = grpc::CreateChannel(serverURI, grpc::InsecureChannelCredentials());
        stub_ = kuksa::val::v1::VAL::NewStub(channel_);
        std::cout << "Connected to " << serverURI << std::endl;
    }

    // Consolidated helper for Get operations.
    void GetTargetValue(const std::string &entryPath) {
        const std::string label = "Target";
        auto view = kuksa::val::v1::VIEW_TARGET_VALUE;

        if (!stub_) {
            std::cerr << "Client not connected. Aborting " << label << " GetValue()." << std::endl;
            return;
        }
        kuksa::val::v1::GetRequest request;
        auto* entryReq = request.add_entries();
        entryReq->set_path(entryPath);
        entryReq->set_view(view);

        kuksa::val::v1::GetResponse response;
        grpc::ClientContext context;
        grpc::Status status = stub_->Get(&context, request, &response);
        if (!status.ok()) {
            std::cerr << label << " Get() RPC failed: " << status.error_message() << std::endl;
            return;
        }
        if (response.error().code() != 0) {
            std::cerr << label << " Get() global error: " << response.error().message() << std::endl;
            return;
        }
        if (response.entries_size() == 0) {
            std::cerr << label << " Get(): No entries returned for \"" << entryPath << "\"." << std::endl;
            return;
        }
        const auto &dataEntry = response.entries(0);
        std::cout << label << " GetValue(): Value for \"" << entryPath << "\": "
                  << DataPointToString(dataEntry.value()) << std::endl;
    }

    void GetCurrentValue(const std::string &entryPath) {
        const std::string label = "Current";
        auto view = kuksa::val::v1::VIEW_CURRENT_VALUE;

        if (!stub_) {
            std::cerr << "Client not connected. Aborting " << label << " GetValue()." << std::endl;
            return;
        }
        kuksa::val::v1::GetRequest request;
        auto* entryReq = request.add_entries();
        entryReq->set_path(entryPath);
        entryReq->set_view(view);

        kuksa::val::v1::GetResponse response;
        grpc::ClientContext context;
        grpc::Status status = stub_->Get(&context, request, &response);
        if (!status.ok()) {
            std::cerr << label << " Get() RPC failed: " << status.error_message() << std::endl;
            return;
        }
        if (response.error().code() != 0) {
            std::cerr << label << " Get() global error: " << response.error().message() << std::endl;
            return;
        }
        if (response.entries_size() == 0) {
            std::cerr << label << " Get(): No entries returned for \"" << entryPath << "\"." << std::endl;
            return;
        }
        const auto &dataEntry = response.entries(0);
        std::cout << label << " GetValue(): Value for \"" << entryPath << "\": "
                  << DataPointToString(dataEntry.value()) << std::endl;
    }

    std::string DataPointToString(const kuksa::val::v1::Datapoint &dp) {
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

    void StreamedUpdate(const std::string &entryPath, float newValue) {
        if (!stub_) {
            std::cerr << "Client not connected. Aborting StreamedUpdate()." << std::endl;
            return;
        }
        grpc::ClientContext context;
        auto stream = stub_->StreamedUpdate(&context);
        kuksa::val::v1::StreamedUpdateRequest request;
        auto* update = request.add_updates();
        auto* dataEntry = update->mutable_entry();
        dataEntry->set_path(entryPath);
        dataEntry->mutable_value()->set_float_(newValue);
        update->add_fields(kuksa::val::v1::FIELD_VALUE);

        if (!stream->Write(request)) {
            std::cerr << "StreamedUpdate: Failed to write request." << std::endl;
            return;
        }
        stream->WritesDone();

        kuksa::val::v1::StreamedUpdateResponse response;
        while (stream->Read(&response)) {
            if (response.error().code() != 0) {
                std::cerr << "StreamedUpdate response error: " << response.error().message() << std::endl;
            } else {
                std::cout << "StreamedUpdate: Received a response." << std::endl;
            }
        }
        grpc::Status status = stream->Finish();
        if (!status.ok()) {
            std::cerr << "StreamedUpdate RPC failed: " << status.error_message() << std::endl;
        } else {
            std::cout << "StreamedUpdate: Completed successfully." << std::endl;
        }
    }

    /*
     * New Subscribe overload:
     * Accepts a user callback.
     */
    void Subscribe(const std::string &entryPath,
                   std::function<void(const std::string&, const std::string&)> userCallback) {
        if (!stub_) {
            std::cerr << "Client not connected. Aborting Subscribe()." << std::endl;
            return;
        }

        kuksa::val::v1::SubscribeRequest request;
        auto* subEntry = request.add_entries();
        subEntry->set_path(entryPath);
        subEntry->set_view(kuksa::val::v1::VIEW_ALL);
        subEntry->add_fields(kuksa::val::v1::FIELD_VALUE);

        grpc::ClientContext context;
        auto reader = stub_->Subscribe(&context, request);
        std::cout << "Subscribe: Listening on \"" << entryPath << "\"." << std::endl;
        kuksa::val::v1::SubscribeResponse response;
        int updateCount = 0;
        while (reader->Read(&response)) {
            ++updateCount;
            std::cout << "Subscribe: Received update #" << updateCount 
                      << " for \"" << entryPath << "\"" << std::endl;
            for (int i = 0; i < response.updates_size(); ++i) {
                const auto &upd = response.updates(i);
                std::string updatePath = upd.entry().path();
                std::string updateValue = DataPointToString(upd.entry().value());
                std::cout << "   Update: Path: " << updatePath
                          << " Value: " << updateValue << std::endl;
                if (userCallback)
                    userCallback(updatePath, updateValue);
            }
        }
        grpc::Status status = reader->Finish();
        if (!status.ok()) {
            std::cerr << "Subscribe RPC failed: " << status.error_message() << std::endl;
        } else {
            std::cout << "Subscribe: Stream finished for \"" << entryPath << "\"." << std::endl;
        }
    }

    void GetServerInfo() {
        if (!stub_) {
            std::cerr << "Client not connected. Aborting GetServerInfo()." << std::endl;
            return;
        }
        kuksa::val::v1::GetServerInfoRequest request;
        kuksa::val::v1::GetServerInfoResponse response;
        grpc::ClientContext context;
        grpc::Status status = stub_->GetServerInfo(&context, request, &response);
        if (!status.ok()) {
            std::cerr << "GetServerInfo RPC failed: " << status.error_message() << std::endl;
            return;
        }
        std::cout << "Server Info:" << std::endl;
        std::cout << "  Name:    " << response.name() << std::endl;
        std::cout << "  Version: " << response.version() << std::endl;
    }

    /*
     * Templated helper to perform a SetValue call.
     */
    template<typename T>
    void SetValueInternal(const std::string &entryPath, const T &newValue, auto setField) {
        if (!stub_) {
            std::cerr << "Client not connected. Aborting SetValue()." << std::endl;
            return;
        }
        kuksa::val::v1::SetRequest request;
        auto* update = request.add_updates();
        kuksa::val::v1::DataEntry* dataEntry = update->mutable_entry();
        dataEntry->set_path(entryPath);
        setValueImpl(dataEntry->mutable_value(), newValue);
        update->add_fields(setField);

        kuksa::val::v1::SetResponse response;
        grpc::ClientContext context;
        grpc::Status status = stub_->Set(&context, request, &response);
        if (!status.ok()) {
            std::cerr << "Set() RPC failed: " << status.error_message() << std::endl;
            return;
        }
        if (response.error().code() != 0) {
            std::cerr << "Set() global error: " << response.error().message() << std::endl;
        } else {
            std::cout << "SetValue(): Updated \"" << entryPath << "\" to " 
                      << DataPointToString(dataEntry->value()) << std::endl;
        }
    }

private:
    // gRPC members.
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<kuksa::val::v1::VAL::Stub> stub_;
};

//------------------------------------------------------------------------------
// Public API: Forward DataBrokerClient calls to the implementation.
//------------------------------------------------------------------------------
DataBrokerClient::DataBrokerClient() : pImpl(std::make_unique<Impl>()) { }
DataBrokerClient::~DataBrokerClient() = default;

void DataBrokerClient::Connect(const std::string &serverURI) {
    pImpl->Connect(serverURI);
}

void DataBrokerClient::GetTargetValue(const std::string &entryPath) {
    pImpl->GetTargetValue(entryPath);
}

void DataBrokerClient::GetCurrentValue(const std::string &entryPath) {
    pImpl->GetCurrentValue(entryPath);
}

void DataBrokerClient::StreamedUpdate(const std::string &entryPath, float newValue) {
    pImpl->StreamedUpdate(entryPath, newValue);
}

// New overload: Subscribe with a callback.
void DataBrokerClient::Subscribe(const std::string &entryPath,
    std::function<void(const std::string&, const std::string&)> userCallback) {
    pImpl->Subscribe(entryPath, userCallback);
}

void DataBrokerClient::GetServerInfo() {
    pImpl->GetServerInfo();
}

//------------------------------------------------------------------------------
// Public overloads for SetValue (non-templated overloads)
//------------------------------------------------------------------------------
void DataBrokerClient::SetCurrentValue(const std::string &entryPath, float newValue) {
    pImpl->SetValueInternal(entryPath, newValue, kuksa::val::v1::FIELD_VALUE);
}

void DataBrokerClient::SetCurrentValue(const std::string &entryPath, double newValue) {
    pImpl->SetValueInternal(entryPath, newValue, kuksa::val::v1::FIELD_VALUE);
}

void DataBrokerClient::SetCurrentValue(const std::string &entryPath, int32_t newValue) {
    pImpl->SetValueInternal(entryPath, newValue, kuksa::val::v1::FIELD_VALUE);
}

void DataBrokerClient::SetCurrentValue(const std::string &entryPath, int64_t newValue) {
    pImpl->SetValueInternal(entryPath, newValue, kuksa::val::v1::FIELD_VALUE);
}

void DataBrokerClient::SetCurrentValue(const std::string &entryPath, bool newValue) {
    pImpl->SetValueInternal(entryPath, newValue, kuksa::val::v1::FIELD_VALUE);
}

void DataBrokerClient::SetCurrentValue(const std::string &entryPath, const std::string &newValue) {
    pImpl->SetValueInternal(entryPath, newValue, kuksa::val::v1::FIELD_VALUE);
}

void DataBrokerClient::SetTargetValue(const std::string &entryPath, float newValue) {
    pImpl->SetValueInternal(entryPath, newValue, kuksa::val::v1::FIELD_ACTUATOR_TARGET);
}

void DataBrokerClient::SetTargetValue(const std::string &entryPath, double newValue) {
    pImpl->SetValueInternal(entryPath, newValue, kuksa::val::v1::FIELD_ACTUATOR_TARGET);
}

void DataBrokerClient::SetTargetValue(const std::string &entryPath, int32_t newValue) {
    pImpl->SetValueInternal(entryPath, newValue, kuksa::val::v1::FIELD_ACTUATOR_TARGET);
}

void DataBrokerClient::SetTargetValue(const std::string &entryPath, int64_t newValue) {
    pImpl->SetValueInternal(entryPath, newValue, kuksa::val::v1::FIELD_ACTUATOR_TARGET);
}

void DataBrokerClient::SetTargetValue(const std::string &entryPath, bool newValue) {
    pImpl->SetValueInternal(entryPath, newValue, kuksa::val::v1::FIELD_ACTUATOR_TARGET);
}

void DataBrokerClient::SetTargetValue(const std::string &entryPath, const std::string &newValue) {
    pImpl->SetValueInternal(entryPath, newValue, kuksa::val::v1::FIELD_ACTUATOR_TARGET);
}

//------------------------------------------------------------------------------
// SubscriptionManager Implementation Using PIMPL
//------------------------------------------------------------------------------
class SubscriptionManager::Impl {
public:
    Impl(DataBrokerClient &client, const std::vector<std::string> &paths)
      : client_(client), signalPaths_(paths) { }

    ~Impl() {
        for (auto &t : threads_) {
            if (t.joinable())
                t.join();
        }
    }

    /*
     * startSubscriptions() now receives a user-provided callback parameter.
     * This callback will be passed into each subscription so that the user's
     * function is called every time an update is received.
     *
     * The callback signature must match:
     *      void callback(const std::string &updatePath, const std::string &updateValue)
     */
    void startSubscriptions(std::function<void(const std::string&, const std::string&)> userCallback) {
        for (const auto &path : signalPaths_) {
            threads_.emplace_back([this, path, userCallback]() {
                // Call the client API's Subscribe with the user's callback.
                client_.Subscribe(path, userCallback);
            });
        }
    }

    void joinAll() {
        for (auto &t : threads_) {
            if (t.joinable())
                t.join();
        }
    }

    void detachAll() {
        for (auto &t : threads_) {
            if (t.joinable())
                t.detach();
        }
    }

private:
    DataBrokerClient &client_;
    std::vector<std::string> signalPaths_;
    std::vector<std::thread> threads_;
};

SubscriptionManager::SubscriptionManager(DataBrokerClient &client, const std::vector<std::string> &paths)
    : pImpl(std::make_unique<Impl>(client, paths)) { }
SubscriptionManager::~SubscriptionManager() = default;

/*
 * startSubscriptions now accepts a user-defined callback function.
 * The user's callback will be used by all subscription threads.
 */
void SubscriptionManager::startSubscriptions(std::function<void(const std::string&, const std::string&)> userCallback) {
    pImpl->startSubscriptions(userCallback);
}

void SubscriptionManager::joinAll() {
    pImpl->joinAll();
}

void SubscriptionManager::detachAll() {
    pImpl->detachAll();
}

} // namespace KuksaClient
