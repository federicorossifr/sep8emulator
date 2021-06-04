#include "io.h"

IOModule::IOModule(cpu* c) {
	_cpu = c;
	_pic = new IOPic();
	Logger::logMessage("[IOModule] PIC controller created");
	plug(_pic,0,16,-1);
	Logger::logMessage("[IOModule] PIC plugged into IO Space and CPU");
	Logger::logMessage("[IOModule] IOModule started");
}

IODeviceFE* IOModule::find(io_addr_t address) {
	for(auto i:_iomap) {
		if(i->start <= address && address <= i->end)
			return i->dev;
	}
	return NULL;	
}

bool IOModule::plug(IODeviceFE* dev,io_addr_t start,int numreg) {
	for(int i = 0; i < numreg; ++i) //Check if a map including start already exists
		if(find(start+i)) return false;
	_iomap.push_back(new IOMap(dev,start,start+numreg-1));
	Logger::logMessage("[IOModule] Device has been plugged in IO space from ",1);
	Logger::printHex(start,4);
	Logger::logMessage(" to ",1);	
	Logger::printHex(start+numreg-1,4);
	Logger::logMessage("");
	dev->_io = this;
	Logger::logMessage("[IOModule] Injected reference to IO space");
	return true;
}

bool IOModule::plug(IOInterruptDeviceFE* dev,io_addr_t start,int numreg,int ir_wire) {
	if(!plug((IODeviceFE*)dev,start,numreg)) return false;
	if(ir_wire >= 0) { //This is an interrupt capable device
		_pic->plug(dev,ir_wire);
		Logger::logMessage("[IOModule] Device plugged into PIC at wire: ",1);
		Logger::printHex(ir_wire,1);
		Logger::logMessage("");
	}
	else { //This is the PIC itself
		_cpu->plugPIC(_pic);
		Logger::logMessage("[IOModule] PIC plugged in CPU");
	}
	return true;
}
