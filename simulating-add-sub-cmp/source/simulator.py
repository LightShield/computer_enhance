from registers import Registers
from logger import Logger

class Simulator:
    def __init__(self):
        self.regs = Registers()
        self.log = Logger.get()  # singleton logger

    def _get_register_name(self, reg_obj):
        # Try common name attributes first
        name = getattr(reg_obj, 'name', None)
        if name:
            return name
        name = getattr(reg_obj, '_name', None)
        if name:
            return name
        # Search Registers fields for the object identity
        for attr in ['ax','bx','cx','dx','sp','bp','si','di','es','ss','ds']:
            try:
                if getattr(self.regs, attr) is reg_obj:
                    return attr.upper()
            except Exception:
                continue
        # Fallback to class name
        return reg_obj.__class__.__name__

    def run_simulation(self, file_path: str):
        # Helper functions for filtering lines
        def _simulation_ended(line: str) -> bool:
            return line.startswith('Final')

        def _is_valid_command(line: str) -> bool:
            return line.strip() != '' and not line.startswith(('---', 'Final', ' '))

        self.log.info(f"--- Simulation Started : {file_path} ---")
        with open(file_path, 'r') as f:
            for line in f:
                if _simulation_ended(line):
                    break
                if not _is_valid_command(line):
                    continue
                self.log.debug(f"Processing line: {line.strip()}")
                command = line.strip()
                self.log.debug(f"Running command: {command}")
                self._run_simulation_step(command)
                self.log.debug(f"After '{command}': {self.regs}")

        self.log.info("--- Simulation Ended ---")
        self.log.info(f'Regs Status:\n{str(self.regs)}')

    def reset_simulation(self):
        self.regs = Registers()

    def _run_simulation_step(self, command_to_run: str):
        command_parts = command_to_run.split()
        command_action_detail = ""
        if command_parts[0] == "mov":
            command_action_detail = self._handle_mov_command(command_parts)
        elif command_parts[0] == "add":
            command_action_detail = self._handle_add_command(command_parts)
        elif command_parts[0] == "sub":
            command_action_detail = self._handle_sub_command(command_parts)
        elif command_parts[0] == "cmp":
            command_action_detail = self._handle_cmp_command(command_parts)
        else:
            self.log.error(f"Unsupported command: {command_to_run}")
            raise ValueError(f"Unsupported command: {command_to_run}")
        self.log.info(f'Executed:"{command_to_run}" |||| caused :  {command_action_detail}')


    def _handle_regs_command(self, command_parts, action_to_apply):
        dest = command_parts[1].rstrip(',')
        src = command_parts[2]

        dest_reg = getattr(self.regs, dest)

        if hasattr(self.regs, src):
            src_reg = getattr(self.regs, src)
            return action_to_apply( dest_reg, src_reg )
        immediate_value = int(src, 0) # auto detect base if 0x or 0b
        return action_to_apply( dest_reg, immediate_value )
    
    def _handle_mov_command(self, command_parts):
            def reg_mov_action(dest_reg, value):
                pre_command_value = dest_reg.value
                dest_reg.value = value
                reg_name = self._get_register_name(dest_reg)
                return  f'{reg_name} {dest_reg} 0x{pre_command_value:04X} -> 0x{value:04X}'
            
            return self._handle_regs_command(command_parts, reg_mov_action)

    def _update_substraction_flags(self, subtraction_result):
        prev_flags_state = self.regs._flags
        if (subtraction_result < 0):
            self.regs._flags.sign_flag = True
        elif (subtraction_result == 0):
            self.regs._flags.zero_flag = True
        
        return  self.regs._flags.print_changed_flags_from_prev_state(prev_flags_state)
    
    def _handle_add_command(self, command_parts):
        pass
    def _handle_sub_command(self, command_parts):
        def reg_sub_action(dest_reg, value):
            pre_command_value = dest_reg.value
            subtraction_result = dest_reg - value
            flags_update = self._update_substraction_flags(subtraction_result)
            dest_reg -= value
            reg_name = self._get_register_name(dest_reg)
            return  f'{reg_name} {dest_reg} 0x{pre_command_value:04X} -> 0x{value:04X} {flags_update}'
        return self._handle_regs_command(command_parts, reg_sub_action)
        
    def _handle_cmp_command(self, command_parts):
        def reg_cmp_action(dest_reg, value):
            subtraction_result = dest_reg - value
            return self._update_substraction_flags(subtraction_result)
        
        return self._handle_regs_command(command_parts, reg_cmp_action)

if __name__ == "__main__":
    import sys
    simulator = Simulator()
    simulator.log.set_level("Info")

    if len(sys.argv) < 2:
        Logger.get().error("Please provide a file path with commands to simulate.")
        sys.exit(1)
    file_path = sys.argv[1]
    simulator.run_simulation(file_path)
