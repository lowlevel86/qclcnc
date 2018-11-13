#include <stdio.h>
#include <unistd.h>
#include <pigpio.h>

#define TRUE 1
#define FALSE 0

//#define ENCODER_A 06 // X
//#define ENCODER_B 12 // X

//#define ENCODER_A 19 // Y
//#define ENCODER_B 26 // Y

#define ENCODER_A 20 // Z
#define ENCODER_B 21 // Z

// encoder states
#define A_HIGH 1
#define B_HIGH 2
#define AB_HIGH 3
#define AB_LOW 0


struct encoder {
   int loc;
   int currState;
   int priorState;
   int error;
};
struct encoder enc;

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


void encEventCallBack(int gpio, int level, uint32_t tick)
{
   if (gpio == ENCODER_A)
   {
      if (level == 1)
      enc.currState |= 0b01; // make the first bit high
      
      if (level == 0)
      enc.currState &= 0b10; // make the first bit low
   }
   
   if (gpio == ENCODER_B)
   {
      if (level == 1)
      enc.currState |= 0b10; // make the second bit high
      
      if (level == 0)
      enc.currState &= 0b01; // make the second bit low
   }
   
   if ((gpio == ENCODER_A) || (gpio == ENCODER_B))
   getEncoderLoc(&enc);
   
   printf("Mechanical degree units: %i\n", enc.loc);
   
   if (enc.error)
   printf("Encoder Error!\n");
}


int main(int argc, char *argv[])
{
   if (gpioInitialise() < 0)
   {
      printf("Pigpio initialisation failed\n");
      return 1;
   }

   // set encoder pins
   gpioSetMode(ENCODER_A, PI_INPUT);
   gpioSetPullUpDown(ENCODER_A, PI_PUD_DOWN);
   gpioSetMode(ENCODER_B, PI_INPUT);
   gpioSetPullUpDown(ENCODER_B, PI_PUD_DOWN);

   // initialize encoder
   enc.loc = 0;
   enc.currState = gpioRead(ENCODER_A) | (gpioRead(ENCODER_B) << 1);
   enc.priorState = enc.currState;
   enc.error = FALSE;
   
   // create gpio alerts
   gpioSetAlertFunc(ENCODER_A, encEventCallBack);
   gpioSetAlertFunc(ENCODER_B, encEventCallBack);

   sleep(10);// the encEventCallBack will be waiting for events during this time

   // cancel gpio alerts
   gpioSetAlertFunc(ENCODER_A, 0);
   gpioSetAlertFunc(ENCODER_B, 0);
   
   gpioTerminate();
   
   return 0;
}
