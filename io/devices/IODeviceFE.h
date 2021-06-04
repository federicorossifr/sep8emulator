#include <iostream>
#ifndef TYPES_H
#include "../../types.h"
#define TYPES_H
#endif
using namespace std;

class IOModule;
class IODeviceFE
{
	public:
		IOModule* _io;
		virtual mem_row_t getRegister(io_addr_t addr) = 0;
		virtual void	  setRegister(io_addr_t addr,mem_row_t val) = 0;
};

class IOInterruptDeviceFE: public IODeviceFE {
public:
	virtual ~IOInterruptDeviceFE() {;}
	virtual void setInterruptWire(bool* b)=0;
};
class IODMADeviceFE: public IOInterruptDeviceFE{
protected:
	Memory& _memory;
public:
	IODMADeviceFE(Memory& m):_memory(m){};
};