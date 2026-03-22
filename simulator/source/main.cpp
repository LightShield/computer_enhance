#include <iostream>
#include <stdexcept>
#include <tuple>
#include "configs_loader.hpp"
#include "logger.h"
#include "simulator.h"

using namespace lightshield::config;
using namespace lightshield;

struct SimulatorConfigs {
    Config<std::string> input_file{
        .default_value = "",
        .value = "",
        .verifier = [](const std::string& v) { return !v.empty(); },
        .flags = {"--input", "-i"},
        .description = "Path to assembly file to simulate",
        .required = true,
        .is_set = false
    };

    Config<std::string> verbosity{
        .default_value = "info",
        .value = "info",
        .verifier = [](const std::string&) { return true; },
        .flags = {"--verbosity", "-v"},
        .description = "Set log verbosity level",
        .required = false,
        .is_set = false
    };

    REGISTER_CONFIG_FIELDS(input_file, verbosity)
};

int main(int argc, char* argv[]) {
    ConfigsLoader<SimulatorConfigs> loader;

    if (loader.init(argc, argv) != 0) {
        return 1;
    }

    std::string input_file = loader.configs.input_file.value;

    Logger::Config logger_config;
    if (loader.configs.verbosity.is_set) {
        logger_config.level = Logger::ParseLogLevel(loader.configs.verbosity.value);
    }
    Logger::Init(logger_config);

    LOGGER.Info("=== Computer Enhance - 8086 Simulator ===");

    try {
        Simulator sim;
        sim.run_simulation(input_file);
        return 0;
    } catch (const std::exception& e) {
        LOGGER.Error("Simulator error: " + std::string(e.what()));
        return 1;
    }
}
