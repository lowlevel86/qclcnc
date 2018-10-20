#ifndef _INDEX_ALIGNMENT_HEADER_H_
#define _INDEX_ALIGNMENT_HEADER_H_

#include "encoder.h"

struct encoderIndex {
   int state;
   int offset;
   int length;
   int firstIndexPulseLoc;
   int secondIndexPulseLoc;
   int firstIndexPulseFound;
   int secondIndexPulseFound;
   int offsetFound;
};

void initIndexAlignmentVars(struct encoderIndex *);
void addIndexOffsetToLoc(struct encoder *, struct encoderIndex *);
void getIndexOffset(struct encoder *, struct encoderIndex *);

#endif // _INDEX_ALIGNMENT_HEADER_H_
