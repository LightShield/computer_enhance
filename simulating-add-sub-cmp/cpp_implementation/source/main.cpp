#include <stdexcept>
#include "configs_loader.h"
#include "logger.h"
#include "simulator.h"

int main(int argc, char* argv[]) {
    ConfigsLoader configs(argv[0]);
    configs.add_positional("input_file", "Path to assembly file to simulate");
    configs.add_flag("-v", "--verbosity", "Set log verbosity level", "level");

    if (!configs.parse(argc, argv)) {
        return 1;
    }

    if (configs.has("help")) {
        configs.print_usage();
        return 0;
    }

    std::string input_file = configs.get_positional(0);
    if (input_file.empty()) {
        Logger::Config error_config;
        error_config.print_metadata = false;
        Logger::Init(error_config);
        LOGGER.Error("No input file specified");
        configs.print_usage();
        return 1;
    }

    Logger::Config logger_config;
    if (configs.has("verbosity")) {
        logger_config.level = Logger::ParseLogLevel(configs.get("verbosity"));
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
