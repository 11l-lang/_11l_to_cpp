#!/usr/bin/env python3

# RKM to WAV converter
# Aleksander Alekseev, 2023
# https://eax.me/rkm-to-wav/

import sys, os
from typing import BinaryIO
Byte = int

WAV_HEADER_SIZE = 44
SAMPLE_RATE = 44100
BAUD_RATE = 700
SAMPLES_PER_BIT = int(SAMPLE_RATE / BAUD_RATE)
#BIT_ZERO_WAVE = [] # initialized below
#BIT_ONE_WAVE = [] # initialized below

temp_arr = [255 - 32] * (SAMPLES_PER_BIT - SAMPLES_PER_BIT//2) + [32] * (SAMPLES_PER_BIT//2)

# tranfer from high to low
BIT_ZERO_WAVE = temp_arr[:]
temp_arr.reverse()
# tranfer from low to high
BIT_ONE_WAVE = temp_arr

# generated with https://fiiir.com/
FIR_WEIGHTS = [
    0.000577926080672738,
    0.000459465018502213,
    0.000314862245890514,
    0.000114058758552773,
    -0.000176232019410298,
    -0.000586368131836170,
    -0.001136970252974637,
    -0.001832200078313594,
    -0.002653914944386688,
    -0.003557457996743626,
    -0.004469734119871817,
    -0.005290040439872776,
    -0.005893881011277897,
    -0.006139718992073919,
    -0.005878331252147731,
    -0.004964157085603880,
    -0.003267801176075040,
    -0.000688684837013550,
    0.002833242904185398,
    0.007307810544085602,
    0.012687912264589127,
    0.018866545092452867,
    0.025678084518976477,
    0.032903957573255177,
    0.040282515704921813,
    0.047522557733791673,
    0.054319636536848272,
    0.060374030484608231,
    0.065409095148934615,
    0.069188648369429409,
    0.071532089395465556,
    0.072326107924878466,
    0.071532089395465556,
    0.069188648369429409,
    0.065409095148934615,
    0.060374030484608238,
    0.054319636536848279,
    0.047522557733791680,
    0.040282515704921813,
    0.032903957573255184,
    0.025678084518976480,
    0.018866545092452870,
    0.012687912264589122,
    0.007307810544085603,
    0.002833242904185400,
    -0.000688684837013550,
    -0.003267801176075043,
    -0.004964157085603879,
    -0.005878331252147732,
    -0.006139718992073923,
    -0.005893881011277897,
    -0.005290040439872781,
    -0.004469734119871819,
    -0.003557457996743629,
    -0.002653914944386688,
    -0.001832200078313594,
    -0.001136970252974637,
    -0.000586368131836170,
    -0.000176232019410298,
    0.000114058758552773,
    0.000314862245890515,
    0.000459465018502213,
    0.000577926080672738,
]
WINDOW_SIZE = len(FIR_WEIGHTS)
fir_window = [128 for i in range(0,WINDOW_SIZE)]

# without the filter Mikrosha has difficulties interpreting the data
def fir_low_pass_filter(arr):
    global fir_window, FIR_WEIGHTS
    filtered = [Byte(0)] * len(arr)
    for i in range(0, len(arr)):
        fir_window = [arr[i]] + fir_window[0:WINDOW_SIZE-1]
        filtered[i] = int(sum([fir_window[j]*FIR_WEIGHTS[j]
                            for j in range(0,WINDOW_SIZE)]))
    return filtered

# writes one encoded byte to the output WAV file
# returns the amount of bytes written
def wav_write_byte(f: BinaryIO, b):
    for i in range(0,8):
        data = BIT_ONE_WAVE if (b >> (7-i)) & 1 else BIT_ZERO_WAVE
        f.write(bytes(fir_low_pass_filter(data)))
    return SAMPLES_PER_BIT*8

_debug = False
def debug(msg):
    if _debug:
        print("DEBUG: " + msg)

if __name__ == '__main__':
    if len(sys.argv) < 2 or '-h' in sys.argv or '--help' in sys.argv:
        print('usage: 2.rkm2wav INPUT_FILE')
        sys.exit(1)

    fin = open(sys.argv[1], 'rb')
    fout = open(os.path.splitext(sys.argv[1])[0] + '.wav', 'wb')
    addr_start = int.from_bytes(fin.read(2), 'big')
    addr_end = int.from_bytes(fin.read(2), 'big')
    data_length = addr_end - addr_start + 1
    fin.seek(4 + data_length)
    checksum = int.from_bytes(fin.read(2), 'big')
    fin.seek(4)

    debug("start address: 0x{:04X}".format(addr_start))
    debug("end address: 0x{:04X}".format(addr_end))
    debug("checksum: 0x{:04X}".format(checksum))

    # reserve space for the WAV header
    fout.write(bytes([Byte(0)] * WAV_HEADER_SIZE))
    file_size = WAV_HEADER_SIZE

    # preamble. this sounds like an ~3 second long 700 Hz tone
    for i in range(0,256):
        file_size += wav_write_byte(fout, 0x00)

    # sync byte
    file_size += wav_write_byte(fout, 0xE6)

    # start address, big-endian
    for i in range(0,2):
        file_size += wav_write_byte(fout, (addr_start >> (1-i)*8) & 0xFF)

    # end address, big-endian
    for i in range(0,2):
        file_size += wav_write_byte(fout, (addr_end >> (1-i)*8) & 0xFF)

    # write the data
    for i in range(0, data_length):
        b = int.from_bytes(fin.read(1), 'big')
        file_size += wav_write_byte(fout, b)

    # checksum, big-endian
    for i in range(0,2):
        file_size += wav_write_byte(fout, (checksum >> (1-i)*8) & 0xFF)

    # some intermediate values in the end, for the FIR filter
    fout.write(bytes(fir_low_pass_filter([128 for i in range(0,WINDOW_SIZE)])))
    file_size += WINDOW_SIZE

    # write the WAV header
    fout.seek(0)
    fout.write(b"RIFF")
    temp = file_size - 8
    fout.write(temp.to_bytes(4, 'little'))
    fout.write(b"WAVEfmt ")
    temp = 16 # size of the rest of the WAV header, for PCM
    fout.write(temp.to_bytes(4, 'little'))
    temp = 1 # no compression, for PCM
    fout.write(temp.to_bytes(2, 'little'))
    temp = 1 # number of channels
    fout.write(temp.to_bytes(2, 'little'))
    temp = SAMPLE_RATE # sample rate
    fout.write(temp.to_bytes(4, 'little'))
    temp = SAMPLE_RATE*1 # bytes per one second
    fout.write(temp.to_bytes(4, 'little'))
    temp = 1 # bytes per sample, total by channels
    fout.write(temp.to_bytes(2, 'little'))
    temp = 8 # _bits_ per sample per one channel
    fout.write(temp.to_bytes(2, 'little'))
    fout.write(b"data")
    temp = file_size - WAV_HEADER_SIZE
    fout.write(temp.to_bytes(4, 'little'))

debug("done")