#include "IOPic.h"

IOPic::IOPic() {
	cpu_interrupted = NULL;
	for(int i = 0; i < IO_DEVICES; ++i) _ir[i] = 0;
}
IOPic::~IOPic() {
	return;
}
mem_row_t IOPic::getRegister(io_addr_t addr) {
	uint8_t tr_index = (addr & 0x000F);
	return _tr[tr_index];
}

void IOPic::setRegister(io_addr_t addr,mem_row_t val) {
	uint8_t tr_index = (addr & 0x000F);
	_tr[tr_index] = val;	
}

void IOPic::plug(IOInterruptDeviceFE* dev,uint8_t ir_wire) {
	dev->setInterruptWire(&_ir[ir_wire]);
}

void IOPic::setInterruptWire(bool* b) {
	cpu_interrupted = b;
}