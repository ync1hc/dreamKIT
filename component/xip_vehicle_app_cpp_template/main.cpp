#include "KuksaClient.hpp"
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

  // Sample usage: get, set and get server info.
  client.GetValue("Vehicle.Speed");
  client.SetValue("Vehicle.Speed", 88.5f);
//   client.SetValue("Vehicle.Status", std::string("Active"));
//   client.SetValue("Vehicle.Enabled", true);
  client.GetServerInfo();

  // If there are signal paths, start subscriptions using the SubscriptionManager.
  if (!config.signalPaths.empty()) {
    SubscriptionManager subManager(client, config.signalPaths);
    subManager.startSubscriptions();

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
