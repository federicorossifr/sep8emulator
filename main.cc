#include <iostream>
#include "io/devices/DummyFE.h"
#include "io/devices/HD/HdFE.h"
#include "io/devices/HD/HdFileBE.h"
#include <vector>
#include <string>
#include <fstream>



Memory mem;

char * readAllBytes(const char * filename, int * read) {
    ifstream ifs(filename, ios::binary|ios::ate);
    ifstream::pos_type pos = ifs.tellg();
    int length = pos;
    char *pChars = new char[length];
    ifs.seekg(0, ios::beg);
    ifs.read(pChars, length);
    ifs.close();
    *read = length;
    return pChars;
}

int main(int argc, char* argv[]) {
	if(argc < 3) {
		cout << "Usage: ./sep8 bios.bin hd.vmdk" << endl;
		cout << "Usage: ./sep8 bios.bin hd.vmdk logfile" << endl;
		exit(1);	
	}
	ofstream logfile;
	if(argc == 4) {
		logfile = ofstream(argv[3]); 	
		Logger::setOutputStream(&logfile);
	} else {
		Logger::setOutputStream(&cout);
	}
	Logger::setLevel(true);
	int biosSize;
	mem_row_t* bios = (mem_row_t*)readAllBytes(argv[1],&biosSize);
	cpu c(mem);
	IOModule i(&c);
	DummyBE* _dbe = new DummyBE;
	DummyFE* d = new DummyFE(_dbe);
	HdFileBE* _be = new HdFileBE(argv[2],4096);
	HdFE* hdd = new HdFE(_be,mem);
	c.plugIO(&i);
	if(!i.plug(d,0x10,3,0)) exit(5);
	if(!i.plug(hdd,0xF0,8,1)) exit(5);
	c.loadBios(&bios[0],biosSize);
	Logger::logMessage("[MACHINE] Starting");
	c.emulate();
	c.dump();
	delete[] bios;
	delete(d);
	delete(hdd);
}