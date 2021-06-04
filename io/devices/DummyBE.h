#include <iostream>
#include <stdlib.h>
#include <ctime>
#ifndef TYPES_H
#include "../../types.h"
#define TYPES_H
#endif

#ifndef IODEVFE_H
#include "IODeviceFE.h"
#define IODEVFE_H
#endif

#ifndef IODEVBE_H
#include "IODeviceBE.h"
#define IODEVBE_H
#endif

class DummyBE: public IODeviceBE {
public:
	DummyBE() {
		srand(0xFE);
	}

	uint8_t nextValue() {
		uint8_t r = (uint8_t)(((double)rand()/(double)RAND_MAX)*(double)265);
		return r;
	}
};
using namespace std;