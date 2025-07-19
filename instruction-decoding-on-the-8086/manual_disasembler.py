from enum import Enum

# read a binary file prvided as an argument
def read_binary_file(filename):
    try:
        with open(filename, 'rb') as file:
            return file.read()
    except IOError as e:
        raise RuntimeError(f"Could not open file: {filename}") from e


class Opcode(Enum):
    MOV = 0b100010

class Direction(Enum):
    SOURCE_IN_REG_FIELD = 0b0
    DESTINATION_IN_REG_FIELD = 0b1

class Width(Enum):
    BYTE = 0b0
    WORD = 0b1

class Mode(Enum):
    Memory_Mode_No_Displacement = 0b00 
    Memory_Mode_8bit_Displacement = 0b01
    Memory_Mode_16bit_Displacement = 0b10
    Register_Mode_No_Displacement = 0b11 # AKA Register to Register Mode

class RegistersWidthByte(Enum):
    AL = 0b000
    CL = 0b001
    DL = 0b010
    BL = 0b011
    AH = 0b100
    CH = 0b101
    DH = 0b110
    BH = 0b111

class RegistersWidthWord(Enum):
    AX = 0b000
    CX = 0b001
    DX = 0b010
    BX = 0b011
    SP = 0b100
    BP = 0b101
    SI = 0b110
    DI = 0b111

def get_registers(reg, width):
    if width == Width.BYTE:
        return RegistersWidthByte(reg)
    elif width == Width.WORD:
        return RegistersWidthWord(reg)
    else:
        raise ValueError("Invalid width for register extraction")

def extract_bits(byte, msb, lsb):
    msb_mask = (1 << msb +1) -1
    extracted_bits = (byte & msb_mask) >> lsb
    print(f"bits after extraction: {((byte & msb_mask) >> lsb):b}")
    return extracted_bits

def decode_instruction(instruction_byte_0, instruction_byte_1):
    opcode = extract_bits(instruction_byte_0, 7, 2) 
    direction = extract_bits(instruction_byte_0, 1, 1)  
    width = extract_bits(instruction_byte_0, 0, 0) 
     
    mode = extract_bits(instruction_byte_1, 7, 6)  
    reg = extract_bits(instruction_byte_1, 5, 3)
    rm = extract_bits(instruction_byte_1, 2, 0)

    print(f"Raw instruction bits: {instruction_byte_0:b} {instruction_byte_1:b}")
    print(f"Decoded opcode: {opcode:06b}, Direction: {direction}, Width: {width}, Mode: {mode}")

    if opcode != Opcode.MOV.value:
        raise RuntimeError(f"Unknown opcode: {opcode}")
    if mode != Mode.Register_Mode_No_Displacement.value:
        raise RuntimeError(f"Unhandled mode: {Mode(mode).name} for MOV instruction")
    
    print(f"Decoded MOV instruction: Direction={Direction(direction).name}, Width={Width(width).name}, Mode={Mode(mode).name}")
    decoded_reg = get_registers(reg, Width(width))
    decoded_rm = get_registers(rm, Width(width))
    if (direction == Direction.SOURCE_IN_REG_FIELD.value):
        return f"MOV {decoded_rm.name}, {decoded_reg.name}"
    return  f"MOV {decoded_reg.name}, {decoded_rm.name}"

def main():
    import argparse

    parser = argparse.ArgumentParser(description="Manual disassembler for 8086 instructions.")
    parser.add_argument("filename", type=str, help="The binary file to disassemble.")
    args = parser.parse_args()

    try:
        binary_data = read_binary_file(args.filename)
        print("Binary data read successfully.")
        print("Binary as bits:")
        print(' '.join(f"{byte:08b}" for byte in binary_data))
        for i in range(0, len(binary_data), 2):
            instruction = binary_data[i:i+2]
            if len(instruction) < 2:
                print("Incomplete instruction at the end of the file.")
                break

            decoded_instruction = decode_instruction(instruction[0], instruction[1])
            print("decoded instruction: ", decoded_instruction)
            with open("decoded_instructions.txt", "a") as f:
                f.write(decoded_instruction + "\n")
    except RuntimeError as e:
        print(e)

if __name__ == "__main__":
    main()