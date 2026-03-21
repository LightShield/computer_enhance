import argparse
import sys
import os

# Add current directory to path so it can find decode_enums etc
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from decoder import decode_instruction
from binary_stream import BinaryStream

def main():
    parser = argparse.ArgumentParser(description="8086 Decoder CLI")
    parser.add_argument("filename", help="Binary file to decode")
    args = parser.parse_args()

    stream = BinaryStream(args.filename)

    while True:
        try:
            byte = stream.read_next_byte()
            if byte is None:
                break
            instr = decode_instruction(stream, byte)
            if instr:
                print(instr)
        except EOFError:
            break
        except Exception as e:
            print(f"Error: {e}")
            break

if __name__ == "__main__":
    main()
