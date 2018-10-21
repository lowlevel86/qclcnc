#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <pigpio.h>
#include <float.h>
#include "encoder.h"
#include "indexAlignment.h"
#include "configure.h"
#include "motorCtrl.h"
#include "gcodeParser.h"
#include "dirFiles.h"


#define TRUE 1
#define FALSE 0
#define UNDEFINED -1
#define POSITIVE 1
#define NEGATIVE -1


#define MAIN_MENU_MODE 0

#define CONFIGURE_MODE 10
#define X_INDEX_PULSE_SEARCH_MODE 11
#define Y_INDEX_PULSE_SEARCH_MODE 12
#define Z_INDEX_PULSE_SEARCH_MODE 13
#define DEFINE_RANGES_MODE 14
#define DEFINE_RANGES_MANUAL_MODE 15
#define DEFINE_RANGES_JOG_MODE 16

#define MOVE_JOG_MODE 20
#define MANUAL_MODE 21
#define JOG_MODE 22

#define LOAD_GCODE_MODE 30

#define GCODE_EXE_MODE 40
#define GCODE_WAIT_MODE 41

#define EXIT_PROGRAM 50


struct encoder xEnc;
struct encoder yEnc;
struct encoder zEnc;
struct encoderIndex xEncIndex;
struct encoderIndex yEncIndex;
struct encoderIndex zEncIndex;
struct config cfg;
struct gcode g;

int machineState = MAIN_MENU_MODE;
char *workingDirectory;
char *workingDirDisplay;
int gfileIncBase = 0;
int gfileSelectNum = -1;
int gfileCnt = 0;
char **gfiles;
char *gCodeFile;
int gCodeIsLoaded = FALSE;

int gCodeRet;
int displayGcodeRet = FALSE;

int searchForIndexPulse = FALSE;
int searchDirection = POSITIVE;
int returnToPreservedLoc = FALSE;
int preserveEncoderLocation = TRUE;
float preservedLoc;

int xLocationConfirmed = FALSE;
int yLocationConfirmed = FALSE;
int zLocationConfirmed = FALSE;
int getIndexOffsetOnly;

double xMinRange, yMinRange, zMinRange;
double xMaxRange, yMaxRange, zMaxRange;

int xJog = 0, yJog = 0, zJog = 0;
int exitProg = FALSE;


void encEventCallBack(int gpio, int level, uint32_t tick)
{
   if (gpio == cfg.X_ENCODER_A)
   {
      if (level == 1)
      xEnc.currState |= 0b01; // make the first bit high
      
      if (level == 0)
      xEnc.currState &= 0b10; // make the first bit low
   }
   if (gpio == cfg.X_ENCODER_B)
   {
      if (level == 1)
      xEnc.currState |= 0b10; // make the second bit high
      
      if (level == 0)
      xEnc.currState &= 0b01; // make the second bit low
   }
   
   if (gpio == cfg.Y_ENCODER_A)
   {
      if (level == 1)
      yEnc.currState |= 0b01; // make the first bit high
      
      if (level == 0)
      yEnc.currState &= 0b10; // make the first bit low
   }
   if (gpio == cfg.Y_ENCODER_B)
   {
      if (level == 1)
      yEnc.currState |= 0b10; // make the second bit high
      
      if (level == 0)
      yEnc.currState &= 0b01; // make the second bit low
   }
   
   if (gpio == cfg.Z_ENCODER_A)
   {
      if (level == 1)
      zEnc.currState |= 0b01; // make the first bit high
      
      if (level == 0)
      zEnc.currState &= 0b10; // make the first bit low
   }
   if (gpio == cfg.Z_ENCODER_B)
   {
      if (level == 1)
      zEnc.currState |= 0b10; // make the second bit high
      
      if (level == 0)
      zEnc.currState &= 0b01; // make the second bit low
   }
   
   
   if ((gpio == cfg.X_ENCODER_A) || (gpio == cfg.X_ENCODER_B))
   {
      if (cfg.X_FLIP == TRUE)
      getFlippedEncoderLoc(&xEnc);
      else
      getEncoderLoc(&xEnc);
   }
   
   if ((gpio == cfg.Y_ENCODER_A) || (gpio == cfg.Y_ENCODER_B))
   {
      if (cfg.Y_FLIP == TRUE)
      getFlippedEncoderLoc(&yEnc);
      else
      getEncoderLoc(&yEnc);
   }
   
   if ((gpio == cfg.Z_ENCODER_A) || (gpio == cfg.Z_ENCODER_B))
   {
      if (cfg.Z_FLIP == TRUE)
      getFlippedEncoderLoc(&zEnc);
      else
      getEncoderLoc(&zEnc);
   }
   
   
   if (searchForIndexPulse)
   {
      if (gpio == cfg.X_ENCODER_I)
      xEncIndex.state = level;
      
      if (gpio == cfg.Y_ENCODER_I)
      yEncIndex.state = level;
      
      if (gpio == cfg.Z_ENCODER_I)
      zEncIndex.state = level;
      
      
      if ((gpio == cfg.X_ENCODER_A) || (gpio == cfg.X_ENCODER_B) || (gpio == cfg.X_ENCODER_I))
      {
         if (getIndexOffsetOnly)
         {
            getIndexOffset(&xEnc, &xEncIndex);
            
            if (xEncIndex.offsetFound)
            {
               cfg.X_INDEX_OFFSET = xEncIndex.offset / cfg.X_MECHANICAL_DEGREES_PER_MM;
               xLocationConfirmed = TRUE;
               returnToPreservedLoc = TRUE;
               searchForIndexPulse = FALSE;
            }
         }
         else
         {
            xEnc.loc += cfg.X_INDEX_OFFSET * cfg.X_MECHANICAL_DEGREES_PER_MM;
            
            getIndexOffset(&xEnc, &xEncIndex);
            
            if ((xLocationConfirmed == FALSE) && (xEncIndex.offsetFound))
            {
               addIndexOffsetToLoc(&xEnc, &xEncIndex);
               xLocationConfirmed = TRUE;
               returnToPreservedLoc = TRUE;
               searchForIndexPulse = FALSE;
            }
            
            xEnc.loc -= cfg.X_INDEX_OFFSET * cfg.X_MECHANICAL_DEGREES_PER_MM;
         }
      }
      
      if ((gpio == cfg.Y_ENCODER_A) || (gpio == cfg.Y_ENCODER_B) || (gpio == cfg.Y_ENCODER_I))
      {
         if (getIndexOffsetOnly)
         {
            getIndexOffset(&yEnc, &yEncIndex);
            
            if (yEncIndex.offsetFound)
            {
               cfg.Y_INDEX_OFFSET = yEncIndex.offset / cfg.Y_MECHANICAL_DEGREES_PER_MM;
               yLocationConfirmed = TRUE;
               returnToPreservedLoc = TRUE;
               searchForIndexPulse = FALSE;
            }
         }
         else
         {
            yEnc.loc += cfg.Y_INDEX_OFFSET * cfg.Y_MECHANICAL_DEGREES_PER_MM;
            
            getIndexOffset(&yEnc, &yEncIndex);
            
            if ((yLocationConfirmed == FALSE) && (yEncIndex.offsetFound))
            {
               addIndexOffsetToLoc(&yEnc, &yEncIndex);
               yLocationConfirmed = TRUE;
               returnToPreservedLoc = TRUE;
               searchForIndexPulse = FALSE;
            }
            
            yEnc.loc -= cfg.Y_INDEX_OFFSET * cfg.Y_MECHANICAL_DEGREES_PER_MM;
         }
      }
      
      if ((gpio == cfg.Z_ENCODER_A) || (gpio == cfg.Z_ENCODER_B) || (gpio == cfg.Z_ENCODER_I))
      {
         if (getIndexOffsetOnly)
         {
            getIndexOffset(&zEnc, &zEncIndex);
            
            if (zEncIndex.offsetFound)
            {
               cfg.Z_INDEX_OFFSET = zEncIndex.offset / cfg.Z_MECHANICAL_DEGREES_PER_MM;
               zLocationConfirmed = TRUE;
               returnToPreservedLoc = TRUE;
               searchForIndexPulse = FALSE;
            }
         }
         else
         {
            zEnc.loc += cfg.Z_INDEX_OFFSET * cfg.Z_MECHANICAL_DEGREES_PER_MM;
            
            getIndexOffset(&zEnc, &zEncIndex);
            
            if ((zLocationConfirmed == FALSE) && (zEncIndex.offsetFound))
            {
               addIndexOffsetToLoc(&zEnc, &zEncIndex);
               zLocationConfirmed = TRUE;
               returnToPreservedLoc = TRUE;
               searchForIndexPulse = FALSE;
            }
            
            zEnc.loc -= cfg.Z_INDEX_OFFSET * cfg.Z_MECHANICAL_DEGREES_PER_MM;
         }
      }
   }
}


void *printDisplay()
{
   int i;
   int gInc = 0;
   int newLineLoc = UNDEFINED;
   int displaySearchingForIndex;
   int gfileIncBasePrior = 0;
   int noWorkingDir = TRUE;
   int machineStatePrior = -1;
   
   
   while (TRUE)
   {
      /////////////////////////////////////////////// MAIN_MENU_MODE
      if (machineState == MAIN_MENU_MODE)
      {
         if (machineState != machineStatePrior)
         {
            printf("----------------------------------------------------\n\r");
            printf("- MAIN MENU -\n\r");
            printf("Press [1] to align/configure machine\n\r");
            printf("Press [2] to move/jog machine\n\r");
            
            if (strlen(workingDirectory) > 38)
            printf("Press [3] to select file [...%s]\n\r", workingDirDisplay);
            else
            printf("Press [3] to select file [%s]\n\r", workingDirDisplay);
            
            if (gfileSelectNum != -1)
            printf("Press [4] to load and execute g-code [%s]\n\r", gCodeFile);
            else
            printf("Press [4] to load and execute g-code []\n\r");
            
            printf("Press [Esc] to exit\n\r");
            printf("----------------------------------------------------\n\r");
            printf("\n\r");
         }
      }
      
      
      
      /////////////////////////////////////////////// CONFIGURE_MODE
      if (machineState == CONFIGURE_MODE)
      {
         if (machineState != machineStatePrior)
         {
            printf("----------------------------------------------------\n\r");
            printf("- CONFIGURE -\n\r");
            printf("Press [1] for quick index pulse auto alignment\n\r");
            printf("Press [2] to define limits for each axis\n\r");
            printf("Press [3] to get the index offset for each axis\n\r");
            printf("Press [Esc] to exit to main menu\n\r");
            printf("\n\r");
            printf("Variables are saved to cnc.conf after program exit.\n\r");
            printf("\n\r");
            printf("Current Axes Limits\n\r");
            printf("X Axis - Min:%f Max:%f\n\r", cfg.X_MIN_LIMIT, cfg.X_MAX_LIMIT);
            printf("Y Axis - Min:%f Max:%f\n\r", cfg.Y_MIN_LIMIT, cfg.Y_MAX_LIMIT);
            printf("Z Axis - Min:%f Max:%f\n\r", cfg.Z_MIN_LIMIT, cfg.Z_MAX_LIMIT);
            
            if ((xLocationConfirmed) && (yLocationConfirmed) && (zLocationConfirmed))
            printf("\n\rLocation confirmed: TRUE\n\r\n\r");
            else
            printf("\n\rLocation confirmed: FALSE\n\r\n\r");
            
            printf("----------------------------------------------------\n\r");
            printf("\n\r");
         }
      }
      
      if (machineState == X_INDEX_PULSE_SEARCH_MODE)
      {
         if (getIndexOffsetOnly)
         {
            if (machineState != machineStatePrior)
            {
               printf("----------------------------------------------------\n\r");
               printf("Check to make sure the x axis is at location %f\n\r",
                      xEnc.loc / cfg.X_MECHANICAL_DEGREES_PER_MM);
               printf("\n\r");
               printf("The axis limits and jog distance must be configured first.\n\r");
               printf("\n\r");
               printf("Continue if the location is correct and the machine\n\r");
               printf("will move the x axis to search for the index pulse.\n\r");
               printf("Press [Spacebar] to continue...\n\r");
               printf("\n\r");
               printf("Press [Esc] to exit to main menu\n\r");
               printf("\n\r");
               
               displaySearchingForIndex = TRUE;
            }
            
            if (searchForIndexPulse)
            if (displaySearchingForIndex)
            {
               printf("Searching for the x axis index...\n\r\n\r");
               displaySearchingForIndex = FALSE;
            }
         }
         else
         {
            if (machineState != machineStatePrior)
            {
               printf("----------------------------------------------------\n\r");
               printf("Check to make sure the x axis is at location %f\n\r",
                      xEnc.loc / cfg.X_MECHANICAL_DEGREES_PER_MM);
               printf("\n\r");
               printf("The axis limits, jog distance, and index offset must be configured first.\n\r");
               printf("\n\r");
               printf("Continue if the location is correct and the machine\n\r");
               printf("will move the x axis to search for the index pulse.\n\r");
               printf("Press [Spacebar] to continue...\n\r");
               printf("\n\r");
               printf("Press [Esc] to exit to main menu\n\r");
               printf("\n\r");
               
               displaySearchingForIndex = TRUE;
            }
            
            if (searchForIndexPulse)
            if (displaySearchingForIndex)
            {
               printf("Searching for the x axis index...\n\r\n\r");
               displaySearchingForIndex = FALSE;
            }
         }
      }
      
      if (machineState == Y_INDEX_PULSE_SEARCH_MODE)
      {
         if (getIndexOffsetOnly)
         {
            if (machineState != machineStatePrior)
            {
               printf("----------------------------------------------------\n\r");
               printf("Check to make sure the y axis is at location %f\n\r",
                      yEnc.loc / cfg.Y_MECHANICAL_DEGREES_PER_MM);
               printf("\n\r");
               printf("The axis limits and jog distance must be configured first.\n\r");
               printf("\n\r");
               printf("Continue if the location is correct and the machine\n\r");
               printf("will move the y axis to search for the index pulse.\n\r");
               printf("Press [Spacebar] to continue...\n\r");
               printf("\n\r");
               printf("Press [Esc] to exit to main menu\n\r");
               printf("\n\r");
               
               displaySearchingForIndex = TRUE;
            }
            
            if (searchForIndexPulse)
            if (displaySearchingForIndex)
            {
               printf("Searching for the y axis index...\n\r\n\r");
               displaySearchingForIndex = FALSE;
            }
         }
         else
         {
            if (machineState != machineStatePrior)
            {
               printf("----------------------------------------------------\n\r");
               printf("Check to make sure the y axis is at location %f\n\r",
                      yEnc.loc / cfg.Y_MECHANICAL_DEGREES_PER_MM);
               printf("\n\r");
               printf("The axis limits, jog distance, and index offset must be configured first.\n\r");
               printf("\n\r");
               printf("Continue if the location is correct and the machine\n\r");
               printf("will move the y axis to search for the index pulse.\n\r");
               printf("Press [Spacebar] to continue...\n\r");
               printf("\n\r");
               printf("Press [Esc] to exit to main menu\n\r");
               printf("\n\r");
               
               displaySearchingForIndex = TRUE;
            }
            
            if (searchForIndexPulse)
            if (displaySearchingForIndex)
            {
               printf("Searching for the y axis index...\n\r\n\r");
               displaySearchingForIndex = FALSE;
            }
         }
      }
      
      if (machineState == Z_INDEX_PULSE_SEARCH_MODE)
      {
         if (getIndexOffsetOnly)
         {
            if (machineState != machineStatePrior)
            {
               printf("----------------------------------------------------\n\r");
               printf("Check to make sure the z axis is at location %f\n\r",
                      zEnc.loc / cfg.Z_MECHANICAL_DEGREES_PER_MM);
               printf("\n\r");
               printf("The axis limits and jog distance must be configured first.\n\r");
               printf("\n\r");
               printf("Continue if the location is correct and the machine\n\r");
               printf("will move the z axis to search for the index pulse.\n\r");
               printf("Press [Spacebar] to continue...\n\r");
               printf("\n\r");
               printf("Press [Esc] to exit to main menu\n\r");
               printf("\n\r");
               
               displaySearchingForIndex = TRUE;
            }
            
            if (searchForIndexPulse)
            if (displaySearchingForIndex)
            {
               printf("Searching for the z axis index...\n\r\n\r");
               displaySearchingForIndex = FALSE;
            }
         }
         else
         {
            if (machineState != machineStatePrior)
            {
               printf("----------------------------------------------------\n\r");
               printf("Check to make sure the z axis is at location %f\n\r",
                      zEnc.loc / cfg.Z_MECHANICAL_DEGREES_PER_MM);
               printf("\n\r");
               printf("The axis limits, jog distance, and index offset must be configured first.\n\r");
               printf("\n\r");
               printf("Continue if the location is correct and the machine\n\r");
               printf("will move the z axis to search for the index pulse.\n\r");
               printf("Press [Spacebar] to continue...\n\r");
               printf("\n\r");
               printf("Press [Esc] to exit to main menu\n\r");
               printf("\n\r");
               
               displaySearchingForIndex = TRUE;
            }
            
            if (searchForIndexPulse)
            if (displaySearchingForIndex)
            {
               printf("Searching for the z axis index...\n\r\n\r");
               displaySearchingForIndex = FALSE;
            }
         }
      }
      
      
      if (machineState == DEFINE_RANGES_MODE)
      {
         if (machineState != machineStatePrior)
         {
            printf("Move each axis as far as possible in both directions.\n\r\n\r");
            printf("Press [1] for \"manual mode\" - Motors will be disabled\n\r");
            printf("Press [2] for \"jog mode\" - Motors will be enabled\n\r");
            printf("Controls: A=-X D=+X S=-Y W=+Y Z=-Z Q=+Z\n\r");
            printf("Press [Spacebar] to save.\n\r");
            printf("\n\r");
            printf("Press [Esc] to exit to main menu\n\r");
            printf("\n\r");
            
            xMinRange = DBL_MAX; yMinRange = DBL_MAX; zMinRange = DBL_MAX;
            xMaxRange = DBL_MIN; yMaxRange = DBL_MIN; zMaxRange = DBL_MIN;
            
            machineState = DEFINE_RANGES_MANUAL_MODE;
         }
      }
      
      if (machineState == DEFINE_RANGES_MANUAL_MODE)
      {
         if (machineState != machineStatePrior)
         {
            printf("Motors are disabled\n\r");
         }
      }
      
      if (machineState == DEFINE_RANGES_JOG_MODE)
      {
         if (machineState != machineStatePrior)
         {
            printf("Motors are enabled\n\r");
         }
      }
      
      
      
      /////////////////////////////////////////////// MOVE_JOG_MODE
      if (machineState == MOVE_JOG_MODE)
      {
         if (machineState != machineStatePrior)
         {
            printf("- MOVE/JOG MODE -\n\r");
            printf("Press [1] for \"manual mode\" - Motors will be disabled\n\r");
            printf("Press [2] for \"jog mode\" - Motors will be enabled\n\r");
            printf("\n\rThe axis limits must be configured first to use jog mode.\n\r");
         }
      }
      
      if (machineState == MANUAL_MODE)
      {
         printf("X%f Y%f Z%f [Esc]\n\r", xEnc.loc / cfg.X_MECHANICAL_DEGREES_PER_MM,
                                         yEnc.loc / cfg.X_MECHANICAL_DEGREES_PER_MM,
                                         zEnc.loc / cfg.X_MECHANICAL_DEGREES_PER_MM);
      }
      
      if (machineState == JOG_MODE)
      {
         printf("X%f Y%f Z%f A=-X D=+X S=-Y W=+Y Z=-Z Q=+Z [Esc]\n\r",
                 xEnc.loc / cfg.X_MECHANICAL_DEGREES_PER_MM,
                 yEnc.loc / cfg.X_MECHANICAL_DEGREES_PER_MM,
                 zEnc.loc / cfg.X_MECHANICAL_DEGREES_PER_MM);
      }
      
      
      
      /////////////////////////////////////////////// LOAD_GCODE_MODE
      if (machineState == LOAD_GCODE_MODE)
      {
         if (machineState != machineStatePrior)
         {
            gfileIncBase = 0;
            gfileCnt = 0;
            freeFileList();
            noWorkingDir = getFileList(workingDirectory, "ngc", &gfileCnt, &gfiles);
         }
         
         if ((machineState != machineStatePrior) || (gfileIncBase != gfileIncBasePrior))
         {
            printf("----------------------------------------------------\n\r");
            printf("- G-CODE FILE CHOOSER -\n\r");
            
            if (noWorkingDir)
            {
               printf("\n\rCould not open directory \"%s\"\n\r", workingDirectory);
               printf("\n\rPress [Esc] to exit to main menu\n\r");
            }
            else
            {
               if (gfileIncBase >= gfileCnt)
               gfileIncBase = 0;
               
               if (gfileCnt == 0)
               printf("There are no \".ngc\" files found.\n\r");
               else
               for (i=gfileIncBase; i < gfileIncBase+10; i++)
               {
                  printf("[%i] %s\n\r", (i+1)%10, gfiles[i]);
                  
                  if (i+1 == gfileCnt)
                  break;
               }
               
               gfileIncBasePrior = gfileIncBase;
               
               if (gfileCnt != 0)
               printf("Press [Spacebar] to continue...\n\r");
               
               printf("\n\r");
               printf("Choose a file by number.\n\r");
               printf("Press [Esc] to exit to main menu\n\r");
            }
            printf("----------------------------------------------------\n\r");
            printf("\n\r");
         }
      }
      
      
      
      /////////////////////////////////////////////// GCODE_EXE_MODE GCODE_WAIT_MODE
      if ((machineState == GCODE_EXE_MODE) || (machineState == GCODE_WAIT_MODE))
      {
         if (machineStatePrior == MAIN_MENU_MODE)
         {
            printf("----------------------------------------------------\n\r");
            printf("- Execute G-CODE -\n\r");
            
            if ((xLocationConfirmed == FALSE) ||
                (yLocationConfirmed == FALSE) ||
                (zLocationConfirmed == FALSE))
            printf("WARNING - Location has not been confirmed.\n\r");
            
            if (gfileSelectNum == -1)
            printf("A g-code file has not been choosen.\n\r");
            
            if (gfileSelectNum != -1)
            printf("Press [Spacebar] to continue...\n\r");
            
            printf("\n\rPress [Esc] to exit to main menu\n\r\n\r");
         }
         
         
         if (gCodeRet != ERR_NO_FEED_RATE)
         if (gCodeRet != OUT_OF_BOUNDS)
         if (gInc != g.codeInc)
         {
            // find where the end of line will be for the g-code
            newLineLoc = UNDEFINED;
            for (i=gInc; i <= g.codeInc; i++)
            {
               if ((g.code[i+1] == 'G') ||
                   (g.code[i+1] == 'M') ||
                   (g.code[i+1] == '(') ||
                   (i+1 == g.codeCnt))
               {
                  newLineLoc = i;
                  break;
               }
            }
            
            // print currect g-code instruction
            if (newLineLoc != UNDEFINED)
            {
               for (i=gInc; i <= newLineLoc; i++)
               {
                  if (g.code[i] == '(')
                  printf("(%s)", g.comm[(int)g.value[i]]);
                  else
                  {
                     printf("%c", g.code[i]);
                     
                     if ((g.code[i] == 'G') || (g.code[i] == 'M'))
                     printf("%i", (int)g.value[i]);
                     else
                     if (g.value[i] == g.value[i])
                     printf("%f", g.value[i]);
                     
                     printf(" ");
                  }
               }
               
               gInc = i;
               printf("\n\r");
            }
         }
         
         if (displayGcodeRet)
         if (gCodeRet == ERR_NO_FEED_RATE)
         {
            printf("\n\rG-code error, no feed rate.\n\r");
            printf("Press [Spacebar] to continue...\n\r");
            
            displayGcodeRet = FALSE;
         }
         
         if (displayGcodeRet)
         if (gCodeRet == OUT_OF_BOUNDS)
         {
            printf("\n\rError, out of bounds.\n\r");
            printf("Press [Spacebar] to continue...\n\r");
            
            displayGcodeRet = FALSE;
         }
         
         if (displayGcodeRet)
         if (gInc == g.codeInc)
         {
            if (gCodeRet == PROGRAM_PAUSE)
            printf("Press [Spacebar] to continue...\n\r");
            
            if (gCodeRet == PROGRAM_END)
            {
               printf("End of Program.\n\r");
               printf("Press [Spacebar] to continue...\n\r");
            }
            
            displayGcodeRet = FALSE;
         }
      }
      
      // reset g-code increment
      if ((machineState != GCODE_EXE_MODE) && (machineState != GCODE_WAIT_MODE))
      {
         gInc = 0;
         g.codeInc = 0;
         gCodeRet = 0;
      }
      
      
      
      /////////////////////////////////////////////// EXIT_PROGRAM
      if (machineState == EXIT_PROGRAM)
      {
         if (machineState != machineStatePrior)
         printf("Do you want to exit? Press [Y] for yes and [N] for no.\n\r\n\r");
      }
      
      
      // display encoder error if true
      if (xEnc.error == TRUE)
      printf("X axis encoder error\n\r");
      
      if (yEnc.error == TRUE)
      printf("Y axis encoder error\n\r");
      
      if (zEnc.error == TRUE)
      printf("Z axis encoder error\n\r");
      
      
      machineStatePrior = machineState;
      gpioDelay(2000);
   
      if (exitProg)
      break;
   }
   
   return NULL;
}


void *readKbd()
{
   int ch;
   
   while (TRUE)
   {
      ch = getchar();
      
      xJog = 0;
      yJog = 0;
      zJog = 0;
      
      /////////////////////////////////////////////// MAIN_MENU_MODE
      if (machineState == MAIN_MENU_MODE)
      {
         if (ch == 49) // 1
         {
            machineState = CONFIGURE_MODE;
            continue;
         }
         
         if (ch == 50) // 2
         {
            machineState = MOVE_JOG_MODE;
            continue;
         }
         
         if (ch == 51) // 3
         {
            machineState = LOAD_GCODE_MODE;
            continue;
         }
         
         if (ch == 52) // 4
         {
            motorCtrlReset();
            gCodeIsLoaded = FALSE;
            machineState = GCODE_WAIT_MODE;
            continue;
         }
      }
      
      
      
      /////////////////////////////////////////////// CONFIGURE_MODE
      if (machineState == CONFIGURE_MODE)
      {
         if (ch == 49) // 1
         {
            xLocationConfirmed = FALSE;
            yLocationConfirmed = FALSE;
            zLocationConfirmed = FALSE;
            
            initIndexAlignmentVars(&xEncIndex);
            initIndexAlignmentVars(&yEncIndex);
            initIndexAlignmentVars(&zEncIndex);
            
            getIndexOffsetOnly = FALSE;
            machineState = X_INDEX_PULSE_SEARCH_MODE;
            continue;
         }
         
         if (ch == 50) // 2
         {
            machineState = DEFINE_RANGES_MODE;
            continue;
         }
         
         if (ch == 51) // 3
         {
            xLocationConfirmed = FALSE;
            yLocationConfirmed = FALSE;
            zLocationConfirmed = FALSE;
            
            initIndexAlignmentVars(&xEncIndex);
            initIndexAlignmentVars(&yEncIndex);
            initIndexAlignmentVars(&zEncIndex);
            
            getIndexOffsetOnly = TRUE;
            machineState = X_INDEX_PULSE_SEARCH_MODE;
            continue;
         }
         
      }
      
      if (machineState == X_INDEX_PULSE_SEARCH_MODE)
      {
         if (ch == 32) // spacebar
         {
            searchForIndexPulse = TRUE;
            continue;
         }
      }
      
      if (machineState == Y_INDEX_PULSE_SEARCH_MODE)
      {
         if (ch == 32) // spacebar
         {
            searchForIndexPulse = TRUE;
            continue;
         }
      }
      
      if (machineState == Z_INDEX_PULSE_SEARCH_MODE)
      {
         if (ch == 32) // spacebar
         {
            searchForIndexPulse = TRUE;
            continue;
         }
      }
      
      
      if (machineState == DEFINE_RANGES_MANUAL_MODE)
      {
         if (ch == 49) // 1
         {
            machineState = DEFINE_RANGES_MANUAL_MODE;
            continue;
         }
         
         if (ch == 50) // 2
         {
            machineState = DEFINE_RANGES_JOG_MODE;
            continue;
         }
         
         if (ch == 32) // spacebar
         {
            if (xMaxRange / cfg.X_MECHANICAL_DEGREES_PER_MM > cfg.X_MAX_LIMIT)
            cfg.X_MAX_LIMIT = xMaxRange / cfg.X_MECHANICAL_DEGREES_PER_MM;
            
            if (xMinRange / cfg.X_MECHANICAL_DEGREES_PER_MM < cfg.X_MIN_LIMIT)
            cfg.X_MIN_LIMIT = xMinRange / cfg.X_MECHANICAL_DEGREES_PER_MM;
            
            if (yMaxRange / cfg.Y_MECHANICAL_DEGREES_PER_MM > cfg.Y_MAX_LIMIT)
            cfg.Y_MAX_LIMIT = yMaxRange / cfg.Y_MECHANICAL_DEGREES_PER_MM;
            
            if (yMinRange / cfg.Y_MECHANICAL_DEGREES_PER_MM < cfg.Y_MIN_LIMIT)
            cfg.Y_MIN_LIMIT = yMinRange / cfg.Y_MECHANICAL_DEGREES_PER_MM;
            
            if (zMaxRange / cfg.Z_MECHANICAL_DEGREES_PER_MM > cfg.Z_MAX_LIMIT)
            cfg.Z_MAX_LIMIT = zMaxRange / cfg.Z_MECHANICAL_DEGREES_PER_MM;
            
            if (zMinRange / cfg.Z_MECHANICAL_DEGREES_PER_MM < cfg.Z_MIN_LIMIT)
            cfg.Z_MIN_LIMIT = zMinRange / cfg.Z_MECHANICAL_DEGREES_PER_MM;
            
            machineState = CONFIGURE_MODE;
            continue;
         }
      }
      
      if (machineState == DEFINE_RANGES_JOG_MODE)
      {
         if (ch == 49) // 1
         {
            machineState = DEFINE_RANGES_MANUAL_MODE;
            continue;
         }
         
         if (ch == 50) // 2
         {
            machineState = DEFINE_RANGES_JOG_MODE;
            continue;
         }
         
         if (ch == 97) // A
         xJog = -1;
         if (ch == 100) // D
         xJog = 1;
         
         if (ch == 115) // S
         yJog = -1;
         if (ch == 119) // W
         yJog = 1;
         
         if (ch == 122) // Z
         zJog = -1;
         if (ch == 113) // Q
         zJog = 1;
         
         if (ch == 32) // spacebar
         {
            if (xMaxRange / cfg.X_MECHANICAL_DEGREES_PER_MM > cfg.X_MAX_LIMIT)
            cfg.X_MAX_LIMIT = xMaxRange / cfg.X_MECHANICAL_DEGREES_PER_MM;
            
            if (xMinRange / cfg.X_MECHANICAL_DEGREES_PER_MM < cfg.X_MIN_LIMIT)
            cfg.X_MIN_LIMIT = xMinRange / cfg.X_MECHANICAL_DEGREES_PER_MM;
            
            if (yMaxRange / cfg.Y_MECHANICAL_DEGREES_PER_MM > cfg.Y_MAX_LIMIT)
            cfg.Y_MAX_LIMIT = yMaxRange / cfg.Y_MECHANICAL_DEGREES_PER_MM;
            
            if (yMinRange / cfg.Y_MECHANICAL_DEGREES_PER_MM < cfg.Y_MIN_LIMIT)
            cfg.Y_MIN_LIMIT = yMinRange / cfg.Y_MECHANICAL_DEGREES_PER_MM;
            
            if (zMaxRange / cfg.Z_MECHANICAL_DEGREES_PER_MM > cfg.Z_MAX_LIMIT)
            cfg.Z_MAX_LIMIT = zMaxRange / cfg.Z_MECHANICAL_DEGREES_PER_MM;
            
            if (zMinRange / cfg.Z_MECHANICAL_DEGREES_PER_MM < cfg.Z_MIN_LIMIT)
            cfg.Z_MIN_LIMIT = zMinRange / cfg.Z_MECHANICAL_DEGREES_PER_MM;
            
            machineState = CONFIGURE_MODE;
            continue;
         }
      }
      
      
      
      /////////////////////////////////////////////// MOVE_JOG_MODE
      if (machineState == MOVE_JOG_MODE)
      {
         if (ch == 49) // 1
         {
            machineState = MANUAL_MODE;
            continue;
         }
         
         if (ch == 50) // 2
         {
            motorCtrlReset();
            machineState = JOG_MODE;
            continue;
         }
      }
      
      if (machineState == MANUAL_MODE)
      {
         if (ch == 50) // 2
         {
            motorCtrlReset();
            machineState = JOG_MODE;
            continue;
         }
      }
      
      if (machineState == JOG_MODE)
      {
         if (ch == 49) // 1
         {
            machineState = MANUAL_MODE;
            continue;
         }
         
         if (ch == 97) // A
         xJog = -1;
         if (ch == 100) // D
         xJog = 1;
         
         if (ch == 115) // S
         yJog = -1;
         if (ch == 119) // W
         yJog = 1;
         
         if (ch == 122) // Z
         zJog = -1;
         if (ch == 113) // Q
         zJog = 1;
      }
      
      
      
      /////////////////////////////////////////////// LOAD_GCODE_MODE
      if (machineState == LOAD_GCODE_MODE)
      {
         if (ch == 32) // spacebar
         gfileIncBase += 10;
         
         if ((ch >= 48) && (ch < 58)) // 0 - 9
         {
            if (ch == 48) // 0
            gfileSelectNum = gfileIncBase + 10 - 1;
            else
            gfileSelectNum = gfileIncBase + (ch-48) - 1;
            
            if (gfileSelectNum >= gfileCnt)
            gfileSelectNum = -1;
            
            if (gfileSelectNum != -1)
            {
               gCodeFile = gfiles[gfileSelectNum];
               gCodeIsLoaded = FALSE;
               machineState = MAIN_MENU_MODE;
            }
         }
      }
      
      
      
      /////////////////////////////////////////////// GCODE_EXE_MODE GCODE_WAIT_MODE
      if (machineState == GCODE_WAIT_MODE)
      {
         if (ch == 32) // spacebar
         {
            if ((gCodeRet == PROGRAM_END) ||
                (gCodeRet == ERR_NO_FEED_RATE) ||
                (gCodeRet == OUT_OF_BOUNDS))
            {
               machineState = MAIN_MENU_MODE;
               continue;
            }
            
            if ((gfileSelectNum != -1) || (gCodeRet == PROGRAM_PAUSE))
            {
               machineState = GCODE_EXE_MODE;
               continue;
            }
         }
      }
      
      
      
      if (ch == 27) // Esc
      {
         if (machineState == MAIN_MENU_MODE)
         machineState = EXIT_PROGRAM;
         else
         machineState = MAIN_MENU_MODE;
      }
      
      
      
      /////////////////////////////////////////////// EXIT_PROGRAM
      if (machineState == EXIT_PROGRAM)
      {
         if (ch == 110) // N
         machineState = MAIN_MENU_MODE;
         
         // exit
         if (ch == 121) // Y
         {
            exitProg = TRUE;
            break;
         }
      }
   }
   
   return NULL;
}


int main(int argCnt, char **args)
{
   pthread_t keyboardThreadID;
   pthread_t printDisplayThreadID;
   int jogRet;
   float jogLength;
   char *gCodeFilePath;
   
   
   if (gpioInitialise() < 0)
   {
      printf("Pigpio initialisation failed\n");
      
      return 1;
   }
   
   // initialize ncurses
   initscr();
   nodelay(stdscr, TRUE);
   noecho();
   setvbuf(stdout, NULL, _IOLBF, 0);
   
   if (argCnt < 2)
   {
      printf("Usage:\n\r");
      printf("sudo ./qclcnc \"/optional/working/directory\"\n\r\n\r");
      
      workingDirectory = "./";
   }
   else
   workingDirectory = args[1];
   
   if (strlen(workingDirectory) > 38)
   workingDirDisplay = workingDirectory + (strlen(workingDirectory) - 38);
   else
   workingDirDisplay = workingDirectory;
   
   if (loadConfigFile("cnc.conf", &cfg))
   {
      printf("Could not load cnc.conf\n\r");
      
      endwin();
      return 1;
   }

   
   // set encoder pins
   gpioSetMode(cfg.X_ENCODER_A, PI_INPUT);
   gpioSetPullUpDown(cfg.X_ENCODER_A, PI_PUD_DOWN);
   gpioSetMode(cfg.X_ENCODER_B, PI_INPUT);
   gpioSetPullUpDown(cfg.X_ENCODER_B, PI_PUD_DOWN);
   gpioSetMode(cfg.X_ENCODER_I, PI_INPUT);
   gpioSetPullUpDown(cfg.X_ENCODER_I, PI_PUD_DOWN);
   
   gpioSetMode(cfg.Y_ENCODER_A, PI_INPUT);
   gpioSetPullUpDown(cfg.Y_ENCODER_A, PI_PUD_DOWN);
   gpioSetMode(cfg.Y_ENCODER_B, PI_INPUT);
   gpioSetPullUpDown(cfg.Y_ENCODER_B, PI_PUD_DOWN);
   gpioSetMode(cfg.Y_ENCODER_I, PI_INPUT);
   gpioSetPullUpDown(cfg.Y_ENCODER_I, PI_PUD_DOWN);
   
   gpioSetMode(cfg.Z_ENCODER_A, PI_INPUT);
   gpioSetPullUpDown(cfg.Z_ENCODER_A, PI_PUD_DOWN);
   gpioSetMode(cfg.Z_ENCODER_B, PI_INPUT);
   gpioSetPullUpDown(cfg.Z_ENCODER_B, PI_PUD_DOWN);
   gpioSetMode(cfg.Z_ENCODER_I, PI_INPUT);
   gpioSetPullUpDown(cfg.Z_ENCODER_I, PI_PUD_DOWN);

   // set motor pins
   gpioSetMode(cfg.X_MOTOR_ENABLE, PI_OUTPUT);
   gpioSetMode(cfg.X_MOTOR_DIRECT, PI_OUTPUT);
   gpioSetMode(cfg.X_MOTOR_STEP, PI_OUTPUT);
   
   gpioSetMode(cfg.Y_MOTOR_ENABLE, PI_OUTPUT);
   gpioSetMode(cfg.Y_MOTOR_DIRECT, PI_OUTPUT);
   gpioSetMode(cfg.Y_MOTOR_STEP, PI_OUTPUT);
   
   gpioSetMode(cfg.Z_MOTOR_ENABLE, PI_OUTPUT);
   gpioSetMode(cfg.Z_MOTOR_DIRECT, PI_OUTPUT);
   gpioSetMode(cfg.Z_MOTOR_STEP, PI_OUTPUT);
   
   // initialize X axis encoder
   xEnc.loc = cfg.X_LOC * cfg.X_MECHANICAL_DEGREES_PER_MM;
   xEnc.currState = gpioRead(cfg.X_ENCODER_A) | (gpioRead(cfg.X_ENCODER_B) << 1);
   xEnc.priorState = xEnc.currState;
   xEnc.error = FALSE;
   
   // initialize Y axis encoder
   yEnc.loc = cfg.Y_LOC * cfg.Y_MECHANICAL_DEGREES_PER_MM;
   yEnc.currState = gpioRead(cfg.Y_ENCODER_A) | (gpioRead(cfg.Y_ENCODER_B) << 1);
   yEnc.priorState = yEnc.currState;
   yEnc.error = FALSE;
   
   // initialize Z axis encoder
   zEnc.loc = cfg.Z_LOC * cfg.Z_MECHANICAL_DEGREES_PER_MM;
   zEnc.currState = gpioRead(cfg.Z_ENCODER_A) | (gpioRead(cfg.Z_ENCODER_B) << 1);
   zEnc.priorState = zEnc.currState;
   zEnc.error = FALSE;
   
   initIndexAlignmentVars(&xEncIndex);
   initIndexAlignmentVars(&yEncIndex);
   initIndexAlignmentVars(&zEncIndex);
   
   
   // create gpio alerts
   gpioSetAlertFunc(cfg.X_ENCODER_A, encEventCallBack);
   gpioSetAlertFunc(cfg.X_ENCODER_B, encEventCallBack);
   gpioSetAlertFunc(cfg.X_ENCODER_I, encEventCallBack);
   
   gpioSetAlertFunc(cfg.Y_ENCODER_A, encEventCallBack);
   gpioSetAlertFunc(cfg.Y_ENCODER_B, encEventCallBack);
   gpioSetAlertFunc(cfg.Y_ENCODER_I, encEventCallBack);
   
   gpioSetAlertFunc(cfg.Z_ENCODER_A, encEventCallBack);
   gpioSetAlertFunc(cfg.Z_ENCODER_B, encEventCallBack);
   gpioSetAlertFunc(cfg.Z_ENCODER_I, encEventCallBack);


   gpioWrite(cfg.X_MOTOR_ENABLE, PI_OFF);
   gpioWrite(cfg.Y_MOTOR_ENABLE, PI_OFF);
   gpioWrite(cfg.Z_MOTOR_ENABLE, PI_OFF);
   
   
   // create threads
   pthread_create(&keyboardThreadID, NULL, &readKbd, NULL);
   pthread_create(&printDisplayThreadID, NULL, &printDisplay, NULL);

   
   while (TRUE)
   {
      if (machineState == MAIN_MENU_MODE)
      {
         gpioWrite(cfg.X_MOTOR_ENABLE, PI_OFF);
         gpioWrite(cfg.Y_MOTOR_ENABLE, PI_OFF);
         gpioWrite(cfg.Z_MOTOR_ENABLE, PI_OFF);
      }
      
      
      //////////////// find the index pulse location for each axis ////////////////
      if (machineState == X_INDEX_PULSE_SEARCH_MODE)
      {
         if (searchForIndexPulse)
         {
            if (preserveEncoderLocation == TRUE)
            {
               motorCtrlReset();
               preservedLoc = xEnc.loc / cfg.X_MECHANICAL_DEGREES_PER_MM;
               preserveEncoderLocation = FALSE;
            }
            
            if (searchDirection == POSITIVE)
            jogRet = jog(cfg.JOG_DISTANCE, 0, 0, cfg.POSITIONING_SPEED, xEnc, yEnc, zEnc, cfg, FALSE);
            
            if (searchDirection == NEGATIVE)
            jogRet = jog(-cfg.JOG_DISTANCE, 0, 0, cfg.POSITIONING_SPEED, xEnc, yEnc, zEnc, cfg, FALSE);
            
            if (jogRet == OUT_OF_BOUNDS)
            {
               if (searchDirection == POSITIVE)
               searchDirection = NEGATIVE;
               else
               searchDirection = POSITIVE;
            }
         }
         
         if (returnToPreservedLoc)
         {
            if (preserveEncoderLocation == FALSE)
            {
               motorCtrlReset();
               jogLength = preservedLoc - xEnc.loc / cfg.X_MECHANICAL_DEGREES_PER_MM;
               preserveEncoderLocation = TRUE; // reset
            }
            
            jogRet = jog(jogLength, 0, 0, cfg.POSITIONING_SPEED, xEnc, yEnc, zEnc, cfg, FALSE);
            
            if ((jogRet == END_OF_JOG) || (jogRet == OUT_OF_BOUNDS))
            {
               machineState = Y_INDEX_PULSE_SEARCH_MODE;
               returnToPreservedLoc = FALSE;
            }
         }
      }
      
      
      if (machineState == Y_INDEX_PULSE_SEARCH_MODE)
      {
         if (searchForIndexPulse)
         {
            if (preserveEncoderLocation == TRUE)
            {
               motorCtrlReset();
               preservedLoc = yEnc.loc / cfg.Y_MECHANICAL_DEGREES_PER_MM;
               preserveEncoderLocation = FALSE;
            }
            
            if (searchDirection == POSITIVE)
            jogRet = jog(0, cfg.JOG_DISTANCE, 0, cfg.POSITIONING_SPEED, xEnc, yEnc, zEnc, cfg, FALSE);
            
            if (searchDirection == NEGATIVE)
            jogRet = jog(0, -cfg.JOG_DISTANCE, 0, cfg.POSITIONING_SPEED, xEnc, yEnc, zEnc, cfg, FALSE);
            
            if (jogRet == OUT_OF_BOUNDS)
            {
               if (searchDirection == POSITIVE)
               searchDirection = NEGATIVE;
               else
               searchDirection = POSITIVE;
            }
         }
         
         if (returnToPreservedLoc)
         {
            if (preserveEncoderLocation == FALSE)
            {
               motorCtrlReset();
               jogLength = preservedLoc - yEnc.loc / cfg.Y_MECHANICAL_DEGREES_PER_MM;
               preserveEncoderLocation = TRUE; // reset
            }
            
            jogRet = jog(0, jogLength, 0, cfg.POSITIONING_SPEED, xEnc, yEnc, zEnc, cfg, FALSE);
            
            if ((jogRet == END_OF_JOG) || (jogRet == OUT_OF_BOUNDS))
            {
               machineState = Z_INDEX_PULSE_SEARCH_MODE;
               returnToPreservedLoc = FALSE;
            }
         }
      }
      
      
      if (machineState == Z_INDEX_PULSE_SEARCH_MODE)
      {
         if (searchForIndexPulse)
         {
            if (preserveEncoderLocation == TRUE)
            {
               motorCtrlReset();
               preservedLoc = zEnc.loc / cfg.Z_MECHANICAL_DEGREES_PER_MM;
               preserveEncoderLocation = FALSE;
            }
            
            if (searchDirection == POSITIVE)
            jogRet = jog(0, 0, cfg.JOG_DISTANCE, cfg.POSITIONING_SPEED, xEnc, yEnc, zEnc, cfg, FALSE);
            
            if (searchDirection == NEGATIVE)
            jogRet = jog(0, 0, -cfg.JOG_DISTANCE, cfg.POSITIONING_SPEED, xEnc, yEnc, zEnc, cfg, FALSE);
            
            if (jogRet == OUT_OF_BOUNDS)
            {
               if (searchDirection == POSITIVE)
               searchDirection = NEGATIVE;
               else
               searchDirection = POSITIVE;
            }
         }
         
         if (returnToPreservedLoc)
         {
            if (preserveEncoderLocation == FALSE)
            {
               motorCtrlReset();
               jogLength = preservedLoc - zEnc.loc / cfg.Z_MECHANICAL_DEGREES_PER_MM;
               preserveEncoderLocation = TRUE; // reset
            }
            
            jogRet = jog(0, 0, jogLength, cfg.POSITIONING_SPEED, xEnc, yEnc, zEnc, cfg, FALSE);
            
            if ((jogRet == END_OF_JOG) || (jogRet == OUT_OF_BOUNDS))
            {
               machineState = CONFIGURE_MODE;
               
               gpioWrite(cfg.X_MOTOR_ENABLE, PI_OFF);
               gpioWrite(cfg.Y_MOTOR_ENABLE, PI_OFF);
               gpioWrite(cfg.Z_MOTOR_ENABLE, PI_OFF);
               
               returnToPreservedLoc = FALSE;
            }
         }
      }
      //////////////// end of find the index pulse location for each axis ////////////////
      
      
      
      //////////////// define limits for each axis ////////////////
      if (machineState == DEFINE_RANGES_MANUAL_MODE)
      {
         gpioWrite(cfg.X_MOTOR_ENABLE, PI_OFF);
         gpioWrite(cfg.Y_MOTOR_ENABLE, PI_OFF);
         gpioWrite(cfg.Z_MOTOR_ENABLE, PI_OFF);
         
         if (xEnc.loc > xMaxRange)
         xMaxRange = xEnc.loc;
         if (xEnc.loc < xMinRange)
         xMinRange = xEnc.loc;
         
         if (yEnc.loc > yMaxRange)
         yMaxRange = yEnc.loc;
         if (yEnc.loc < yMinRange)
         yMinRange = yEnc.loc;
         
         if (zEnc.loc > zMaxRange)
         zMaxRange = zEnc.loc;
         if (zEnc.loc < zMinRange)
         zMinRange = zEnc.loc;
      }
      
      if (machineState == DEFINE_RANGES_JOG_MODE)
      {
         gpioWrite(cfg.X_MOTOR_ENABLE, PI_ON);
         gpioWrite(cfg.Y_MOTOR_ENABLE, PI_ON);
         gpioWrite(cfg.Z_MOTOR_ENABLE, PI_ON);
         
         jogRet = jog(xJog*cfg.JOG_DISTANCE, yJog*cfg.JOG_DISTANCE, zJog*cfg.JOG_DISTANCE,
                      cfg.JOG_SPEED, xEnc, yEnc, zEnc, cfg, TRUE);
         
         if (jogRet == END_OF_JOG)
         {
            xJog = 0;
            yJog = 0;
            zJog = 0;
         }
         
         if (xEnc.loc > xMaxRange)
         xMaxRange = xEnc.loc;
         if (xEnc.loc < xMinRange)
         xMinRange = xEnc.loc;
         
         if (yEnc.loc > yMaxRange)
         yMaxRange = yEnc.loc;
         if (yEnc.loc < yMinRange)
         yMinRange = yEnc.loc;
         
         if (zEnc.loc > zMaxRange)
         zMaxRange = zEnc.loc;
         if (zEnc.loc < zMinRange)
         zMinRange = zEnc.loc;
      }
      //////////////// end of define limits for each axis ////////////////
      
      
      
      //////////////// move or jog cnc ////////////////
      if (machineState == MANUAL_MODE)
      {
         gpioWrite(cfg.X_MOTOR_ENABLE, PI_OFF);
         gpioWrite(cfg.Y_MOTOR_ENABLE, PI_OFF);
         gpioWrite(cfg.Z_MOTOR_ENABLE, PI_OFF);
      }
      
      if (machineState == JOG_MODE)
      {
         jogRet = jog(xJog*cfg.JOG_DISTANCE, yJog*cfg.JOG_DISTANCE, zJog*cfg.JOG_DISTANCE,
                      cfg.JOG_SPEED, xEnc, yEnc, zEnc, cfg, FALSE);
         
         if (jogRet == END_OF_JOG)
         {
            xJog = 0;
            yJog = 0;
            zJog = 0;
         }
      }
      //////////////// end of move or jog cnc ////////////////
      
      
      
      //////////////// execute g-code ////////////////
      if (machineState == GCODE_WAIT_MODE)
      {
         if (gCodeIsLoaded == FALSE)
         {
            if (gfileSelectNum != -1)
            {
               gCodeFilePath = (char *)malloc(strlen(workingDirectory)+1+strlen(gCodeFile)+1);
               sprintf(gCodeFilePath, "%s/%s", workingDirectory, gCodeFile);
               
               if (parseGcode(gCodeFilePath, &g))
               gCodeIsLoaded = FALSE;
               else
               gCodeIsLoaded = TRUE;
               
               free(gCodeFilePath);
            }
         }
      }
      
      if (machineState == GCODE_EXE_MODE)
      {
         gCodeRet = executeGcode(&g, xEnc, yEnc, zEnc, cfg);
         
         if ((gCodeRet == ERR_NO_FEED_RATE) ||
             (gCodeRet == PROGRAM_PAUSE) ||
             (gCodeRet == PROGRAM_END) ||
             (gCodeRet == OUT_OF_BOUNDS))
         {
            machineState = GCODE_WAIT_MODE;
            displayGcodeRet = TRUE;
         }
      }
      //////////////// end of execute g-code ////////////////
      
      
      
      gpioDelay(200);
      
      // exit
      if (exitProg)
      break;
   }
   
   
   cfg.X_LOC = xEnc.loc / cfg.X_MECHANICAL_DEGREES_PER_MM;
   cfg.Y_LOC = yEnc.loc / cfg.Y_MECHANICAL_DEGREES_PER_MM;
   cfg.Z_LOC = zEnc.loc / cfg.Z_MECHANICAL_DEGREES_PER_MM;
   
   if (saveToConfigFile("./cnc.conf", cfg))
   printf("Could not save variables to cnc.conf\n\r");
   
   
   gpioWrite(cfg.X_MOTOR_ENABLE, PI_OFF);
   gpioWrite(cfg.Y_MOTOR_ENABLE, PI_OFF);
   gpioWrite(cfg.Z_MOTOR_ENABLE, PI_OFF);
   
   
   freeFileList();
   freeParsedGcode();
   
   // cancel gpio alerts
   gpioSetAlertFunc(cfg.X_ENCODER_A, 0);
   gpioSetAlertFunc(cfg.X_ENCODER_B, 0);
   gpioSetAlertFunc(cfg.X_ENCODER_I, 0);
   gpioSetAlertFunc(cfg.Y_ENCODER_A, 0);
   gpioSetAlertFunc(cfg.Y_ENCODER_B, 0);
   gpioSetAlertFunc(cfg.Y_ENCODER_I, 0);
   gpioSetAlertFunc(cfg.Z_ENCODER_A, 0);
   gpioSetAlertFunc(cfg.Z_ENCODER_B, 0);
   gpioSetAlertFunc(cfg.Z_ENCODER_I, 0);
   
   // block until all threads complete
   pthread_join(printDisplayThreadID, NULL);
   pthread_join(keyboardThreadID, NULL);

   endwin();
   return 0;
}
