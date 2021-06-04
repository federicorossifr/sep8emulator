#include "HdFileBE.h"

HdFileBE::HdFileBE(const char * filename,addr_t s){
	size = s;
	fd=fstream(filename, ios_base::in | ios_base::out | ios_base::binary);
	if(!fd.is_open()){
		Logger::logMessage("Error during HD file opening");
		exit(1);
	}
}


bool HdFileBE::write_sector(int sn, const mem_row_t*buf){
	int offset = sn * SECTOR_SIZE;
	if(outsideHD(offset)){
		Logger::logMessage("Sector number is outside HD");
		return false;	
	}
	fd.seekg (offset, fd.beg);
	fd.write((const char*)buf,SECTOR_SIZE);
	fd.flush();
	return true;
}
bool HdFileBE::read_sector(int sn, const mem_row_t*buf){
	int offset = sn * SECTOR_SIZE;
	if(outsideHD(offset)){
		Logger::logMessage("Sector number is outside HD");		
		return false;	
	}
	fd.seekg(offset, fd.beg);
	fd.read((char*)buf,SECTOR_SIZE);
	return true;
}
bool HdFileBE::outsideHD(unsigned int off){
	if (off>size)
		return true;
	return false;
}