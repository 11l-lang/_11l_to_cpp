# [https://github.com/weewStack/Python-projects/blob/master/000-BMP-Converter/n5110_BMP_converter.py <- https://github.com/weewStack/Python-projects <- https://youtu.be/0Kwqdkhgbfw <- https://stackoverflow.com/questions/10439104/reading-bmp-files-in-python <- google:‘python bmp’]
import sys  # To get the data from the argument
import struct  # To unpack the data from the header
from typing import List


if __name__ == '__main__':
    #  1- Getting the file name
    file_name = sys.argv[1]
    file_name = file_name.split(".")[0]


    #  2- Opening & preparation to read the data the  file
    bmp = open(file_name + '.bmp', 'rb')

    # Getting the offset postion 10 -> 4 reads
    bmp.seek(10, 0)
    offset = struct.unpack('I', bmp.read(4))[0]

    # Get the height & width  : postion 18,22 -> 4 reads
    bmp.seek(18, 0)
    bmp_w = struct.unpack('I', bmp.read(4))[0]
    bmp_h = struct.unpack('I', bmp.read(4))[0]

    print(bmp_h, bmp_w)

    # Get the size  : postion 34 -> 4 reads
    bmp.seek(34, 0)
    bmp_s = struct.unpack('I', bmp.read(4))[0]


    # Getting the number of bytes in a row
    bmp_b = int(bmp_s/bmp_h)
    print(bmp_h, bmp_w, bmp_s, bmp_b)
    # 3-  Reading Data from the Picture
    bmp.seek(offset, 0)

    bmp_line = ''
    bmp_list : List[str] = []
    bmp_list_v : List[str] = []

    for line in range(bmp_h):
        for byte in range(bmp_b):
            bmp_byte = bmp.read(1)
            bmp_line += bin(255-bmp_byte[0])[2:].zfill(8)
        bmp_list.append(bmp_line[:bmp_w])
        bmp_list_v.append(bmp_line[:bmp_w].replace("0", " "))
        bmp_line = ''
    bmp_list_v.reverse()
    for line in bmp_list_v:
        print(line)

    # 4- Reshape the data to adjust to n5110
    byte_word = ""
    n5110_line : List[str] = []
    n5110_array : List[List[str]] = []

    for line in range(0, bmp_h, 8):
        for bit_num in range(bmp_w):
            for bit in range(line, line + 8):
                if bit > bmp_h - 1:
                    byte_word += "0"
                else:
                    byte_word += bmp_list[bit][bit_num]
            n5110_line.append('0x' + hex(int(byte_word, 2))[2:].upper().lower())
            byte_word = ''
        n5110_array.append(n5110_line)
        n5110_line = []

    n5110_array.reverse()
    # 5- Save the new array in a text file

    text_file = open(file_name + '.txt', 'w', newline = "\n")
    text_file.write(
        'static unsigned short ' + file_name + '_rows = ' + str(len(n5110_array)) + ';\n'
    )
    text_file.write(
        'static unsigned short ' + file_name + '_cols = ' + str(len(n5110_array[0])) + ';\n'
    )
    text_file.write('static unsigned char ' + file_name + '[] =\n')
    text_file.write('{\n')
    for l_cnt, lines in enumerate(n5110_array):
        for cnt, hexa in enumerate(lines):
            text_file.write(hexa)
            if cnt < len(lines)-1:
                text_file.write(',')
        if l_cnt < len(n5110_array)-1:
            text_file.write(',\n')
        else:
            text_file.write('\n')
    text_file.write('};')
