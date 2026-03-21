#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "change_tracking.h"
#include "register_proxy.h"
#include "register_types.h"

class Registers {
public:
    Registers();

    Register16Proxy get16(const std::string& name);
    Register8Proxy get8(const std::string& name);

    bool is8(const std::string& name) const;
    bool is16(const std::string& name) const;

    std::string dump() const;

    void mark_register_change(const std::string& name, uint16_t old_value, uint16_t new_value);
    void mark_flag_change(const std::string& flag_name, bool old_value, bool new_value);
    ChangeSet get_last_changes();

    void capture_flags();
    void check_flag_changes();

    // Accessors for simulator convenience
    uint16_t get_ip() const { return m_ip.value; }
    void set_ip(uint16_t value) { m_ip.value = value; }
    
    // Flags is public for easier manipulation in commands, but we could make it private
    Flags m_flags;

private:
    std::unordered_map<std::string, Register16*> m_reg16_map;
    std::unordered_map<std::string, uint8_t*> m_reg8_map;

    Register16 m_ax, m_bx, m_cx, m_dx, m_si, m_di, m_bp, m_sp, m_ip;

    ChangeSet m_change_set;
    uint16_t m_captured_flags_value;
};
