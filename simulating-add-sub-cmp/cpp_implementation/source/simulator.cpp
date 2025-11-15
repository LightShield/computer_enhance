// simulator.cpp
#include "simulator.h"
#include "commands.h"
#include "logger.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

static std::vector<std::string> split(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) tokens.push_back(token);
    return tokens;
}

Simulator::Simulator() : regs() {}

void Simulator::run_simulation(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file) {
        LOGGER.Error("Cannot open file: {}", filepath);
        throw std::runtime_error("Cannot open file: " + filepath);
    }

    LOGGER.Info("Starting simulation from file: {}", filepath);

    std::string line;
    int line_num = 0;
    while (std::getline(file, line)) {
        line_num++;

        if (line.find("Final") == 0) {
            LOGGER.Info("Found 'Final' marker at line {}, stopping simulation", line_num);
            break;
        }
        if (line.empty() || line[0] == '-' || std::isspace(line[0])) continue;

        LOGGER.Debug("Processing line {}: {}", line_num, line);

        try {
            std::string result = run_command(line);
            LOGGER.Info("Command result: {}", result);
            LOGGER.Info("Registers: {}", regs.dump());
        } catch (const std::exception& e) {
            LOGGER.Error("Error processing line {}: {}", line_num, e.what());
        }
    }

    LOGGER.Info("Simulation completed");
}

std::string Simulator::run_command(const std::string& line) {
    auto tokens = split(line);

    if (tokens.empty()) {
        LOGGER.Warn("Empty command line received");
        throw std::runtime_error("Empty command");
    }

    const std::string& cmd = tokens[0];
    uint32_t cmd_hash = hash_command(cmd.c_str());

    LOGGER.Debug("Looking up command: {} (hash: {})", cmd, cmd_hash);

    for (size_t i = 0; i < COMMANDS_TABLE_SIZE; ++i) {
        if (commands_table[i].hash == cmd_hash) {
            // Pass Registers and arguments excluding command itself
            std::vector<std::string> args(tokens.begin() + 1, tokens.end());
            LOGGER.Debug("Executing command '{}' with {} arguments", cmd, args.size());
            return commands_table[i].handler(regs, args);
        }
    }

    LOGGER.Error("Unknown command: {}", cmd);
    throw std::runtime_error("Unknown command: " + cmd);
}
