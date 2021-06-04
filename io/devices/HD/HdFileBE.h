#include <iostream>
#include <stdlib.h>
#include <ctime>
#ifndef TYPES_H
#include "../../../types.h"
#define TYPES_H
#endif

#ifndef IODEVFE_H
#include "../IODeviceFE.h"
#define IODEVFE_H
#endif

#ifndef HDBE_H
#include "HdBE.h"
#define HDBE_H
#endif
#define SECTOR_SIZE 512
#include <fstream>



using namespace std;
class HdFileBE: public HdBE{
	fstream fd;
	addr_t size;
	bool outsideHD(unsigned int sn);
public:
	HdFileBE(const char * filename,addr_t s);
	~HdFileBE(){fd.close();};
	virtual bool write_sector(int sn, const mem_row_t*buf) override;
	virtual bool read_sector(int sn, const mem_row_t*buf) override;
};