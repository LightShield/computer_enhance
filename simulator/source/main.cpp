#include <stdexcept>
#include "configs_loader.h"
#include "logger.h"
#include "simulator.h"

struct SimulatorConfigs {
    Config<std::string> input_file{
        "input_file",
        nullptr,
        "--input",
        "Path to assembly file to simulate",
        true,
        ""
    };

    Config<std::string> verbosity{
        "verbosity",
        "-v",
        "--verbosity",
        "Set log verbosity level",
        false,
        "info"
    };

    auto get_all_configs() {
        return std::tie(input_file, verbosity);
    }

    auto get_all_configs() const {
        return std::tie(input_file, verbosity);
    }
};

int main(int argc, char* argv[]) {
    ConfigsLoader<SimulatorConfigs> configs(argv[0]);

    if (!configs.parse_and_validate(argc, argv)) {
        Logger::Config error_config;
        error_config.print_metadata = false;
        Logger::Init(error_config);
        if (!configs.get_error().empty()) {
            LOGGER.Error("{}", configs.get_error());
            configs.print_usage();
        }
        return 1;
    }

    std::string input_file = configs.input_file.value;

    Logger::Config logger_config;
    if (configs.verbosity.was_provided) {
        logger_config.level = Logger::ParseLogLevel(configs.verbosity.value);
    }
    Logger::Init(logger_config);

    LOGGER.Info("=== Computer Enhance - 8086 Simulator ===");

    try {
        Simulator sim;
        sim.run_simulation(input_file);
        return 0;
    } catch (const std::exception& e) {
        LOGGER.Error("Simulator error: {}", e.what());
        return 1;
    }
}
