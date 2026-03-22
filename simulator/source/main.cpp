#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "logger.h"
#include "simulator.h"

using namespace lightshield;

int main(int argc, char* argv[]) {
    std::string input_file;
    
    // Very basic CLI parsing for now
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--input" || arg == "-i") && i + 1 < argc) {
            input_file = argv[++i];
        }
    }

    if (input_file.empty()) {
        std::cerr << "Usage: " << argv[0] << " --input <file>" << std::endl;
        return 1;
    }

    Logger::Config logger_config;
    logger_config.level = LogLevel::Info;
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
