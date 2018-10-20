#include "encoder.h"

#define TRUE 1
#define FALSE 0


void getEncoderLoc(struct encoder *enc)
{
   // A:_--__--__--_ moving to the
   // B:__--__--__-- positive direction

   // A:_--__--__--_ moving to the
   // B:--__--__--__ negative direction

   // if the A or B pin on the encoder changes to a different state
   if (((enc->currState == A_HIGH) && (enc->priorState == AB_LOW)) ||
       ((enc->currState == AB_HIGH) && (enc->priorState == A_HIGH)) ||
       ((enc->currState == B_HIGH) && (enc->priorState == AB_HIGH)) ||
       ((enc->currState == AB_LOW) && (enc->priorState == B_HIGH)))
   enc->loc += 1;

   if (((enc->currState == B_HIGH) && (enc->priorState == AB_LOW)) ||
       ((enc->currState == AB_HIGH) && (enc->priorState == B_HIGH)) ||
       ((enc->currState == A_HIGH) && (enc->priorState == AB_HIGH)) ||
       ((enc->currState == AB_LOW) && (enc->priorState == A_HIGH)))
   enc->loc -= 1;

   // error checking
   // if the A and B pins on the encoder skips a state
   if (((enc->currState == AB_HIGH) && (enc->priorState == AB_LOW)) ||
       ((enc->currState == AB_LOW) && (enc->priorState == AB_HIGH)) ||
       ((enc->currState == A_HIGH) && (enc->priorState == B_HIGH)) ||
       ((enc->currState == B_HIGH) && (enc->priorState == A_HIGH)))
   enc->error = TRUE;
      
   enc->priorState = enc->currState;
}

void getFlippedEncoderLoc(struct encoder *enc)
{
   // A:_--__--__--_ moving to the
   // B:__--__--__-- negative direction

   // A:_--__--__--_ moving to the
   // B:--__--__--__ positive direction

   // if the A or B pin on the encoder changes to a different state
   if (((enc->currState == A_HIGH) && (enc->priorState == AB_LOW)) ||
       ((enc->currState == AB_HIGH) && (enc->priorState == A_HIGH)) ||
       ((enc->currState == B_HIGH) && (enc->priorState == AB_HIGH)) ||
       ((enc->currState == AB_LOW) && (enc->priorState == B_HIGH)))
   enc->loc -= 1;

   if (((enc->currState == B_HIGH) && (enc->priorState == AB_LOW)) ||
       ((enc->currState == AB_HIGH) && (enc->priorState == B_HIGH)) ||
       ((enc->currState == A_HIGH) && (enc->priorState == AB_HIGH)) ||
       ((enc->currState == AB_LOW) && (enc->priorState == A_HIGH)))
   enc->loc += 1;

   // error checking
   // if the A and B pins on the encoder skips a state
   if (((enc->currState == AB_HIGH) && (enc->priorState == AB_LOW)) ||
       ((enc->currState == AB_LOW) && (enc->priorState == AB_HIGH)) ||
       ((enc->currState == A_HIGH) && (enc->priorState == B_HIGH)) ||
       ((enc->currState == B_HIGH) && (enc->priorState == A_HIGH)))
   enc->error = TRUE;
      
   enc->priorState = enc->currState;
}
