#!/usr/bin/env python
import sys
import io
import os
from binascii import unhexlify

if len(sys.argv) < 3:
	print("Usage: ./assembler.py align_sector source.as");
	print("Usage: ./assembler.py align_sector source.as reloc_addr");	
	sys.exit();
global SECTOR_SIZE
SECTOR_SIZE=512
global relocator;
relocator = False;
global start_address
start_address = None;
if len(sys.argv) == 4:
	relocator = True;
	start_address = int(sys.argv[3],16);
	print("Assembling module relocating address to: " + sys.argv[3])


instr_trad = {
	"HLT"		:"0x00",
	"NOP"		:"0x01",
	"MOVALAH"	:"0x02",
	"MOVAHAL"	:"0x03",
	"INCDP"	 	:"0x04",
	"SHLAL"		:"0x05",
	"SHRAL"	 	:"0x06",
	"NOTAL"	 	:"0x07",
	"SHLAH"	 	:"0x08",
	"SHRAH"	 	:"0x09",
	"NOTAH"	 	:"0x0A",
	"PUSHAL"	:"0x0B",
	"POPAL"		:"0x0C",
	"PUSHAH"	:"0x0D",
	"POPAH"	    :"0x0E",
	"PUSHDP"	:"0x0F",
	"POPDP"		:"0x10",
	"RET"		:"0x11",
	"IRET"		:"0x12",
	"EXCHSP"    :"0x15",	

	"INaddrAL"  :"0x20",
	"OUTALaddr" :"0x21",
	"MOVopDP"	:"0x22",
	"MOVopSP" 	:"0x23",
	"MOVaddrDP" :"0x24",
	"MOVDPaddr" :"0x25",
	"LIDTPaddr"	:"0x26",

	"MOV(DP)AL" :"0x40",
	"CMP(DP)AL" :"0x41",
	"ADD(DP)AL" :"0x42",
	"SUB(DP)AL" :"0x43",
	"AND(DP)AL" :"0x44",
	"OR(DP)AL" 	:"0x45",
	"MOV(DP)AH" :"0x46",
	"CMP(DP)AH" :"0x47",
	"ADD(DP)AH" :"0x48",
	"SUB(DP)AH" :"0x49",
	"AND(DP)AH" :"0x4A",
	"OR(DP)AH" 	:"0x4B",

	"MOVAL(DP)" :"0x60",
	"MOVAH(DP)" :"0x61",

	"MOVopAL" 	:"0x80",
	"CMPopAL" 	:"0x81",
	"ADDopAL" 	:"0x82",
	"SUBopAL" 	:"0x83",
	"ANDopAL" 	:"0x84",
	"ORopAL" 	:"0x85",
	"MOVopAH" 	:"0x86",
	"CMPopAH" 	:"0x87",
	"ADDopAH" 	:"0x88",
	"SUBopAH" 	:"0x89",
	"ANDopAH" 	:"0x8A",
	"ORopAH" 	:"0x8B",
	"INTop"		:"0x8C",

	"MOVaddrAL" :"0xA0",
	"CMPaddrAL" :"0xA1",
	"ADDaddrAL" :"0xA2",
	"SUBaddrAL" :"0xA3",
	"ANDaddrAL" :"0xA4",
	"ORaddrAL" 	:"0xA5",
	"MOVaddrAH" :"0xA6",
	"CMPaddrAH" :"0xA7",
	"ADDaddrAH" :"0xA8",
	"SUBaddrAH" :"0xA9",
	"ANDaddrAH" :"0xAA",
	"ORaddrAH" 	:"0xAB",

	"MOVALaddr"	:"0xC0",
	"MOVAHaddr" :"0xC1",

	"JMPaddr"	:"0xE0",
	"JEaddr"	:"0xE1",
	"JNEaddr"	:"0xE2",
	"JAaddr"	:"0xE3",
	"JAEaddr"	:"0xE4",
	"JBaddr"	:"0xE5",
	"JBEaddr"	:"0xE6",
	"JGaddr"	:"0xE7",
	"JGEaddr"	:"0xE8",
	"JLaddr"	:"0xE9",
	"JLEaddr"	:"0xEA",
	"JZaddr"	:"0xEB",
	"JNZaddr"	:"0xEC",
	"JCaddr"	:"0xED",
	"JNCaddr"	:"0xEE",
	"JOaddr"	:"0xEF",
	"JNOaddr"	:"0xF0",
	"JSaddr"	:"0xF1",
	"JNSaddr"	:"0xF2",
	"CALLaddr"	:"0xF3",
}
addressable_regs = [
	"AL",
	"AH",
	"SP",
	"DP",
	"(DP)",
	""
]

def splitBytes(strl): #WE ASSUME LITERLAS ARE ALWAYS GIVEN IN HEX
	output = [];
	strl = strl[2:]
	while(len(strl)%2):
		strl="0"+strl; #NORMALIZE LENGTH
	for i in range(len(strl),0,-2):
		output.append("0x"+strl[i-2:i]);
	return output;

def prepareOperand(operand):
	return splitBytes(operand[1:])

def assembly(lines):
	lines = [x.strip() for x in lines]
	out = [];
	for index,line in enumerate(lines):
		if line.startswith("#"):
			continue;

		if line.startswith("%") or line.startswith("&"):
			tmp = line[1:];
			tmp = tmp.split(",");
			newlines = [];
			print(tmp[0]);
			with open(os.path.dirname(os.path.realpath(__file__))+"/"+tmp[0]) as fd_n:
				newlines = fd_n.readlines(); 
			bin_instr = [];

			if line.startswith("%"):
				global relocator;
				global start_address;
				old_relocator = relocator;
				relocator = True;
				start_address = int(tmp[1],16);
				bin_instr = assembly(newlines);
				relocator = old_relocator;
			else:
				bin_instr = [x.strip() for x in newlines];
			tmp_instr = [];
			for i in bin_instr:
				tmp_instr.append("MOV $"+i+",AL");
				tmp_instr.append("MOV AL,"+tmp[1]);
				tmp[1] = hex(int(tmp[1],16)+1)
			lines[index+1:index+1] = tmp_instr;
			continue;
		encodedOp  ="";
		encodedSrc = [];
		encodedDst =[];
		instr = line.split();
		opcode = instr[0];
		if len(instr) == 1:
			out.extend([instr_trad[opcode]]);
			continue;
		operands = instr[1].split(",");

		if len(operands) < 2:
			operands.append("");
		src = operands[0];
		dst = operands[1];

		if src in addressable_regs:
			opcode+=src;
		elif src.startswith('$'):
			opcode+="op";
			encodedSrc = prepareOperand(src);
		else:
			if relocator and instr[0] != "IN" and instr[0] != "OUT":
				__tmp = int(src,16);
				__tmp+=start_address
				tmp=hex(__tmp);
				encodedSrc = splitBytes(tmp);
				print("Relocated " + src + " to " + tmp)
			else:
				tmp = hex(int(src,16));
				encodedSrc = splitBytes(src);
			opcode+="addr"


		if dst in addressable_regs:
			opcode+=dst;
		elif dst.startswith('$'):
			opcode+="op";
			encodedDst = prepareOperand(dst);
		else:
			if relocator and instr[0] != "IN" and instr[0] != "OUT":
				__dsttmp = int(dst,16);
				__dsttmp+=start_address;
				dsttmp=hex(__dsttmp);
				encodedDst = splitBytes(dsttmp);
				print("Relocated " + dst +" to " + dsttmp)
			else:
				tmp = hex(int(dst,16));
				encodedDst = splitBytes(dst);
			opcode+="addr"

		encodedOp=instr_trad[opcode];
		out.append(encodedOp);
		out.extend(encodedSrc);
		if len(encodedDst) > 0:
			out.extend(encodedDst);
	return out;

filename=sys.argv[2];
sectors = int(sys.argv[1]);
lines = [];
with open(filename) as fd:
	lines = fd.readlines();
out = assembly(lines);
outfilename = os.path.splitext(filename)[0];
outfile = io.open(outfilename+".bin",'wb');
while len(out) < 512*sectors: #align to sector
	out.append("0x00");
outfile.write(unhexlify(''.join(format(i[2:], '>02s') for i in out)))
outfile.close();
