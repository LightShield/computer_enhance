from registers import Registers
from logger import Logger

class Simulator:
    def __init__(self):
        self.regs = Registers()
        self.log = Logger.get()  # singleton logger
        self.log.set_level("INFO")

    def _run_simulation_step(self, command_to_run: str):
        command_parts = command_to_run.split()
        if command_parts[0] == "mov":
            dest = command_parts[1].rstrip(',')
            src = command_parts[2]
            pre_command_value = getattr(self.regs, dest)
            
            self.log.debug(f'dest: {dest}, src: {src}')

            # Handle register to register move
            if hasattr(self.regs, src):
                setattr(self.regs, dest, getattr(self.regs, src).value)
            # Handle immediate to register move
            else:
                try:
                    immediate_value = int(src, 16)
                except ValueError:
                    self.log.error(f"Invalid immediate value: {src}")
                    raise
                setattr(self.regs, dest, immediate_value)

            post_command_value = getattr(self.regs, dest)
            self.log.info(f"Executed:\"{command_to_run}\" |||| {dest} 0x{pre_command_value.value:04X} -> 0x{post_command_value:04X}")
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
    if len(sys.argv) < 2:
        Logger.get().error("Please provide a file path with commands to simulate.")
        sys.exit(1)
    file_path = sys.argv[1]
    simulator.run_simulation(file_path)
