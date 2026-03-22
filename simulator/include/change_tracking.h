#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace lightshield {

struct RegisterChange {
    std::string name;
    uint16_t old_value;
    uint16_t new_value;
    uint8_t m_reserved[4] = {0}; // alignment
};

struct FlagChange {
    std::string flag_name;
    bool old_value;
    bool new_value;
    uint8_t m_reserved[6] = {0}; // alignment
};

struct ChangeSet {
    std::vector<RegisterChange> register_changes;
    std::vector<FlagChange> flags_changes;

    bool has_changes() const {
        return !register_changes.empty() || !flags_changes.empty();
    }

    void clear() {
        register_changes.clear();
        flags_changes.clear();
    }
};

} // namespace lightshield
