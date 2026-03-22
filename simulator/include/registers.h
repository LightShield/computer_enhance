#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "change_tracking.h"
#include "register_proxy.h"
#include "register_types.h"

namespace lightshield {

/**
 * @brief 8086 Register file and state tracking.
 */
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

    uint16_t get_ip() const { return m_ip.value; }
    void set_ip(uint16_t value) { m_ip.value = value; }
    
    Flags& flags() { return m_flags; }
    const Flags& flags() const { return m_flags; }

private:
    // 8-byte aligned members
    std::unordered_map<std::string, Register16*> m_reg16_map;
    std::unordered_map<std::string, uint8_t*> m_reg8_map;
    ChangeSet m_change_set;

    // 16-bit and smaller members
    Register16 m_ax, m_bx, m_cx, m_dx, m_si, m_di, m_bp, m_sp, m_ip;
    Flags m_flags;
    uint16_t m_captured_flags_value;
    
    // Trailing padding to satisfy -Wpadded without pragmas
    uint16_t m_reserved0 = 0; 
};

} // namespace lightshield
