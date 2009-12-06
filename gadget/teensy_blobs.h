// teensy_blobs.h
// Author : A. John Ochsner IV
// Group  : Rebekah Leslie & Thomas DuBuisson

#ifndef TEENSY_BLOBS_H
#define TEENSY_BLOBS_H
#endif

#include <avr/pgmspace.h>

#define MAX_BLOBS 8192

struct blob
  {
	prog_uint16_t data;
  };

typedef struct blob blob_t;

uint8_t teensy_store(uint64_t index, uint16_t data);
uint8_t teensy_read(uint64_t index, uint16_t *data);
uint8_t teensy_delete(uint64_t index);

