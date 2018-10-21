#include <stdio.h>
#include <math.h>
#include <pigpio.h>
#include "motorCtrl.h"


#define TRUE 1
#define FALSE 0
#define UNDEFINED -1
#define MICROSECS_IN_SEC 1000000
#define SECS_IN_MIN 60
#define TOGGLES_IN_PULSE 2


double xLoc, yLoc, zLoc;
double xBgn, yBgn, zBgn;
double xEnd, yEnd, zEnd;
double xPosOffset = 0, yPosOffset = 0, zPosOffset = 0;
int xStepToggle = 0, yStepToggle = 0, zStepToggle = 0;
double xTimeElapsed = 0, yTimeElapsed = 0, zTimeElapsed = 0;
double xToggleTime = UNDEFINED;
double yToggleTime = UNDEFINED;
double zToggleTime = UNDEFINED;
int timeReset = TRUE;
int pathReset = TRUE;
int enableMotors = TRUE;
int xMotorEnabled, yMotorEnabled, zMotorEnabled;
double feedRate = UNDEFINED;
int moveMotorsReady = FALSE;
int relativePositioning = FALSE;
int gCodeReset = TRUE;

int jogReset = FALSE;


void motorCtrlReset()
{
   xPosOffset = 0; yPosOffset = 0; zPosOffset = 0;
   xStepToggle = 0; yStepToggle = 0; zStepToggle = 0;
   xTimeElapsed = 0; yTimeElapsed = 0; zTimeElapsed = 0;
   xToggleTime = UNDEFINED;
   yToggleTime = UNDEFINED;
   zToggleTime = UNDEFINED;
   timeReset = TRUE;
   pathReset = TRUE;
   enableMotors = TRUE;
   feedRate = UNDEFINED;
   moveMotorsReady = FALSE;
   relativePositioning = FALSE;
   gCodeReset = TRUE;
   
   jogReset = FALSE;
}


void moveMotors(double xPathFrac, double yPathFrac, double zPathFrac, struct config cfg)
{
   if ((xPathFrac <= yPathFrac) && (xPathFrac <= zPathFrac))
   {
      if (xTimeElapsed >= xToggleTime)
      {
         if (xEnd - xBgn > 0)
         {
            if (cfg.X_FLIP == TRUE)
            gpioWrite(cfg.X_MOTOR_DIRECT, PI_OFF);
            else
            gpioWrite(cfg.X_MOTOR_DIRECT, PI_ON);
         }
         else
         {
            if (cfg.X_FLIP == TRUE)
            gpioWrite(cfg.X_MOTOR_DIRECT, PI_ON);
            else
            gpioWrite(cfg.X_MOTOR_DIRECT, PI_OFF);
         }
         
         if (xStepToggle)
         gpioWrite(cfg.X_MOTOR_STEP, PI_ON);
         else
         gpioWrite(cfg.X_MOTOR_STEP, PI_OFF);
         
         xStepToggle = !xStepToggle;
         
         xTimeElapsed = 0;
      }
   }
   
   if ((yPathFrac <= xPathFrac) && (yPathFrac <= zPathFrac))
   {
      if (yTimeElapsed >= yToggleTime)
      {
         if (yEnd - yBgn > 0)
         {
            if (cfg.Y_FLIP == TRUE)
            gpioWrite(cfg.Y_MOTOR_DIRECT, PI_OFF);
            else
            gpioWrite(cfg.Y_MOTOR_DIRECT, PI_ON);
         }
         else
         {
            if (cfg.Y_FLIP == TRUE)
            gpioWrite(cfg.Y_MOTOR_DIRECT, PI_ON);
            else
            gpioWrite(cfg.Y_MOTOR_DIRECT, PI_OFF);
         }
         
         if (yStepToggle)
         gpioWrite(cfg.Y_MOTOR_STEP, PI_ON);
         else
         gpioWrite(cfg.Y_MOTOR_STEP, PI_OFF);
         
         yStepToggle = !yStepToggle;
         
         yTimeElapsed = 0;
      }
   }
   
   if ((zPathFrac <= xPathFrac) && (zPathFrac <= yPathFrac))
   {
      if (zTimeElapsed >= zToggleTime)
      {
         if (zEnd - zBgn > 0)
         {
            if (cfg.Z_FLIP == TRUE)
            gpioWrite(cfg.Z_MOTOR_DIRECT, PI_OFF);
            else
            gpioWrite(cfg.Z_MOTOR_DIRECT, PI_ON);
         }
         else
         {
            if (cfg.Z_FLIP == TRUE)
            gpioWrite(cfg.Z_MOTOR_DIRECT, PI_ON);
            else
            gpioWrite(cfg.Z_MOTOR_DIRECT, PI_OFF);
         }
         
         if (zStepToggle)
         gpioWrite(cfg.Z_MOTOR_STEP, PI_ON);
         else
         gpioWrite(cfg.Z_MOTOR_STEP, PI_OFF);
         
         zStepToggle = !zStepToggle;
         
         zTimeElapsed = 0;
      }
   }
}


int readInstructions(struct gcode *g, struct encoder x, struct encoder y, struct encoder z, struct config cfg)
{
   double travelLength;
   double longestAxisLength;
   double linearSpeed;
   
   linearSpeed = UNDEFINED;
   moveMotorsReady = FALSE;
   
   while (g->codeInc < g->codeCnt)
   {
      if (g->code[g->codeInc] == 'G')
      {
         if ((g->value[g->codeInc] == 0) || (g->value[g->codeInc] == 1))
         {
            if (g->value[g->codeInc] == 0)
            linearSpeed = cfg.POSITIONING_SPEED;
            
            g->codeInc++;
            while ((g->code[g->codeInc] == 'X') ||
                   (g->code[g->codeInc] == 'Y') ||
                   (g->code[g->codeInc] == 'Z') ||
                   (g->code[g->codeInc] == 'F'))
            {
               if (g->code[g->codeInc] == 'X')
               {
                  xBgn = xEnd;
                  if (relativePositioning)
                  xEnd += g->value[g->codeInc] - xPosOffset;
                  else
                  xEnd = g->value[g->codeInc] - xPosOffset;
                  moveMotorsReady = TRUE;
                  g->codeInc++;
               }
               
               if (g->code[g->codeInc] == 'Y')
               {
                  yBgn = yEnd;
                  if (relativePositioning)
                  yEnd += g->value[g->codeInc] - yPosOffset;
                  else
                  yEnd = g->value[g->codeInc] - yPosOffset;
                  moveMotorsReady = TRUE;
                  g->codeInc++;
               }
               
               if (g->code[g->codeInc] == 'Z')
               {
                  zBgn = zEnd;
                  if (relativePositioning)
                  zEnd += g->value[g->codeInc] - zPosOffset;
                  else
                  zEnd = g->value[g->codeInc] - zPosOffset;
                  moveMotorsReady = TRUE;
                  g->codeInc++;
               }
               
               if (g->code[g->codeInc] == 'F')
               {
                  feedRate = g->value[g->codeInc];
                  g->codeInc++;
               }
            }
            
            g->codeInc--;
            break;
         }
         
         if (g->value[g->codeInc] == 4)
         {
            g->codeInc++;
            gpioDelay(g->value[g->codeInc]*1000);
         }
         
         // use absolute positioning
         if (g->value[g->codeInc] == 90)
         {
            relativePositioning = FALSE;
         }
         
         // use relative positioning
         if (g->value[g->codeInc] == 91)
         {
            relativePositioning = TRUE;
         }
         
         // set current position
         if (g->value[g->codeInc] == 92)
         {
            g->codeInc++;
            while ((g->code[g->codeInc] == 'X') ||
                   (g->code[g->codeInc] == 'Y') ||
                   (g->code[g->codeInc] == 'Z'))
            {
               if (g->code[g->codeInc] == 'X')
               {
                  xPosOffset = g->value[g->codeInc] - x.loc / (double)cfg.X_MECHANICAL_DEGREES_PER_MM;;
                  xBgn -= xPosOffset;
                  xEnd -= xPosOffset;
                  g->codeInc++;
               }
               
               if (g->code[g->codeInc] == 'Y')
               {
                  yPosOffset = g->value[g->codeInc] - y.loc / (double)cfg.X_MECHANICAL_DEGREES_PER_MM;;
                  yBgn -= yPosOffset;
                  yEnd -= yPosOffset;
                  g->codeInc++;
               }
               
               if (g->code[g->codeInc] == 'Z')
               {
                  zPosOffset = g->value[g->codeInc] - z.loc / (double)cfg.X_MECHANICAL_DEGREES_PER_MM;;
                  zBgn -= zPosOffset;
                  zEnd -= zPosOffset;
                  g->codeInc++;
               }
            }
            
            g->codeInc--;
         }
      }
      
      if (g->code[g->codeInc] == 'M')
      {
         if (g->value[g->codeInc] == 0)
         {
            g->codeInc++;
            return PROGRAM_PAUSE;
         }
         
         if (g->value[g->codeInc] == 2)
         {
            motorCtrlReset();
            return PROGRAM_END;
         }
         
         // enable axes
         if (g->value[g->codeInc] == 17)
         {
            g->codeInc++;
            while ((g->code[g->codeInc] == 'X') ||
                   (g->code[g->codeInc] == 'Y') ||
                   (g->code[g->codeInc] == 'Z'))
            {
               if (g->code[g->codeInc] == 'X')
               {
                  gpioWrite(cfg.X_MOTOR_ENABLE, PI_ON);
                  xMotorEnabled = TRUE;
                  g->codeInc++;
               }
               
               if (g->code[g->codeInc] == 'Y')
               {
                  gpioWrite(cfg.Y_MOTOR_ENABLE, PI_ON);
                  yMotorEnabled = TRUE;
                  g->codeInc++;
               }
               
               if (g->code[g->codeInc] == 'Z')
               {
                  gpioWrite(cfg.Z_MOTOR_ENABLE, PI_ON);
                  zMotorEnabled = TRUE;
                  g->codeInc++;
               }
            }
            
            g->codeInc--;
         }
         
         // disable axes
         if (g->value[g->codeInc] == 18)
         {
            g->codeInc++;
            while ((g->code[g->codeInc] == 'X') ||
                   (g->code[g->codeInc] == 'Y') ||
                   (g->code[g->codeInc] == 'Z'))
            {
               if (g->code[g->codeInc] == 'X')
               {
                  gpioWrite(cfg.X_MOTOR_ENABLE, PI_OFF);
                  xMotorEnabled = FALSE;
                  g->codeInc++;
               }
               
               if (g->code[g->codeInc] == 'Y')
               {
                  gpioWrite(cfg.Y_MOTOR_ENABLE, PI_OFF);
                  yMotorEnabled = FALSE;
                  g->codeInc++;
               }
               
               if (g->code[g->codeInc] == 'Z')
               {
                  gpioWrite(cfg.Z_MOTOR_ENABLE, PI_OFF);
                  zMotorEnabled = FALSE;
                  g->codeInc++;
               }
            }
            
            g->codeInc--;
         }
      }
      
      g->codeInc++;
   }
   
   
   if (g->codeInc >= g->codeCnt)
   {
      motorCtrlReset();
      return PROGRAM_END;
   }
   
   // return error if no feed rate
   if ((feedRate == UNDEFINED) && (linearSpeed == UNDEFINED))
   {
      motorCtrlReset();
      return ERR_NO_FEED_RATE;
   }
   
   if (linearSpeed == UNDEFINED)
   linearSpeed = feedRate;
   
   // return error if out of bounds
   if ((xEnd > cfg.X_MAX_LIMIT) || (xEnd < cfg.X_MIN_LIMIT))
   {
      motorCtrlReset();
      return OUT_OF_BOUNDS;
   }
   
   if ((yEnd > cfg.Y_MAX_LIMIT) || (yEnd < cfg.Y_MIN_LIMIT))
   {
      motorCtrlReset();
      return OUT_OF_BOUNDS;
   }
   
   if ((zEnd > cfg.Z_MAX_LIMIT) || (zEnd < cfg.Z_MIN_LIMIT))
   {
      motorCtrlReset();
      return OUT_OF_BOUNDS;
   }
   
   
   // find stepper motor driver input toggle timings
   
   xToggleTime = (double)(SECS_IN_MIN * MICROSECS_IN_SEC) /
                         (cfg.X_PULSES_PER_MM * TOGGLES_IN_PULSE * linearSpeed);
   yToggleTime = (double)(SECS_IN_MIN * MICROSECS_IN_SEC) /
                         (cfg.Y_PULSES_PER_MM * TOGGLES_IN_PULSE * linearSpeed);
   zToggleTime = (double)(SECS_IN_MIN * MICROSECS_IN_SEC) /
                         (cfg.Z_PULSES_PER_MM * TOGGLES_IN_PULSE * linearSpeed);
   
   // find travel length
   travelLength = sqrt(pow(sqrt(pow(xEnd - xBgn, 2) +
                                pow(yEnd - yBgn, 2)), 2) +
                                pow(zEnd - zBgn, 2));
   
   // find the axis with the longest travel length
   if (fabs(xEnd - xBgn) > fabs(yEnd - yBgn))
   longestAxisLength = fabs(xEnd - xBgn);
   else
   longestAxisLength = fabs(yEnd - yBgn);
   
   if (fabs(zEnd - zBgn) > longestAxisLength)
   longestAxisLength = fabs(zEnd - zBgn);
   
   // increase the toggle time for an axis the more perpendicular it is to the path
   // 2 axes each moving at 0.7071... mm/sec == a combined 1.0 mm/sec
   xToggleTime *= travelLength / longestAxisLength;
   yToggleTime *= travelLength / longestAxisLength;
   zToggleTime *= travelLength / longestAxisLength;
   
   return 0;
}


int executeGcode(struct gcode *g, struct encoder x, struct encoder y, struct encoder z, struct config cfg)
{
   double xPathFrac, yPathFrac, zPathFrac;
   static uint32_t startTick, endTick;
   int ret = 0;
   
   
   // find how much time elapsed
   if (timeReset)
   {
      endTick = gpioTick();
      timeReset = FALSE;
   }
   startTick = endTick;
   endTick = gpioTick();
   
   xTimeElapsed += endTick - startTick;
   yTimeElapsed += endTick - startTick;
   zTimeElapsed += endTick - startTick;
   
   
   if (enableMotors)
   {
      gpioWrite(cfg.X_MOTOR_ENABLE, PI_ON);
      gpioWrite(cfg.Y_MOTOR_ENABLE, PI_ON);
      gpioWrite(cfg.Z_MOTOR_ENABLE, PI_ON);
      xMotorEnabled = TRUE;
      yMotorEnabled = TRUE;
      zMotorEnabled = TRUE;
      enableMotors = FALSE;
   }
   
   
   // convert mechanical degrees to millimeters
   xLoc = x.loc / (double)cfg.X_MECHANICAL_DEGREES_PER_MM;
   yLoc = y.loc / (double)cfg.Y_MECHANICAL_DEGREES_PER_MM;
   zLoc = z.loc / (double)cfg.Z_MECHANICAL_DEGREES_PER_MM;
   
   
   if (pathReset)
   {
      xBgn = xLoc;//no path
      xEnd = xLoc;
      yBgn = yLoc;
      yEnd = yLoc;
      zBgn = zLoc;
      zEnd = zLoc;
      pathReset = FALSE;
   }
   
   if (gCodeReset)
   {
      g->codeInc = 0;
      gCodeReset = FALSE;
   }
   
   // get the percentage of how much each axis has traveled
   if (xEnd - xBgn == 0.0)
   xPathFrac = 1.0;
   else
   xPathFrac = (xLoc - xBgn) / (xEnd - xBgn);
   
   if (yEnd - yBgn == 0.0)
   yPathFrac = 1.0;
   else
   yPathFrac = (yLoc - yBgn) / (yEnd - yBgn);
   
   if (zEnd - zBgn == 0.0)
   zPathFrac = 1.0;
   else
   zPathFrac = (zLoc - zBgn) / (zEnd - zBgn);
   
   
   if (((xPathFrac >= 1.0) || (xMotorEnabled == FALSE)) &&
       ((yPathFrac >= 1.0) || (yMotorEnabled == FALSE)) &&
       ((zPathFrac >= 1.0) || (zMotorEnabled == FALSE)))
   moveMotorsReady = FALSE;
   
   
   if (moveMotorsReady)
   moveMotors(xPathFrac, yPathFrac, zPathFrac, cfg);
   else
   ret = readInstructions(g, x, y, z, cfg);
   
   
   return ret;
}


int jog(double xJog, double yJog, double zJog, double jogSpeed,
        struct encoder x, struct encoder y, struct encoder z, struct config cfg, int disableBounds)
{
   static uint32_t startTick, endTick;
   double xPathFrac, yPathFrac, zPathFrac;
   static double xJogPrior = 0, yJogPrior = 0, zJogPrior = 0;
   double linearSpeed;
   double travelLength;
   double longestAxisLength;
   
   
   // find how much time elapsed
   if (timeReset)
   {
      endTick = gpioTick();
      timeReset = FALSE;
   }
   startTick = endTick;
   endTick = gpioTick();
   
   xTimeElapsed += endTick - startTick;
   yTimeElapsed += endTick - startTick;
   zTimeElapsed += endTick - startTick;
   
   
   if (enableMotors)
   {
      gpioWrite(cfg.X_MOTOR_ENABLE, PI_ON);
      gpioWrite(cfg.Y_MOTOR_ENABLE, PI_ON);
      gpioWrite(cfg.Z_MOTOR_ENABLE, PI_ON);
      xMotorEnabled = TRUE;
      yMotorEnabled = TRUE;
      zMotorEnabled = TRUE;
      enableMotors = FALSE;
   }
   
   
   // convert mechanical degrees to millimeters
   xLoc = x.loc / (double)cfg.X_MECHANICAL_DEGREES_PER_MM;
   yLoc = y.loc / (double)cfg.Y_MECHANICAL_DEGREES_PER_MM;
   zLoc = z.loc / (double)cfg.Z_MECHANICAL_DEGREES_PER_MM;
   
   
   if (pathReset)
   {
      xBgn = xLoc;//no path
      xEnd = xLoc;
      yBgn = yLoc;
      yEnd = yLoc;
      zBgn = zLoc;
      zEnd = zLoc;
      pathReset = FALSE;
   }
   
   // get the percentage of how much each axis has traveled
   if (xEnd - xBgn == 0.0)
   xPathFrac = 1.0;
   else
   xPathFrac = (xLoc - xBgn) / (xEnd - xBgn);
   
   if (yEnd - yBgn == 0.0)
   yPathFrac = 1.0;
   else
   yPathFrac = (yLoc - yBgn) / (yEnd - yBgn);
   
   if (zEnd - zBgn == 0.0)
   zPathFrac = 1.0;
   else
   zPathFrac = (zLoc - zBgn) / (zEnd - zBgn);
   
   
   if ((xPathFrac >= 1.0) && (yPathFrac >= 1.0) && (zPathFrac >= 1.0))
   moveMotorsReady = FALSE;
   
   if ((xJog != xJogPrior) || (yJog != yJogPrior) || (zJog != zJogPrior))
   moveMotorsReady = FALSE;
   
   
   xJogPrior = xJog;
   yJogPrior = yJog;
   zJogPrior = zJog;
   
   
   // return error if out of bounds
   if (disableBounds == FALSE)
   if (((xLoc > cfg.X_MAX_LIMIT) && (xJog > 0)) ||
       ((yLoc > cfg.Y_MAX_LIMIT) && (yJog > 0)) ||
       ((zLoc > cfg.Z_MAX_LIMIT) && (zJog > 0)) ||
       ((xLoc < cfg.X_MIN_LIMIT) && (xJog < 0)) ||
       ((yLoc < cfg.Y_MIN_LIMIT) && (yJog < 0)) ||
       ((zLoc < cfg.Z_MIN_LIMIT) && (zJog < 0)))
   {
      motorCtrlReset();
      return OUT_OF_BOUNDS;
   }
   
   
   if (moveMotorsReady)
   {
      moveMotors(xPathFrac, yPathFrac, zPathFrac, cfg);
      jogReset = TRUE;
   }
   else
   {
      xBgn = xEnd;
      xEnd += xJog;
      
      yBgn = yEnd;
      yEnd += yJog;
      
      zBgn = zEnd;
      zEnd += zJog;
      
      linearSpeed = jogSpeed;
      
      // find stepper motor driver input toggle timings
      
      xToggleTime = (double)(SECS_IN_MIN * MICROSECS_IN_SEC) /
                            (cfg.X_PULSES_PER_MM * TOGGLES_IN_PULSE * linearSpeed);
      yToggleTime = (double)(SECS_IN_MIN * MICROSECS_IN_SEC) /
                            (cfg.Y_PULSES_PER_MM * TOGGLES_IN_PULSE * linearSpeed);
      zToggleTime = (double)(SECS_IN_MIN * MICROSECS_IN_SEC) /
                            (cfg.Z_PULSES_PER_MM * TOGGLES_IN_PULSE * linearSpeed);
      
      // find travel length
      travelLength = sqrt(pow(sqrt(pow(xEnd - xBgn, 2) +
                                   pow(yEnd - yBgn, 2)), 2) +
                                   pow(zEnd - zBgn, 2));
      
      // find the axis with the longest travel length
      if (fabs(xEnd - xBgn) > fabs(yEnd - yBgn))
      longestAxisLength = fabs(xEnd - xBgn);
      else
      longestAxisLength = fabs(yEnd - yBgn);
      
      if (fabs(zEnd - zBgn) > longestAxisLength)
      longestAxisLength = fabs(zEnd - zBgn);
      
      // increase the toggle time for an axis the more perpendicular it is to the path
      // 2 axes each moving at 0.7071... mm/sec == a combined 1.0 mm/sec
      xToggleTime *= travelLength / longestAxisLength;
      yToggleTime *= travelLength / longestAxisLength;
      zToggleTime *= travelLength / longestAxisLength;
      
      moveMotorsReady = TRUE;
      
      if (jogReset)
      {
         motorCtrlReset();
         return END_OF_JOG;
      }
   }
   
   return 0;
}
