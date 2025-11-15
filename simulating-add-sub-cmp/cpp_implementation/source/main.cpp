#include "logger.h"
#include "simulator.h"
#include <stdexcept>

int main() {
    // Initialize logger with Info level for cleaner output
    Logger::Config config;
    config.level = LogLevel::Info;
    Logger::Init(config);

    LOGGER.Info("=== Computer Enhance - 8086 Simulator ===");
    LOGGER.Info("Simulator with integrated zero-overhead logger");

    try {
        // Create simulator instance
        Simulator sim;

        LOGGER.Info("\n=== Testing Manual Commands ===");

        // Test MOV commands
        LOGGER.Info("Executing: mov ax, 10");
        sim.run_command("mov ax, 10");
        LOGGER.Info("Registers: {}", sim.get_registers().dump());

        LOGGER.Info("Executing: mov bx, 20");
        sim.run_command("mov bx, 20");
        LOGGER.Info("Registers: {}", sim.get_registers().dump());

        // Test ADD command
        LOGGER.Info("Executing: add ax, bx");
        sim.run_command("add ax, bx");
        LOGGER.Info("Registers: {}", sim.get_registers().dump());

        // Test SUB command
        LOGGER.Info("Executing: sub ax, 5");
        sim.run_command("sub ax, 5");
        LOGGER.Info("Registers: {}", sim.get_registers().dump());

        // Test CMP command
        LOGGER.Info("Executing: cmp ax, bx");
        sim.run_command("cmp ax, bx");
        LOGGER.Info("Registers: {}", sim.get_registers().dump());

        LOGGER.Info("\n=== Testing with Debug Level ===");
        LOGGER.SetLevel(LogLevel::Debug);

        LOGGER.Info("Executing: mov cx, 100");
        sim.run_command("mov cx, 100");

        LOGGER.Info("Executing: add cx, 50");
        sim.run_command("add cx, 50");

        LOGGER.Info("\n=== Simulator Test Completed Successfully ===");

    } catch (const std::exception& e) {
        LOGGER.Error("Simulator error: {}", e.what());
        return 1;
    }

    return 0;
}
