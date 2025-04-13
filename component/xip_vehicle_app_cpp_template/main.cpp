#include "KuksaClient.hpp"

#include <iostream>
#include <chrono>
#include <thread>

using namespace KuksaClient;

int main(int argc, char **argv) {
  // Parse configuration from "config.json"
  Config config;
  if (!ConfigParser::parse("/usr/local/bin/config.json", config)) {
    std::cerr << "Failed to load configuration. Exiting." << std::endl;
    return 1;
  }

  // Create the client and connect.
  std::cout << "Starting Kuksa Databroker Client (v1)..." << std::endl;
  DataBrokerClient client;
  client.Connect(config.serverURI);

  if (config.debug) {
    std::cout << "Debug mode enabled." << std::endl;
  }
  client.GetServerInfo();

  // Sample usage: get, set and get server info.
  client.GetTargetValue("Vehicle.Speed");
  client.GetCurrentValue("Vehicle.Speed");
  client.SetCurrentValue("Vehicle.Speed", 88.5f);
  client.SetTargetValue("Vehicle.Speed", 88.5f);
  // client.SetValue("Vehicle.Status", std::string("Active"));
  // client.SetValue("Vehicle.Enabled", true);

  // If there are signal paths, start subscriptions using the SubscriptionManager.
  if (!config.signalPaths.empty()) {
      // Define a user callback that will be called for each update.
      auto userCallback = [](const std::string &updatePath, const std::string &updateValue) {
        std::cout << "[User Callback] Update for " << updatePath 
                  << ": " << updateValue << std::endl;
    };
    SubscriptionManager subManager(client, config.signalPaths);
    // Start subscriptions with the user callback.
    subManager.startSubscriptions(userCallback);

    // Let subscriptions run for 30 seconds (adjust as needed).
    std::this_thread::sleep_for(std::chrono::seconds(30));

    // In a real application, you might signal threads to stop
    // Here we simply detach them.
    subManager.detachAll();
  } else {
    std::cout << "No signals found in configuration." << std::endl;
  }

  std::cout << "Exiting client." << std::endl;
  return 0;
}
