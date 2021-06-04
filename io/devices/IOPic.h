#include <iostream>
#ifndef TYPES_H
#include "../../types.h"
#define TYPES_H
#endif

#ifndef IODEVFE_H
#include "IODeviceFE.h"
#define IODEVFE_H
#endif

using namespace std;


class IOPic: public IOInterruptDeviceFE
{
	mem_row_t _tr[IO_DEVICES];
	bool _ir[IO_DEVICES];
	bool* cpu_interrupted;
	int nextIntIndex() {
		for(int i = 0; i < IO_DEVICES; ++i) {
			if(_ir[i]) {
				return i;
			}
		}
		return -1;
	}
public:
	IOPic();
	~IOPic();
	mem_row_t getRegister(io_addr_t addr) override;
	void setRegister(io_addr_t addr,mem_row_t val) override;
	void setInterruptWire(bool* b) override;
	void plug(IOInterruptDeviceFE* dev,uint8_t _ir);
	void sendInt() {
		if(nextIntIndex() >= 0)
			*cpu_interrupted = true;
	};
	int_type_t getIntType() { int ind = nextIntIndex(); _ir[ind] = 0; return _tr[ind];}
};
