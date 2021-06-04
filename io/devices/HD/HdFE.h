#include <iostream>
#ifndef HDBE_H
#include "HdBE.h"
#define HDBE_H
#endif
#ifndef TYPES_H
#include "../../types.h"
#define TYPES_H
#endif

#ifndef IODEVFE_H
#include "../IODeviceFE.h"
#define IODEVFE_H
#endif

#ifndef IO_H
#include "../../io.h"
#define IO_H
#endif

#define SECTOR_SIZE  512
#define ADDR_MASK  0x07
#define RW_MASK 0x01
#define INT_MASK 0x02
#define DMA_MASK 0x04

using namespace std;


class HdFE: public IODMADeviceFE
{		  //0xF0 	0xF1 	0xF2 	0xF3 	0xF4 	0xF5 	0xF6	0xF7
	enum {	iSN1,	iSN2,	iSN3,	iSN4,	iCMD,	iBR,	iRSR,	iDMAPTR};
	mem_row_t SN1=0,SN2=0,SN3=0,SN4=0,CMD=0,RSR=0;
	addr_t DMAPTR = 0;
	mem_row_t BR[SECTOR_SIZE];
	int next=0;
	int next_dma_ptr=0;
	HdBE* be;
	bool* _int_ref;
	addr_t fetchAddress(addr_t,bool);
public:
	HdFE(HdBE* be_,Memory& m):IODMADeviceFE(m),be(be_){}
	virtual mem_row_t getRegister(io_addr_t addr) override;
	virtual void setRegister(io_addr_t addr,mem_row_t val) override;
	virtual void setInterruptWire(bool* b) override;
	virtual ~HdFE() {delete(be);};
};
