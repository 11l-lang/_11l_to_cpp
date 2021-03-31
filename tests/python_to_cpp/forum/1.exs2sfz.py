# [https://forum.11l-lang.org/threads/supported-modules.10/#post-20]
""" EXS24 to SFZ sample library metadata converter.

    Might work. Might awaken some forgotten trickster goddess and lure her to your computer. Enjoy.

    Copyright (c) 2013, vonred

    Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is
    hereby granted, provided that the above copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
    INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE
    FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
    LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
    ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. """


import sys
import os
import struct
from typing import List


instrument_data : bytes

def chunk_size(instrument_data, offset):
	return 84 + struct.unpack_from('<I', instrument_data, offset + 4)[0]

class EXSChunk:

	offset : int
	__size = 0

	def size(self):
		""" size is specified in bytes, at byte 4-8, and does not include common chunk elements;
			that is, does not include the first 84 bytes """

		if self.__size == 0:
			self.__size = chunk_size(instrument_data, self.offset)

		return self.__size

	def id(self):
		""" return the chunk's id number; n.b. that this does not seem to actually be used --
			the sequence of chunks of each type in the file is used instead """

		return struct.unpack_from('<I', instrument_data, self.offset + 8)[0]

	def name(self):
		""" the name of a chunk starts at byte 20, and has a max. length of 64 bytes;
			this is treated as a zero-terminated utf-8 string """

		return instrument_data[self.offset + 20:self.offset + 84].decode('utf-8').split('\x00')[0]

	def display(self):
		"""" display the raw chunk data in hexdump format """
		raise NotImplementedError() # [-`"0x{0:04X}: ".format(...)` and `"{0:02X}".format(...)` are not supported yet-]
'''
		for line in range(0, self.size(), 16):
			print("0x{0:04X}: ".format(line), end='')
			for i in range(0, 16, 4):
				for j in range(0, 4):
					index = line + i + j
					if index >= self.size():
						print("  ", end='')
					else:
						print("{0:02X}".format(instrument_data[self.offset + index]), end='')
				print(" ", end='')
			print(" ", end='')
			end = line + 16
			if end > self.size():
				end = self.size()
			for c in instrument_data[self.offset + line:self.offset + end]:
				if c > 13 and c < 128:
					print(chr(c), end='')
				else:
					print(" ", end='')
			print("")'''

EXSHeader_sig = 0x00000101

class EXSHeader(EXSChunk):

	def __init__(self, offset):
		self.offset = offset

		if not offset == 0:
			raise RuntimeError("Found header at location  other than beginning of file! offset is {}".format(offset))


class EXSZone(EXSChunk):

	def __init__(self, offset):
		self.offset = offset

	def rootnote(self):
		return struct.unpack_from('B', instrument_data, self.offset + 85)[0]

	def finetune(self):
		return struct.unpack_from('b', instrument_data, self.offset + 86)[0]

	def pan(self):
		return struct.unpack_from('b', instrument_data, self.offset + 87)[0]

	def volumeadjust(self):
		return struct.unpack_from('b', instrument_data, self.offset + 88)[0]

	def startnote(self):
		return struct.unpack_from('B', instrument_data, self.offset + 90)[0]
	def endnote(self):
		return struct.unpack_from('B', instrument_data, self.offset + 91)[0]

	def minvel(self):
		return struct.unpack_from('B', instrument_data, self.offset + 93)[0]
	def maxvel(self):
		return struct.unpack_from('B', instrument_data, self.offset + 94)[0]

	def samplestart(self):
		return struct.unpack_from('<i', instrument_data, self.offset + 96)[0]
	def sampleend(self):
		return struct.unpack_from('<i', instrument_data, self.offset + 100)[0]

	def loopstart(self):
		return struct.unpack_from('<i', instrument_data, self.offset + 104)[0]
	def loopend(self):
		return struct.unpack_from('<i', instrument_data, self.offset + 108)[0]

	def loop(self):
		return struct.unpack_from('B', instrument_data, self.offset + 117)[0]

	def pitchtrack(self):
		return not (struct.unpack_from('B', instrument_data, self.offset + 84)[0] & 1)
	def oneshot(self):
		return struct.unpack_from('B', instrument_data, self.offset + 84)[0] & 2

	def group(self):
		group = struct.unpack_from('<i', instrument_data, self.offset + 172)[0]
		if group >= 0:
			return group

		# FIXME: the group can be -1 -- just returning the last group for now
		## Because `self.instrument.groups` became inaccessible just return 0 [there is no difference in converted files anyway]
		return 0 # len(self.instrument.groups) - 1

	def sampleindex(self):
		return struct.unpack_from('<I', instrument_data, self.offset + 176)[0]


class EXSGroup(EXSChunk):

	def __init__(self, offset):
		self.offset = offset

	def polyphony(self):
		return struct.unpack_from('B', instrument_data, self.offset + 86)[0]

	def trigger(self):
		return struct.unpack_from('B', instrument_data, self.offset + 157)[0]

	def output(self):
		return struct.unpack_from('B', instrument_data, self.offset + 158)[0]
	def sequence(self):
		return struct.unpack_from('<i', instrument_data, self.offset + 164)[0]


class EXSSample(EXSChunk):

	def __init__(self, offset):
		self.offset = offset

	def length(self):
		return struct.unpack_from('<i', instrument_data, self.offset + 88)[0]

	def rate(self):
		return struct.unpack_from('<i', instrument_data, self.offset + 92)[0]

	def bitdepth(self):
		return struct.unpack_from('B', instrument_data, self.offset + 96)[0]


class EXSParam(EXSChunk):

	def __init__(self, offset):
		self.offset = offset


class EXSSamplePool:

	locations : List[str]

	def __init__(self):
		self.locations = []

	def locate(self, filename, search_depth=1):
		""" try to locate the directory containing a sample; if found,
			it will be added to the list of known locations which are searched first """

		# note that, as NTFS and HFS are not (usually) case-sensitive, neither are filename comparisons here

		last = ""
		def search_location(search):
			if search in self.locations:
				return ''

			path = os.path.realpath(os.path.abspath(os.path.join(self.base, search)))
			if path == last:
				return ''

			for name in os.listdir(path):
				location = os.path.join(path, name)
				if os.path.isfile(location):
					if name.lower() == filename:
						self.locations.append(search)
						return search
				elif os.path.isdir(location):
					location = search_location(os.path.join(search, name))
					if location != '':
						return location
			return ''

		filename = filename.lower()

		# first try if the file is located in a known location
		for location in self.locations:
			for name in os.listdir(os.path.join(self.base, location)):
				if name.lower() == filename:
					if os.path.isfile(os.path.join(self.base, location, filename)):
						return location

		# try to locate the file
		search = ""
		for _i in range(0, search_depth):
			location = search_location(search)
			if location != '':
				return location
			last = os.path.realpath(os.path.abspath(os.path.join(self.base, search)))
			search = os.path.join(search, "..")

		raise RuntimeError("Couldn't locate sample {0}!".format(filename))


class EXSSamplePoolDummy(EXSSamplePool):

	name : str

	def __init__(self, name=''):
		self.name = name

	def path(self, filename):
		return os.path.join(self.name, filename)


class EXSSamplePoolFixed(EXSSamplePool):

	base : str

	def __init__(self, path):
		if os.path.isdir(path):
			self.base = os.path.dirname(path)
		else:
			raise RuntimeError("{0}is not a valid path!".format(path))

		super().__init__()

	def path(self, filename):
		return os.path.join(self.base, self.locate(filename), filename)


class EXSSamplePoolLocator(EXSSamplePool):

	base : str

	def __init__(self, exsfile_name):
		self.base = os.path.dirname(exsfile_name)
		super().__init__()

	def path(self, filename):
		return os.path.join(self.locate(filename, 4), filename)


class EXSInstrument:

	pool : EXSSamplePool

	exsfile_name : str

	def __init__(self, exsfile_name, sample_location=''):

		self.zones : List[EXSZone] = []
		self.groups : List[EXSGroup] = []
		self.samples : List[EXSSample] = []

		self.exsfile_name = exsfile_name

		if os.path.getsize(exsfile_name) > 1024 * 1024:
			raise RuntimeError("EXS file is too large; will not parse! (size > 1 MebiByte)")

		global instrument_data
		instrument_data = open(exsfile_name, 'rb').read()

		# ensure this is a valid file we can parse
		if struct.unpack_from('>I', instrument_data, 0)[0] == EXSHeader_sig and instrument_data[16:20] == b'SOBT':
			raise RuntimeError("File is a big endian EXS file; cannot parse!")
		if not struct.unpack_from('<I', instrument_data, 0)[0] == EXSHeader_sig and instrument_data[16:20] == b'TBOS':
			raise RuntimeError("File is not an EXS file; will not parse!")

		# if isinstance(sample_location, EXSSamplePool):
		# 	self.pool = sample_location
		# elif sample_location is None:
		if sample_location == '':
			self.pool = EXSSamplePoolLocator(exsfile_name)
		else:
			self.pool = EXSSamplePoolFixed(sample_location)

		# parse EXS file
		offset = 0
		end = len(instrument_data)
		while offset < end:
			sig = struct.unpack_from('<I', instrument_data, offset)[0]

			if sig == EXSHeader_sig:
				EXSHeader(offset)
			elif sig == 0x01000101:
				self.zones.append(EXSZone(offset))
			elif sig == 0x02000101:
				self.groups.append(EXSGroup(offset))
			elif sig == 0x03000101:
				self.samples.append(EXSSample(offset))
			elif sig == 0x04000101:
				EXSParam(offset)
			else:
				raise RuntimeError("Encountered an unknown chunk signature! signature is " + hex(sig))

			offset += chunk_size(instrument_data, offset)

	def build_sequences(self):
		""" exs handles round robin samples by using groups that point to the next group, and so on
			until the sequence is reset by pointing to group -1;
			here we trace each of those chains for simple processing later """

		sequences : List[List[int]] = []

		for gid, group in enumerate(self.groups):
			if not group.sequence():
				continue

			for sequence in sequences:
				if group in sequence:
					break
			else:

				# trace back to the first group in the chain by looking for a group that points to this chain,
				# and repeating the process until we end up at a group that's not pointed to

				sequence : List[int] = []

				cont = True
				while cont == True:
					cont = False
					for gid2, g in enumerate(self.groups):
						if g.sequence() == gid and not gid2 == g.sequence() and not gid in sequence:
							sequence.append(gid)
							gid = gid2
							cont = True
							break

				# now that we're at the start of the chain, simply follow it to the end

				sequence = []
				while not gid == -1 and not gid in sequence:
					sequence.append(gid)
					gid = self.groups[gid].sequence()

				if len(sequence) > 1:
					sequences.append(sequence)

		return sequences

	def convert(self, sfzfilename, overwrite=False):

		sequences = self.build_sequences()

		def get_sequence_position(zone):
			for sequence in sequences:
				if zone.group() in sequence:
					return sequence.index(zone.group())
			return 0

		def get_rootnote(zone):
			if not zone.pitchtrack():
				if zone.rootnote() < zone.startnote() or zone.rootnote() > zone.endnote():
					return zone.startnote()
			return zone.rootnote()

		ranges : Dict[Tuple[int, int, int, int, int], EXSZone] = {}
		for zone in self.zones:
			key = (zone.startnote(), zone.endnote(), get_rootnote(zone), zone.pan(), get_sequence_position(zone))
			if not key in ranges:
				empty_list : List[EXSZone] = []
				ranges[key] = empty_list
			ranges[key].append(zone)

		key_sequence : Dict[Tuple[int, int, int, int, int], int] = {}
		for key in sorted(ranges.keys()):

			# to make round robin samples and choke groups work together,
			# take the first exs group in a sequence and use that for all members of the sequence

			keyrange = ranges[key]
			group = keyrange[0].group()

			for sequence in sequences:
				if group in sequence:
					group = sequence[0]
					break
			else:
				continue

			key_sequence[key] = group

		if not overwrite and os.path.isfile(sfzfilename):
			raise RuntimeError("file {0} already exists; will not overwrite!".format(sfzfilename))

		sfzfile = open(sfzfilename, 'wt')

		sfzfile.write("// this file was generated from {exsfile} using vonred's {name}. Trickster goddess incoming.\n\n".
				format(exsfile=os.path.basename(self.exsfile_name), name=os.path.basename('1.exs2sfz.py')))

		for key in sorted(ranges.keys()):
			keyrange = ranges[key]

			sfzfile.write("<group>")

			if (key[0] == key[1] == key[2]):
				# one key triggers a sample, use minimal parameters
				sfzfile.write(" key={key}".format(key=key[0]))
			else:
				# multiple keys triggering a sample, add extra parameters
				# studio one's presence gets confused when it encounters key=nn pitch_keytrack=1; be explicit
				sfzfile.write("lokey={startnote} hikey={endnote} pitch_keycenter={rootnote}".
						format(startnote=keyrange[0].startnote(), endnote=keyrange[0].endnote(),
							   rootnote=keyrange[0].rootnote()))

			choke_group : int
			if key in key_sequence:
				choke_group = key_sequence[key]
			else:
				choke_group = keyrange[0].group()

			choke_voices : Dict[Tuple[int, int], int] = {}
			for zone in keyrange:
				if (zone.minvel(), zone.maxvel()) in choke_voices:
					choke_voices[(zone.minvel(), zone.maxvel())] += 1;
				else:
					choke_voices[(zone.minvel(), zone.maxvel())] = 1

			if self.groups[keyrange[0].group()].polyphony() == choke_voices[(keyrange[0].minvel(), keyrange[0].maxvel())]:
				# add a choke group for e.g. hihats in drum libraries

				if choke_group == 0:
					# group 0 can't be used as a choke group in sfz, so change it where needed
					choke_group = len(self.groups)

				sfzfile.write(" group={group} off_by={group} polyphony={polyphony} off_mode=fast".
						format(group=choke_group, polyphony=self.groups[keyrange[0].group()].polyphony()))

			if self.groups[keyrange[0].group()].output() > 0:
				sfzfile.write(" output={output}".format(output=self.groups[keyrange[0].group()].output()))

			if keyrange[0].pan():
				sfzfile.write(" pan={pan}".format(pan=keyrange[0].pan()))

			if keyrange[0].oneshot():
				sfzfile.write(" loop_mode=one_shot")

			for sequence in sequences:
				if keyrange[0].group() in sequence:
					sfzfile.write(" seq_length={seq_length} seq_position={seq_position}".
							format(seq_length=len(sequence), seq_position=sequence.index(keyrange[0].group()) + 1))
					break

			sfzfile.write(" pitch_keytrack={pitchtrack}\n".format(pitchtrack=int(keyrange[0].pitchtrack())))

			for zone in keyrange:
				sfzfile.write("\t<region> lovel={lovel:03} hivel={hivel:03} amp_velcurve_{hivel:03}=1".
						format(lovel=zone.minvel(), hivel=zone.maxvel()))

				if zone.finetune():
					sfzfile.write(" tune={finetune}".
							format(finetune=zone.finetune()))

				if zone.volumeadjust():
					sfzfile.write(" volume={volumeadjust}".
							format(volumeadjust=zone.volumeadjust()))

				if zone.samplestart():
					sfzfile.write(" offset={samplestart}".
							format(samplestart=zone.samplestart()))

				if zone.sampleend() and zone.sampleend() < self.samples[zone.sampleindex()].length():
					sfzfile.write(" end={sampleend}".
							format(sampleend=zone.sampleend()))

				if zone.loop():
					sfzfile.write(" loop_mode=loop_sustain loop_start={loopstart} loop_end={loopend}".
							format(loopstart=zone.loopstart(), loopend=zone.loopend() - 1))

				if self.groups[zone.group()].trigger() == 1:
					sfzfile.write(" trigger=release")

				sfzfile.write(" sample={sample}".
						format(sample=self.pool.path(self.samples[zone.sampleindex()].name())))

				sfzfile.write("\n")


if __name__ == '__main__':

	if len(sys.argv) < 3 or len(sys.argv) > 4:
		print("Usage: {0} EXSfile.exs SFZfile.sfz [samplefolder]".format('1.exs2sfz.py'))
		print()
		print("    the samplefolder argument is optional; if not specified, the program will")
		print("    attempt to locate the samples by searching folders surrounding the exs file")
		print()

		sys.exit(64)

	samplefolder = sys.argv[3] if len(sys.argv) == 4 else ''
	try:
		exs = EXSInstrument(sys.argv[1], samplefolder)
		exs.convert(sys.argv[2])
	except RuntimeError as rerr:
		sys.exit(rerr)
