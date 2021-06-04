CC=g++
CFLAGS= -Wall -g -lpthread  --std=c++11

cpu.o: cpu/cpu.h cpu/cpu.cc types.h
	$(CC) $(CFLAGS) -c cpu/cpu.cc -o cpu.o
IOPic.o: io/devices/IOPic.h io/devices/IODeviceBE.h io/devices/IODeviceFE.h
	$(CC) $(CFLAGS) -c io/devices/IOPic.cc -o IOPic.o
Dummy.o: io/devices/DummyBE.h io/devices/DummyFE.h io/devices/DummyFE.cc	
	$(CC) $(CFLAGS) -c io/devices/DummyFE.cc -o Dummy.o
HdFileBE.o: io/devices/HD/HdBE.h io/devices/HD/HdFileBE.h io/devices/HD/HdFileBE.cc	HdFE.o
	$(CC) $(CFLAGS) -c io/devices/HD/HdFileBE.cc -o HdFileBE.o
HdFE.o: io/devices/HD/HdFE.h io/devices/HD/HdFE.cc 
	$(CC) $(CFLAGS) -c io/devices/HD/HdFE.cc -o HdFE.o
io.o : io/*.h io/io.cc IOPic.o Dummy.o
	$(CC) $(CFLAGS) -c io/io.cc -o io.o
main.o: cpu/cpu.h main.cc #io/devices/VGA/VGA.h io/devices/VGA/VgaBE.h io/devices/VGA/VgaConsoleBE.h  
	$(CC) $(CFLAGS) -c main.cc -o main.o
Logger.o: util/Logger.h util/Logger.cc
	$(CC) $(CFLAGS) -c util/Logger.cc -o Logger.o
sep8: io.o cpu.o main.o IOPic.o Logger.o HdFE.o HdFileBE.o
	$(CC) $(CFLAGS) io.o IOPic.o Dummy.o cpu.o main.o Logger.o HdFE.o HdFileBE.o -o sep8


bios: assembler/arch/bios.as
	./assembler/assembler.py 0 assembler/arch/bios.as;

sys: assembler/arch/sys.as
	./assembler/assembler.py 2 assembler/arch/sys.as;

usr: assembler/usr.as
	./assembler/assembler.py 2 assembler/usr.as 0x200000;


software: bios sys usr
	cat assembler/arch/sys.bin assembler/usr.bin > mydisk.vmdk;
all: sep8 software


clean:
	rm *.o sep8 mydisk.vmdk assembler/usr.bin assembler/arch/bios.bin assembler/arch/sys.bin
