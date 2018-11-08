#include <stdio.h>
#include <pigpio.h>

/*
#define MOTOR_ENABLE 2 // X
#define MOTOR_DIRECT 3 // X
#define MOTOR_STEP 4   // X
*/
/*
#define MOTOR_ENABLE 17 // Y
#define MOTOR_DIRECT 27 // Y
#define MOTOR_STEP 22   // Y
*/

#define MOTOR_ENABLE 18 // Z
#define MOTOR_DIRECT 23 // Z
#define MOTOR_STEP 24   // Z


int main(void)
{
   int i;
   int stepToggle = 1;
   
   if (gpioInitialise() < 0)
   {
      fprintf(stderr, "pigpio initialisation failed\n");
      return 1;
   }

   // set motor pins
   gpioSetMode(MOTOR_ENABLE, PI_OUTPUT);
   gpioSetMode(MOTOR_DIRECT, PI_OUTPUT);
   gpioSetMode(MOTOR_STEP, PI_OUTPUT);
   
   gpioWrite(MOTOR_ENABLE, PI_ON);
   gpioWrite(MOTOR_DIRECT, PI_ON); // positive direction
   gpioWrite(MOTOR_STEP, PI_OFF);

   for (i=0; i < 100; i++)
   {
      if (stepToggle)
      gpioWrite(MOTOR_STEP, PI_ON);
      else
      gpioWrite(MOTOR_STEP, PI_OFF);

      stepToggle = !stepToggle;
      
      if (stepToggle == 0)
      printf("%i input pulses sent\n", i/2+1);
      
      gpioDelay(10000);
   }

   gpioWrite(MOTOR_ENABLE, PI_OFF);
   gpioWrite(MOTOR_DIRECT, PI_OFF);
   gpioWrite(MOTOR_STEP, PI_OFF);

   gpioTerminate();
   
   return 0;
}
