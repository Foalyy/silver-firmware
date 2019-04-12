#!/usr/bin/env python

import sys

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage : " + sys.argv[0] + " FILE...", file=sys.stderr)
        sys.exit(0)

    for i in range(len(sys.argv) - 1):
        filename = sys.argv[i + 1]
        with open(filename, "r") as file:
            magic = file.readline().strip()
            if magic != "P1":
                print("Invalid file magic number : '" + magic + "'", file=sys.stderr)
                sys.exit(0)
            dimensions = file.readline()
            while dimensions.startswith("#"):
                dimensions = file.readline()
            width = int(dimensions.split(" ")[0].strip())
            height = int(dimensions.split(" ")[1].strip())
            if height not in [5, 8, 16, 32, 64]:
                print("Only heights of 5, 8, 16, 32 or 64 are supported", file=sys.stderr)
                sys.exit(0)
            dataRaw = file.read()
            data = ""
            for c in dataRaw:
                if c == "0" or c == "1":
                    data += c
            if len(data) != width * height:
                print("Invalid data length " + len(data) + ", expected " + str(width) + "x" + str(height) + "=" + str(width*height), file=sys.stderr)
                sys.exit(0)
            #print("Width : " + str(width))
            #print("Height : " + str(height))
            #print("Data : " + data)
            print("const Font::Char" + str(height) + " ICON_" + filename.split(".")[0].upper() + " = {")
            print("    " + str(width) + ",")
            print("    " + str(height) + ",")
            print("    {")
            for x in range(width):
                print("        0b" + "".join([str(data[x + (height - y - 1) * width]) for y in range(height)]) + ",")
            print("    }")
            print("};")