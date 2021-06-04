#include "HdFE.h"
addr_t HdFE::fetchAddress(addr_t start, bool count) {
	mem_row_t addr1 = _memory[start++];
	mem_row_t addr2 = _memory[start++];
	mem_row_t addr3 = _memory[start++];
	mem_row_t addr4 = _memory[start++];
	addr_t addr = ((addr_t)addr1) + ((addr_t)addr2 << 8) + ((addr_t) addr3 << 16);
	if(count){
		addr = addr + ((addr_t)addr4 << 24);
	}
	return addr;
}

void  HdFE::setRegister(io_addr_t addr,mem_row_t val){
	int index = addr&ADDR_MASK;
	switch (index){
		case iDMAPTR:
			DMAPTR += (((addr_t)val) << (8*next_dma_ptr++));
			if(next_dma_ptr>2) next_dma_ptr=0;
			break;
		case iSN1:
			SN1 = val;
			break;
		case iSN2:
			SN2 = val;
			break;
		case iSN3:
			SN3 = val;
			break;
		case iSN4:
			SN4 = val;
			break;
		case iCMD:
			CMD = val;
			RSR&=0x00; //Reset status register whenever you start a new transaction
			next=next_dma_ptr=0; //Reset pointers as well
			if (CMD&DMA_MASK){ // A dma operation has been requested
				addr_t where = fetchAddress(DMAPTR,false);
				addr_t count = fetchAddress(DMAPTR+3,true);
				uint32_t sn = SN1 | SN2 << 8 | SN3 << 16 | SN4 << 24; 
				for (addr_t i = 0; i < count; ++i) {
					if (!(CMD&RW_MASK)) {
						if(!be->read_sector(sn+i,_memory.begin()+where+SECTOR_SIZE*i)) {
							RSR|=0x80;//Setting FAIL BIT
							break;
						}
					}
					else {
						if(!be->write_sector(sn+i,_memory.begin()+where+SECTOR_SIZE*i)) {
							RSR|=0x80;
							break;
						}
					}
				}
				RSR|=0x01;
				if (CMD&INT_MASK){
					*_int_ref = true; 
					_io->notifyPIC();
				}
				return;
			}
			if(!(CMD&RW_MASK)) {// A read operation has been requested, in any case prebuffer the sector
				uint32_t sn = SN1 | SN2 << 8 | SN3 << 16 | SN4 << 24;
				if(!be->read_sector(sn,BR)) RSR|=0x80; //Setting FAIL BIT
				RSR|=0x01; //Setting FINISH BIT
				if(CMD&INT_MASK){
					*_int_ref = true; 
					_io->notifyPIC();
				}
				return;	
			}
			break;
		case iBR:
			if(!CMD&RW_MASK){ //NO WRITE OPERATION
				Logger::logMessage("Doing a write transaction without write command in CMD");
				break;
			}
			BR[next++] = val;
			if(next == SECTOR_SIZE){
				uint32_t sn = SN1 | SN2 << 8 | SN3 << 16 | SN4 << 24;
				if(!be->write_sector(sn,BR)) RSR|=0x80; //Setting FAIL BIT
				RSR|=0x01; //Setting FINISH BIT
				next = 0;
				if(CMD&INT_MASK){
					*_int_ref = true; 
					_io->notifyPIC();
				}
			}
			break;
		default:
			break;	
	}
}
mem_row_t HdFE::getRegister(io_addr_t addr){
	int index = addr&ADDR_MASK;
	switch (index){
		case iBR: //SIMPLY RETURN CURRENT BYTE FROM BUFFER REGISTER
		{
			if(RSR==0x01) { //READ OPERATIONS DONE WHEN DEVICE IS NOT READY ARE IGNORED
				mem_row_t v = BR[next++];
				next = next%SECTOR_SIZE;
				return v;
			}
			break;
		}
		case iRSR:
			return RSR;
		default:
			break;	
	}
	return 0x00;
}

void HdFE::setInterruptWire(bool* b) {
	_int_ref=b;
}