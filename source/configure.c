#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "configure.h"
#include "encoder.h"
#include "varsFile.h"

#define TRUE 1
#define FALSE 0


int loadConfigFile(char *filename, struct config *cfg)
{
   int i;
   int varListCnt = 42;
   char **valueList;
   char *varList[42];
   varList[0] = "X_ENCODER_I";
   varList[1] = "X_ENCODER_A";
   varList[2] = "X_ENCODER_B";
   varList[3] = "Y_ENCODER_I";
   varList[4] = "Y_ENCODER_A";
   varList[5] = "Y_ENCODER_B";
   varList[6] = "Z_ENCODER_I";
   varList[7] = "Z_ENCODER_A";
   varList[8] = "Z_ENCODER_B";

   varList[9] = "X_MECHANICAL_DEGREES_PER_MM";
   varList[10] = "Y_MECHANICAL_DEGREES_PER_MM";
   varList[11] = "Z_MECHANICAL_DEGREES_PER_MM";

   varList[12] = "X_MOTOR_ENABLE";
   varList[13] = "X_MOTOR_DIRECT";
   varList[14] = "X_MOTOR_STEP";
   varList[15] = "Y_MOTOR_ENABLE";
   varList[16] = "Y_MOTOR_DIRECT";
   varList[17] = "Y_MOTOR_STEP";
   varList[18] = "Z_MOTOR_ENABLE";
   varList[19] = "Z_MOTOR_DIRECT";
   varList[20] = "Z_MOTOR_STEP";

   varList[21] = "X_PULSES_PER_MM";
   varList[22] = "Y_PULSES_PER_MM";
   varList[23] = "Z_PULSES_PER_MM";

   varList[24] = "POSITIONING_SPEED";
   varList[25] = "JOG_SPEED";
   varList[26] = "JOG_DISTANCE";

   varList[27] = "X_MAX_LIMIT";
   varList[28] = "X_MIN_LIMIT";
   varList[29] = "Y_MAX_LIMIT";
   varList[30] = "Y_MIN_LIMIT";
   varList[31] = "Z_MAX_LIMIT";
   varList[32] = "Z_MIN_LIMIT";

   varList[33] = "X_LOC";
   varList[34] = "Y_LOC";
   varList[35] = "Z_LOC";

   varList[36] = "X_INDEX_OFFSET";
   varList[37] = "Y_INDEX_OFFSET";
   varList[38] = "Z_INDEX_OFFSET";
   
   varList[39] = "X_FLIP";
   varList[40] = "Y_FLIP";
   varList[41] = "Z_FLIP";
   
   if (readVarsFile(filename, (char **)varList, varListCnt, &valueList))
   return TRUE;
   
   for (i=0; i <= 26; i++)
   {
      if (strcmp(valueList[i], "") == 0)
      {
         printf("%s=?\n\rNo value for %s\n\r", varList[i], varList[i]);
         return TRUE;
      }
   }
   
   cfg->X_ENCODER_I = atoi(valueList[0]);
   cfg->X_ENCODER_A = atoi(valueList[1]);
   cfg->X_ENCODER_B = atoi(valueList[2]);
   cfg->Y_ENCODER_I = atoi(valueList[3]);
   cfg->Y_ENCODER_A = atoi(valueList[4]);
   cfg->Y_ENCODER_B = atoi(valueList[5]);
   cfg->Z_ENCODER_I = atoi(valueList[6]);
   cfg->Z_ENCODER_A = atoi(valueList[7]);
   cfg->Z_ENCODER_B = atoi(valueList[8]);

   cfg->X_MECHANICAL_DEGREES_PER_MM = atof(valueList[9]);
   cfg->Y_MECHANICAL_DEGREES_PER_MM = atof(valueList[10]);
   cfg->Z_MECHANICAL_DEGREES_PER_MM = atof(valueList[11]);

   cfg->X_MOTOR_ENABLE = atoi(valueList[12]);
   cfg->X_MOTOR_DIRECT = atoi(valueList[13]);
   cfg->X_MOTOR_STEP = atoi(valueList[14]);
   cfg->Y_MOTOR_ENABLE = atoi(valueList[15]);
   cfg->Y_MOTOR_DIRECT = atoi(valueList[16]);
   cfg->Y_MOTOR_STEP = atoi(valueList[17]);
   cfg->Z_MOTOR_ENABLE = atoi(valueList[18]);
   cfg->Z_MOTOR_DIRECT = atoi(valueList[19]);
   cfg->Z_MOTOR_STEP = atoi(valueList[20]);

   cfg->X_PULSES_PER_MM = atof(valueList[21]);
   cfg->Y_PULSES_PER_MM = atof(valueList[22]);
   cfg->Z_PULSES_PER_MM = atof(valueList[23]);

   cfg->POSITIONING_SPEED = atof(valueList[24]);
   cfg->JOG_SPEED = atof(valueList[25]);
   cfg->JOG_DISTANCE = atof(valueList[26]);

   cfg->X_MAX_LIMIT = atof(valueList[27]);
   cfg->X_MIN_LIMIT = atof(valueList[28]);
   cfg->Y_MAX_LIMIT = atof(valueList[29]);
   cfg->Y_MIN_LIMIT = atof(valueList[30]);
   cfg->Z_MAX_LIMIT = atof(valueList[31]);
   cfg->Z_MIN_LIMIT = atof(valueList[32]);

   cfg->X_LOC = atof(valueList[33]);
   cfg->Y_LOC = atof(valueList[34]);
   cfg->Z_LOC = atof(valueList[35]);

   cfg->X_INDEX_OFFSET = atof(valueList[36]);
   cfg->Y_INDEX_OFFSET = atof(valueList[37]);
   cfg->Z_INDEX_OFFSET = atof(valueList[38]);
   
   cfg->X_FLIP = atoi(valueList[39]);
   cfg->Y_FLIP = atoi(valueList[40]);
   cfg->Z_FLIP = atoi(valueList[41]);
   
   freeValueList();
   return FALSE;
}


int saveToConfigFile(char *filename, struct config cfg)
{
   int i;
   int ret;
   int varListCnt = 12;
   char *valueList[12];
   char *varList[12];
   varList[0] = "X_MAX_LIMIT";
   varList[1] = "X_MIN_LIMIT";
   varList[2] = "Y_MAX_LIMIT";
   varList[3] = "Y_MIN_LIMIT";
   varList[4] = "Z_MAX_LIMIT";
   varList[5] = "Z_MIN_LIMIT";

   varList[6] = "X_LOC";
   varList[7] = "Y_LOC";
   varList[8] = "Z_LOC";

   varList[9] = "X_INDEX_OFFSET";
   varList[10] = "Y_INDEX_OFFSET";
   varList[11] = "Z_INDEX_OFFSET";

   
   for (i = 0; i < varListCnt; i++)
   {
      valueList[i] = (char *)malloc(32 * sizeof(char));
      valueList[i][0] = 0;
   }
   
   /*
   sprintf(valueList[0], "%i", cfg.X_ENCODER_I);
   sprintf(valueList[1], "%i", cfg.X_ENCODER_A);
   sprintf(valueList[2], "%i", cfg.X_ENCODER_B);
   sprintf(valueList[3], "%i", cfg.Y_ENCODER_I);
   sprintf(valueList[4], "%i", cfg.Y_ENCODER_A);
   sprintf(valueList[5], "%i", cfg.Y_ENCODER_B);
   sprintf(valueList[6], "%i", cfg.Z_ENCODER_I);
   sprintf(valueList[7], "%i", cfg.Z_ENCODER_A);
   sprintf(valueList[8], "%i", cfg.Z_ENCODER_B);
   
   sprintf(valueList[9], "%f", cfg.X_MECHANICAL_DEGREES_PER_MM);
   sprintf(valueList[10], "%f", cfg.Y_MECHANICAL_DEGREES_PER_MM);
   sprintf(valueList[11], "%f", cfg.Z_MECHANICAL_DEGREES_PER_MM);
   
   sprintf(valueList[12], "%i", cfg.X_MOTOR_ENABLE);
   sprintf(valueList[13], "%i", cfg.X_MOTOR_DIRECT);
   sprintf(valueList[14], "%i", cfg.X_MOTOR_STEP);
   sprintf(valueList[15], "%i", cfg.Y_MOTOR_ENABLE);
   sprintf(valueList[16], "%i", cfg.Y_MOTOR_DIRECT);
   sprintf(valueList[17], "%i", cfg.Y_MOTOR_STEP);
   sprintf(valueList[18], "%i", cfg.Z_MOTOR_ENABLE);
   sprintf(valueList[19], "%i", cfg.Z_MOTOR_DIRECT);
   sprintf(valueList[20], "%i", cfg.Z_MOTOR_STEP);
   
   sprintf(valueList[21], "%f", cfg.X_PULSES_PER_MM);
   sprintf(valueList[22], "%f", cfg.Y_PULSES_PER_MM);
   sprintf(valueList[23], "%f", cfg.Z_PULSES_PER_MM);
   
   sprintf(valueList[24], "%f", cfg.POSITIONING_SPEED);
   sprintf(valueList[25], "%f", cfg.JOG_SPEED);
   sprintf(valueList[26], "%f", cfg.JOG_DISTANCE);
   
   sprintf(valueList[27], "%f", cfg.X_MAX_LIMIT);
   sprintf(valueList[28], "%f", cfg.X_MIN_LIMIT);
   sprintf(valueList[29], "%f", cfg.Y_MAX_LIMIT);
   sprintf(valueList[30], "%f", cfg.Y_MIN_LIMIT);
   sprintf(valueList[31], "%f", cfg.Z_MAX_LIMIT);
   sprintf(valueList[32], "%f", cfg.Z_MIN_LIMIT);
   
   sprintf(valueList[33], "%f", cfg.X_LOC);
   sprintf(valueList[34], "%f", cfg.Y_LOC);
   sprintf(valueList[35], "%f", cfg.Z_LOC);
   
   sprintf(valueList[36], "%f", cfg.X_INDEX_OFFSET);
   sprintf(valueList[37], "%f", cfg.Y_INDEX_OFFSET);
   sprintf(valueList[38], "%f", cfg.Z_INDEX_OFFSET);
   
   sprintf(valueList[39], "%i", cfg.X_FLIP);
   sprintf(valueList[40], "%i", cfg.Y_FLIP);
   sprintf(valueList[41], "%i", cfg.Z_FLIP);
   */
   
   sprintf(valueList[0], "%f", cfg.X_MAX_LIMIT);
   sprintf(valueList[1], "%f", cfg.X_MIN_LIMIT);
   sprintf(valueList[2], "%f", cfg.Y_MAX_LIMIT);
   sprintf(valueList[3], "%f", cfg.Y_MIN_LIMIT);
   sprintf(valueList[4], "%f", cfg.Z_MAX_LIMIT);
   sprintf(valueList[5], "%f", cfg.Z_MIN_LIMIT);
   
   sprintf(valueList[6], "%f", cfg.X_LOC);
   sprintf(valueList[7], "%f", cfg.Y_LOC);
   sprintf(valueList[8], "%f", cfg.Z_LOC);
   
   sprintf(valueList[9], "%f", cfg.X_INDEX_OFFSET);
   sprintf(valueList[10], "%f", cfg.Y_INDEX_OFFSET);
   sprintf(valueList[11], "%f", cfg.Z_INDEX_OFFSET);
   
   ret = writeToVarsFile(filename, (char **)varList, varListCnt, (char **)valueList);
   
   for (i = 0; i < varListCnt; i++)
   {
      free(valueList[i]);
   }
   
   return ret;
}
