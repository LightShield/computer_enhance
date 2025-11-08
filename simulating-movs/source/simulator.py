from registers import Registers
from logger import Logger

class Simulator:
    def __init__(self):
        self.regs = Registers()
        self.log = Logger.get()  # singleton logger

    def _run_simulation_step(self, command_to_run: str):
        command_parts = command_to_run.split()
        if command_parts[0] == "mov":
            dest = command_parts[1].rstrip(',')
            src = command_parts[2]

            dest_reg = getattr(self.regs, dest)
            pre_command_value = dest_reg.value

            if hasattr(self.regs, src):
                src_reg = getattr(self.regs, src)
                dest_reg.value = src_reg.value  # copy value, not object
            else:
                immediate_value = int(src, 16)
                dest_reg.value = immediate_value  # assign immediate value

            post_command_value = dest_reg.value
            self.log.info(f'Executed:"{command_to_run}" |||| {dest} 0x{pre_command_value:04X} -> 0x{post_command_value:04X}')
        else:
            self.log.error(f"Unsupported command: {command_to_run}")
            raise ValueError(f"Unsupported command: {command_to_run}")

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


if __name__ == "__main__":
    import sys
    simulator = Simulator()
    simulator.log.set_level("Info")

    if len(sys.argv) < 2:
        Logger.get().error("Please provide a file path with commands to simulate.")
        sys.exit(1)
    file_path = sys.argv[1]
    simulator.run_simulation(file_path)
