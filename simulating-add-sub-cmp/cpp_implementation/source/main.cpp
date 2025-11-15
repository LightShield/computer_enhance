#include <iostream>
#include <stdexcept>
#include <string>
#include "logger.h"
#include "simulator.h"

void print_usage(const char* program_name) {
    LOGGER.Info("Usage: {} [options] <input_file>", program_name);
    LOGGER.Info("Options:");
    LOGGER.Info("  -v, --verbosity <level>  Set log verbosity (debug, info, warn, error)");
    LOGGER.Info("                           Default: info");
    LOGGER.Info("\nExample:");
    LOGGER.Info("  {} ../resources/listing_0046_add_sub_cmp.txt", program_name);
    LOGGER.Info("  {} -v debug test.txt", program_name);
}

LogLevel parse_log_level(const std::string& level_str) {
    if (level_str == "debug") return LogLevel::Debug;
    if (level_str == "info") return LogLevel::Info;
    if (level_str == "warn") return LogLevel::Warn;
    if (level_str == "error") return LogLevel::Error;

    LOGGER.Warn("Unknown log level '{}', defaulting to Info", level_str);
    return LogLevel::Info;
}

int main(int argc, char* argv[]) {
    LogLevel log_level = LogLevel::Info;
    std::string input_file;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-v" || arg == "--verbosity") {
            if (i + 1 < argc) {
                log_level = parse_log_level(argv[++i]);
            } else {
                std::cerr << "Error: --verbosity requires an argument\n";
                return 1;
            }
        } else if (arg == "-h" || arg == "--help") {
            Logger::Config config;
            config.level = LogLevel::Info;
            Logger::Init(config);
            print_usage(argv[0]);
            return 0;
        } else {
            input_file = arg;
        }
    }

    Logger::Config config;
    config.level = log_level;
    Logger::Init(config);

    if (input_file.empty()) {
        LOGGER.Error("No input file specified");
        print_usage(argv[0]);
        return 1;
    }

    LOGGER.Info("=== Computer Enhance - 8086 Simulator ===");
    LOGGER.Debug("Log level: {}", static_cast<int>(log_level));

    try {
        Simulator sim;
        sim.run_simulation(input_file);
    } catch (const std::exception& e) {
        LOGGER.Error("Simulator error: {}", e.what());
        return 1;
    }

    return 0;
}
