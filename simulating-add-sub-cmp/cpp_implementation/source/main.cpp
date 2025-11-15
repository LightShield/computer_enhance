#include <stdexcept>
#include "configs_loader.h"
#include "logger.h"
#include "simulator.h"

int main(int argc, char* argv[]) {
    ConfigsLoader configs(argv[0]);

    ConfigsLoader::PositionalConfig input_config = {
        .name = "input_file",
        .description = "Path to assembly file to simulate",
        .required = true
    };
    configs.add_positional(input_config);

    ConfigsLoader::FlagConfig verbosity_config = {
        .short_flag = "-v",
        .long_flag = "--verbosity",
        .description = "Set log verbosity level",
        .value_name = "level"
    };
    configs.add_flag(verbosity_config);

    if (!configs.parse_and_validate(argc, argv)) {
        Logger::Config error_config;
        error_config.print_metadata = false;
        Logger::Init(error_config);
        LOGGER.Error("{}", configs.get_error());
        configs.print_usage();
        return 1;
    }

    if (configs.has("help")) {
        configs.print_usage();
        return 0;
    }

    std::string input_file = configs.get_positional(0);

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
