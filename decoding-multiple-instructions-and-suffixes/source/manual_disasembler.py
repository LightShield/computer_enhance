
import argparse
from decode_enums import Opcode, Direction, Width, Mode, get_registers, RegistersModeNon11, GetStringFromRegistersModeNon11
from binary_stream import BinaryStream

def extract_bits(byte, msb, lsb):
    msb_mask = (1 << msb +1) -1
    extracted_bits = (byte & msb_mask) >> lsb
    print(f"bits after extraction: {((byte & msb_mask) >> lsb):b}")
    return extracted_bits


def decode_mov_reg2reg_no_displacement(reg, rm, direction, width):
    decoded_reg = get_registers(reg, Width(width))
    decoded_rm = get_registers(rm, Width(width))
    if (direction == Direction.SOURCE_IN_REG_FIELD.value):
        return f"MOV {decoded_rm.name}, {decoded_reg.name}"
    return  f"MOV {decoded_reg.name}, {decoded_rm.name}"

def decode_mov_reg2reg(binary_stream, instruction_byte_0):
    instruction_byte_1 = binary_stream.read_next_byte()
    direction = extract_bits(instruction_byte_0, 1, 1)  
    width = extract_bits(instruction_byte_0, 0, 0) 
     
    mode = extract_bits(instruction_byte_1, 7, 6)  
    reg = extract_bits(instruction_byte_1, 5, 3)
    rm = extract_bits(instruction_byte_1, 2, 0)

    print(f"Raw instruction bits: {instruction_byte_0:b} {instruction_byte_1:b}")
    print(f"Decoded opcode: {Opcode.MOV_REG2REG.value}, Direction: {direction}, Width: {width}, Mode: {mode}")
    print(f"Decoded MOV instruction: Direction={Direction(direction).name}, Width={Width(width).name}, Mode={Mode(mode).name}")

    if mode == Mode.Register_Mode_No_Displacement.value:
        return decode_mov_reg2reg_no_displacement(reg, rm, direction, width)
    
    decoded_reg = get_registers(reg, Width(width))
    decoded_rm = GetStringFromRegistersModeNon11(RegistersModeNon11(rm))

    if mode == Mode.Memory_Mode_No_Displacement.value:
        if rm == 0b110:  # Special case for BP register
            raise RuntimeError("MOV instruction with BP register in memory mode is not supported")
        return return_decoded_mov_reg2reg_direction(decoded_reg, f"[{decoded_rm}]", direction)
    
    displacement_low = binary_stream.read_next_byte()
    displacement_value = displacement_low
    if mode == Mode.Memory_Mode_16bit_Displacement.value:
        displacement_high = binary_stream.read_next_byte()
        displacement_value = (displacement_high << 8) | displacement_low
    
    if displacement_value != 0:
        decoded_rm = f"[{decoded_rm} + {displacement_value}]"
    else:
        decoded_rm = f"[{decoded_rm}]"

    print(f"Displacement mode detected: {Mode(mode).name}, Displacement value: {displacement_value}")
    return return_decoded_mov_reg2reg_direction(decoded_reg, decoded_rm, direction)

def return_decoded_mov_reg2reg_direction(decoded_reg, decoded_rm, direction):
    if direction == Direction.SOURCE_IN_REG_FIELD.value:
        return f"MOV {decoded_rm}, {decoded_reg.name}"
    return f"MOV {decoded_reg.name}, {decoded_rm}"

def decode_mov_immediate2reg(instruction_byte_0, binary_stream):
    reg = extract_bits(instruction_byte_0, 2, 0)
    width = extract_bits(instruction_byte_0, 3, 3)
    immediate_value = binary_stream.read_next_byte()

    if width == Width.WORD.value:
        immediate_high = binary_stream.read_next_byte()
        immediate_value |= (immediate_high << 8) 

    decoded_reg = get_registers(reg, Width(width))
    return f"MOV {decoded_reg.name}, {immediate_value}"

def decode_instruction(binary_stream, instruction_byte_0):
    opcode = extract_bits(instruction_byte_0, 7, 2)
    if Opcode.is_mov_immediate2reg(opcode):
        print("MOV immediate to register instruction detected")
        return decode_mov_immediate2reg(instruction_byte_0, binary_stream)
        
    match opcode:
        case Opcode.MOV_REG2REG.value:
            return decode_mov_reg2reg(binary_stream, instruction_byte_0)
        case _:
            raise RuntimeError(f"Unhandled opcode: {opcode:06b}")   
   

def main():
    parser = argparse.ArgumentParser(description="Manual disassembler for 8086 instructions.")
    parser.add_argument("filename", type=str, help="The binary file to disassemble.")
    args = parser.parse_args()

    binary_stream = BinaryStream(args.filename)
    with open("decoded_instructions.txt", "a") as f:
        while True:
            try:
                next_byte = binary_stream.read_next_byte()
                if next_byte is None:
                    print("No more bytes to read.")
                    break
                decoded_instruction = decode_instruction(binary_stream, next_byte)
                print("decoded instruction: ", decoded_instruction)
                f.write(decoded_instruction + "\n")
            except StopIteration:
                print("End of file reached - No more bytes to read.")
                break
            except RuntimeError as e:
                print(f"Error decoding instruction: {e}")
                break

if __name__ == "__main__":
    main()