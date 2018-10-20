#include <stdio.h>
#include <stdlib.h>
#include "indexAlignment.h"
#include "encoder.h"

#define TRUE 1
#define FALSE 0
#define HIGH 1
#define LOW 0
#define UNDEFINED -1


void initIndexAlignmentVars(struct encoderIndex *encIndex)
{
   encIndex->state = UNDEFINED;
   encIndex->firstIndexPulseFound = FALSE;
   encIndex->secondIndexPulseFound = FALSE;
   encIndex->offsetFound = FALSE;
}


void addIndexOffsetToLoc(struct encoder *enc, struct encoderIndex *encIndex)
{
   if (encIndex->offset)
   if (encIndex->offsetFound == TRUE)
   if ((encIndex->firstIndexPulseFound == TRUE) && (encIndex->secondIndexPulseFound == TRUE))
   {
      enc->loc += encIndex->offset;
      encIndex->offset = 0;
   }
}


void getIndexOffset(struct encoder *enc, struct encoderIndex *encIndex)
{
   int offset;
   
   if ((encIndex->state == HIGH) && (enc->currState == AB_LOW) && (encIndex->firstIndexPulseFound == FALSE))
   {
      encIndex->firstIndexPulseLoc = enc->loc;
      encIndex->firstIndexPulseFound = TRUE;
   }
   
   if ((encIndex->state == HIGH) && (enc->currState == AB_LOW) && (encIndex->secondIndexPulseFound == FALSE))
   {
      encIndex->secondIndexPulseLoc = enc->loc;
      
      if (encIndex->secondIndexPulseLoc != encIndex->firstIndexPulseLoc)
      {
         encIndex->length = abs(encIndex->secondIndexPulseLoc - encIndex->firstIndexPulseLoc);
         encIndex->secondIndexPulseFound = TRUE;
      }
   }

   // find offset to snap location 0 to index pulse location
   if (encIndex->offsetFound == FALSE)
   if ((encIndex->firstIndexPulseFound == TRUE) && (encIndex->secondIndexPulseFound == TRUE))
   {
      offset = encIndex->firstIndexPulseLoc % encIndex->length;
      
      if (encIndex->firstIndexPulseLoc >= 0)
      {
         if (offset < encIndex->length / 2)
         encIndex->offset = -offset;
         else
         encIndex->offset = encIndex->length - offset;
      }
      else
      {
         if (-offset < encIndex->length / 2)
         encIndex->offset = -offset;
         else
         encIndex->offset = -(encIndex->length + offset);
      }
      
      encIndex->offsetFound = TRUE;
   }
}
