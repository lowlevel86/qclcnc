#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "gcodeParser.h"

#define TRUE 1
#define FALSE 0



int memAllocated = FALSE;
int commCnt;
int codeCnt;
int *code;
double *value;
int *commSz;
char **comm;
   


void freeParsedGcode()
{
   int i;
   
   if (memAllocated)
   {
      for (i = 0; i < commCnt; i++)
      {
         free(comm[i]);
      }
      free(comm);
      free(value);
      free(code);
      free(commSz);
      
      memAllocated = FALSE;
   }
}



int parseGcode(char *gfile, struct gcode *g)
{
   int i;
   int c;
   int codeInc;
   int commInc;
   int cntLock, readStr;
   char numBuff[32];
   int numBuffInc;
   int cInc;
   FILE *filePtr;
   
   
   filePtr = fopen(gfile, "r");
   
   if (filePtr == NULL)
   return 1;
   
   freeParsedGcode();
   
   
   // get the code and comment counts
   codeCnt = 0;
   commCnt = 0;
   cntLock = FALSE;
   while ((c = fgetc(filePtr)) != EOF)
   {
      if (cntLock == FALSE)
      if ((c == 'F') ||// add codes here
          (c == 'G') ||// and below
          (c == 'M') ||
          (c == 'X') ||
          (c == 'Y') ||
          (c == 'Z') ||
          (c == '('))
      codeCnt++;
      
      // code count for parsing just numbers
      if (cntLock == FALSE)
      if (c == ' ')
      {
         numBuffInc = 0;
         while ((c = fgetc(filePtr)) != EOF)
         {
            if (c == ' ')
            continue;
            
            if ((isdigit(c)) || (c == '-') || (c == '.'))
            numBuffInc++;
            else
            break;
         }
         
         fseek(filePtr, -1, SEEK_CUR);
         
         if (numBuffInc)
         codeCnt++;
      }
      
      if (cntLock == FALSE)
      if (c == '(')
      {
         commCnt++;
         cntLock = TRUE;
      }
      
      if (cntLock == TRUE)
      if (c == ')')
      cntLock = FALSE;
   }
   
   
   
   // allocate memory for comment size data
   commSz = (int *)malloc(commCnt * sizeof(int));
   
   
   rewind(filePtr);
   
   // get the size of each comment
   readStr = FALSE;
   commInc = 0;
   while ((c = fgetc(filePtr)) != EOF)
   {
      if (readStr)
      commSz[commInc]++;
      
      if (readStr == FALSE)
      if (c == '(')
      {
         commSz[commInc] = 0;
         readStr = TRUE;
      }
      
      if (readStr == TRUE)
      if (c == ')')
      {
         readStr = FALSE;
         commInc++;
      }
   }
   
   
   
   // allocate memory for code data
   code = (int *)malloc(codeCnt * sizeof(int));
   value = (double *)malloc(codeCnt * sizeof(double));
   
   // initialize with zeros
   for (i=0; i < codeCnt; i++)
   {
      code[i] = 0;
      value[i] = 0;
   }
   
   
   rewind(filePtr);
   
   // get the code data
   codeInc = 0;
   commInc = 0;
   cntLock = FALSE;
   while ((c = fgetc(filePtr)) != EOF)
   {
      if (cntLock == FALSE)
      if ((c == 'F') ||// add codes here
          (c == 'G') ||// and above
          (c == 'M') ||
          (c == 'X') ||
          (c == 'Y') ||
          (c == 'Z'))
      {
         code[codeInc] = c;
         
         numBuffInc = 0;
         while ((c = fgetc(filePtr)) != EOF)
         {
            if ((isdigit(c)) || (c == '-') || (c == '.'))
            numBuff[numBuffInc] = c;
            else
            break;
            
            numBuffInc++;
         }
         
         fseek(filePtr, -1, SEEK_CUR);
         
         if (numBuffInc)
         {
            numBuff[numBuffInc] = 0;
            value[codeInc] = atof(numBuff);
         }
         else
         value[codeInc] = NAN;
         
         codeInc++;
      }
      
      // parsing just a number
      if (cntLock == FALSE)
      if (c == ' ')
      {
         numBuffInc = 0;
         while ((c = fgetc(filePtr)) != EOF)
         {
            if (c == ' ')
            continue;
            
            if ((isdigit(c)) || (c == '-') || (c == '.'))
            {
               numBuff[numBuffInc] = c;
               numBuffInc++;
            }
            else
            break;
         }
         
         fseek(filePtr, -1, SEEK_CUR);
         
         if (numBuffInc)
         {
            numBuff[numBuffInc] = 0;
            value[codeInc] = atof(numBuff);
            codeInc++;
         }
      }
      
      // save the comment number in the value array
      if (cntLock == FALSE)
      if (c == '(')
      {
         code[codeInc] = c;
         value[codeInc] = commInc;
         commInc++;
         codeInc++;
         
         cntLock = TRUE;
      }
      
      if (cntLock == TRUE)
      if (c == ')')
      cntLock = FALSE;
   }
   
   
   
   // allocate memory for comment data
   comm = (char **)malloc(commCnt * sizeof(char *));
   
   for (i = 0; i < commCnt; i++)
   {
      comm[i] = (char *)malloc(commSz[i] * sizeof(char));
   }
   
   memAllocated = TRUE;
   
   
   rewind(filePtr);
   
   // get the comment data
   readStr = FALSE;
   commInc = 0;
   while ((c = fgetc(filePtr)) != EOF)
   {
      if (readStr == TRUE)
      if (c == ')')
      {
         readStr = FALSE;
         comm[commInc][cInc] = 0;
         commInc++;
      }
      
      if (readStr)
      {
         comm[commInc][cInc] = c;
         cInc++;
      }
      
      if (readStr == FALSE)
      if (c == '(')
      {
         readStr = TRUE;
         cInc = 0;
      }
   }
   
   
   
   g->codeCnt = codeCnt;
   g->codeInc = 0;
   g->code = code;
   g->value = value;
   g->commSz = commSz;
   g->comm = comm;
   
   fclose(filePtr);
   
   return 0;
}
