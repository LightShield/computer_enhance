#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct RegisterChange {
    std::string name;
    uint16_t old_value;
    uint16_t new_value;
};

struct FlagsChange {
    std::string flag_name;
    bool old_value;
    bool new_value;
};

struct ChangeSet {
    std::vector<RegisterChange> register_changes;
    std::vector<FlagsChange> flags_changes;

    bool has_changes() const {
        return !register_changes.empty() || !flags_changes.empty();
    }

    void clear() {
        register_changes.clear();
        flags_changes.clear();
    }
};
