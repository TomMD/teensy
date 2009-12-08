// teensy_blobs.c
// Author : A. John Ochsner IV
// Group  : Rebekah Leslie & Thomas DuBuisson

#include "teensy_blobs.h"
#include "../commands.h"
#include <string.h>

uint64_t total_reads = 0;
uint64_t total_writes = 0;
uint64_t bytes_read = 0;
uint64_t bytes_written = 0;

blob_t blobs[MAX_BLOBS];

uint8_t teensy_store(uint64_t index, uint16_t data)
{
	if ( (index < 0) || (index > MAX_BLOBS) )
		return EINVALIDINDEX;
	blobs[index].data = data;
	return ENOERR;
}

uint8_t teensy_read(uint64_t index, uint16_t * data)
{
	if ( (index < 0) || (index > MAX_BLOBS) )
		return EINVALIDINDEX;
	//memcpy( (void*) data, (void*) &blobs[index].data, 2);
	*data = blobs[index].data;
	return ENOERR;
}

uint8_t teensy_delete(uint64_t index)
{
	if ( (index < 0) || (index > MAX_BLOBS) )
		return EINVALIDINDEX;
	blobs[index].data = 0;

	return ENOERR;
}


