#include <iostream>
#include <iomanip>
#ifndef CPU_H
#include "../cpu/cpu.h"
#endif

#ifndef IOPIC_H
#include "devices/IOPic.h"
#define IOPIC_H
#endif

#ifndef IODEVBE_H
#include "devices/IODeviceBE.h"
#define IODEVBE_H
#endif

#ifndef IODEVFE_H
#include "devices/IODeviceFE.h"
#define IODEVFE_H
#endif

#ifndef TYPES_H
#include "../types.h"
#define TYPES_H
#endif

#include <vector>


using namespace std;
struct IOMap
{
	IODeviceFE* dev;
	io_addr_t start;
	io_addr_t end;
	IOMap(IODeviceFE* d,io_addr_t s,io_addr_t e): dev(d), start(s), end(e){}
};

class IOModule {
	vector<IOMap*> _iomap;
	cpu* _cpu;
	IOPic* _pic;
	public:
		IOModule(cpu*);
		~IOModule() {
			for(auto i:_iomap) {
				delete(i);
			}
			delete(_pic);
		}
		IODeviceFE* find(io_addr_t address);
		bool plug(IODeviceFE* dev,io_addr_t start,int numreg);
		bool plug(IOInterruptDeviceFE* dev,io_addr_t start,int numreg,int ir_wire);
		void notifyPIC() {_pic->sendInt();}
};

