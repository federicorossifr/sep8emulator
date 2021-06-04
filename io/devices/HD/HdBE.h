#include <iostream>
#ifndef IODEVBE_H
#include "../IODeviceBE.h"
#define IODEVBE_H
#endif
#include <stdlib.h>
#include <ctime>
#ifndef TYPES_H
#include "../../../types.h"
#define TYPES_H
#endif
using namespace std;

class HdBE: public IODeviceBE
{
public:
	virtual ~HdBE() {;}
	virtual bool write_sector(int sn, const mem_row_t*buf)=0;
	virtual bool read_sector(int sn, const mem_row_t*buf)=0;
};