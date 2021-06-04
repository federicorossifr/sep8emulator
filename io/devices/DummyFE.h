#include <iostream>
#include "DummyBE.h"
#ifndef TYPES_H
#include "../../types.h"
#define TYPES_H
#endif

#ifndef IODEVFE_H
#include "IODeviceFE.h"
#define IODEVFE_H
#endif

#ifndef IO_H
#include "../io.h"
#define IO_H
#endif

using namespace std;
/*
*/
class DummyFE: public IOInterruptDeviceFE 
{
	enum {RCR,RBR,RSR};  //START+0x0,0x1,0x2
	const mem_row_t startCMD = 0xEA;
	mem_row_t registers[3] = {0x00,0x00,0x00};
	bool* _int_ref;
	DummyBE* _backend;
public:
	DummyFE(DummyBE* be): _backend(be){return;};
	virtual ~DummyFE() {delete(_backend);}
	mem_row_t getRegister(io_addr_t addr) override;
	void setRegister(io_addr_t addr,mem_row_t val) override;
	void setInterruptWire(bool* b) override;	
};