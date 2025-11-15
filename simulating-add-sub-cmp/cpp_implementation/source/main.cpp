#include "logger.h"

int main() {
    // Initialize logger with Debug level for verbose output
    Logger::Config config;
    config.level = LogLevel::Debug;
    Logger::Init(config);

    // Get logger instance (zero overhead access)
    auto& logger = Logger::Get();

    // Sanity check - test all log levels
    logger.Info("=== Computer Enhance - Logger Integration Test ===");
    logger.Debug("Logger initialized successfully with Debug level");

    logger.Info("Project: computer_enhance/simulating-add-sub-cmp/cpp_implementation");
    logger.Debug("Using zero-overhead logger from shared/logger_cpp submodule");

    // Test different log levels
    logger.Error("Test ERROR level - this is red");
    logger.Warn("Test WARN level - this is yellow");
    logger.Info("Test INFO level - this is green");
    logger.Debug("Test DEBUG level - this is cyan");

    // Test format strings
    int test_value = 42;
    logger.Info("Format test: answer = {}", test_value);
    logger.Debug("Format test: multiple values = {}, {}, {}", 1, 2, 3);

    // Change verbosity
    logger.Info("\n=== Changing to Error level only ===");
    logger.SetLevel(LogLevel::Error);
    logger.Error("Only errors will show now");
    logger.Warn("This won't show");
    logger.Info("This won't show either");

    // Reset to Info
    logger.SetLevel(LogLevel::Info);
    logger.Info("\n=== Logger sanity check completed successfully ===");
    logger.Info("Logger submodule integration verified!");

    return 0;
}
