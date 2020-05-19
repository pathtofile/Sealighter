"""
Helper script to print an file outputted by Sealighter
"""
import argparse
import json


def main():
    parser = argparse.ArgumentParser("Print Sealighter output files")
    parser.add_argument("output_file", help="Path to output file")
    args = parser.parse_args()
    with open(args.output_file, "r") as f:
        text = f.read()

    for line in text.split("\n"):
        if line.strip() != "":
            j = json.loads(line)
            print(json.dumps(j, indent=2))

if __name__ == "__main__":
    main()
