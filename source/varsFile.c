#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include "varsFile.h"

#define TRUE 1
#define FALSE 0



char **retValueListPtr;
int retValueListCnt;
int retValueListAllocated = FALSE;


void freeValueList()
{
   int i;
   
   if (retValueListAllocated)
   {
      for (i = 0; i < retValueListCnt; i++)
      {
         free(retValueListPtr[i]);
      }
      free(retValueListPtr);
      
      retValueListAllocated = FALSE;
   }
}


int readVarsFile(char *filename, char **inputVarList,
                 int inputVarListCnt, char ***retValueList)
{
   int i, j, c;
   int fileLineCnt;
   FILE *filePtr;
   int ignoreTxt;
   
   int readValue;
   int *valueSz;
   int readVar;
   int *varSz;
   
   int retValueListInc;
   int valueCharInc;
   int varCharInc;
   char **valueList;
   char **varList;
   
   
   filePtr = fopen(filename, "r");
   
   if (filePtr == NULL)
   return TRUE;
   
   
   // find how many lines are in the file
   fileLineCnt = 0;
   while ((c = fgetc(filePtr)) != EOF)
   {
      if (c == 10)
      fileLineCnt++;
   }
   fileLineCnt++;
   
   
   // allocate memory to get the size of each value
   valueSz = (int *)malloc(fileLineCnt * sizeof(int));
   
   // allocate memory to get the size of each variable
   varSz = (int *)malloc(fileLineCnt * sizeof(int));
   
   
   rewind(filePtr);
   
   // get the character length of each variable label and value
   i = 0;
   valueSz[0] = 0;
   varSz[0] = 0;
   ignoreTxt = FALSE;
   readValue = FALSE;
   readVar = TRUE;
   while ((c = fgetc(filePtr)) != EOF)
   {
      if (c == '#')
      ignoreTxt = TRUE;
      
      if (c == 13)
      continue;
      
      if (c == 10)
      {
         if (readVar)// if '=' was not found
         varSz[i] = 0;
         
         i++;
         valueSz[i] = 0;
         varSz[i] = 0;
         ignoreTxt = FALSE;
         readValue = FALSE;
         readVar = TRUE;
         continue;
      }
      
      if (ignoreTxt)
      continue;
      
      // count the length of each variable value
      if (readValue)
      valueSz[i]++;
      
      if (c == '=')
      {
         readValue = TRUE;
         readVar = FALSE;
         continue;
      }
      
      if (isspace(c))
      continue;
      
      // count the length of each variable label
      if (readVar)
      varSz[i]++;
   }
   
   
   // allocate memory for the variable list
   varList = (char **)malloc(fileLineCnt * sizeof(char *));
   
   for (i = 0; i < fileLineCnt; i++)
   {
      varList[i] = (char *)malloc((varSz[i]+1) * sizeof(char));
   }
   
   // allocate memory for the value list
   valueList = (char **)malloc(fileLineCnt * sizeof(char *));
   
   for (i = 0; i < fileLineCnt; i++)
   {
      valueList[i] = (char *)malloc((valueSz[i]+1) * sizeof(char));
   }
   
   
   rewind(filePtr);
   
   // read the label and value of each variable
   i = 0;
   valueCharInc = 0;
   varCharInc = 0;
   ignoreTxt = FALSE;
   readValue = FALSE;
   readVar = TRUE;
   while ((c = fgetc(filePtr)) != EOF)
   {
      if (c == '#')
      ignoreTxt = TRUE;
      
      if (c == 13)
      continue;
      
      if (c == 10)
      {
         valueList[i][valueCharInc] = 0;
         valueCharInc = 0;
         
         varList[i][varCharInc] = 0;
         varCharInc = 0;
         
         i++;
         ignoreTxt = FALSE;
         readValue = FALSE;
         readVar = TRUE;
         continue;
      }
      
      if (ignoreTxt)
      continue;
      
      // read the value of each variable
      if (readValue)
      if (valueSz[i])
      {
         valueList[i][valueCharInc] = c;
         valueCharInc++;
      }
      
      if (c == '=')
      {
         readValue = TRUE;
         readVar = FALSE;
         continue;
      }
      
      if (isspace(c))
      continue;
      
      // read the label of each variable
      if (readVar)
      if (varSz[i])
      {
         varList[i][varCharInc] = c;
         varCharInc++;
      }
   }
   
   valueList[i][valueCharInc] = 0;
   varList[i][varCharInc] = 0;
   
   
   freeValueList();
   
   // allocate memory for the returning value list
   retValueListCnt = inputVarListCnt;
   retValueListPtr = (char **)malloc(retValueListCnt * sizeof(char *));
   
   retValueListAllocated = TRUE;
   
   
   retValueListInc = 0;
   for (i=0; i < inputVarListCnt; i++)
   {
      for (j=0; j < fileLineCnt; j++)
      {
         if (varSz[j])
         if (!strcmp(inputVarList[i], varList[j]))
         {
            retValueListPtr[retValueListInc] = (char *)malloc((valueSz[j]+1) * sizeof(char));
            
            strcpy(retValueListPtr[retValueListInc], valueList[j]);
            retValueListInc++;
            
            break;
         }
      }
   }
   
   
   // link to allocated memory of the variable values
   *retValueList = (char **)retValueListPtr;
   
   
   for (i = 0; i < fileLineCnt; i++)
   {
      free(varList[i]);
   }
   free(varList);
   
   for (i = 0; i < fileLineCnt; i++)
   {
      free(valueList[i]);
   }
   free(valueList);
   
   free(valueSz);
   free(varSz);
   
   
   fclose(filePtr);
   return FALSE;
}





int writeToVarsFile(char *filename, char **inputVarList,
                    int inputVarListCnt, char **inputValueList)
{
   struct stat stat_p;
   int i, j;
   FILE *filePtr;
   char *fileData;
   int fileLineCnt;
   int ignoreTxt;
   
   int readValue;
   int *valueSz;
   int readVar;
   int *varSz;
   
   int valueCharInc;
   int varCharInc;
   char **valueList;
   char **varList;
   
   int varsNotFoundCnt;
   int *varsNotFound;
   int varFound;
   int charInc;
   
   
   
   // if the file does not exist
   if (-1 == stat(filename, &stat_p))
   {
      // open file for write
      filePtr = fopen(filename,"w");
      
      if (!filePtr)
      return TRUE;
      
      // write the variables to file
      for (i=0; i < inputVarListCnt; i++)
      {
         fprintf(filePtr, "%s=%s\n", inputVarList[i], inputValueList[i]);// change \n to \n\r for windows
      }
      
      fclose(filePtr);
      
      return FALSE;
   }
   
   
   
   // allocate memory for bmp file
   fileData = (char *)malloc(stat_p.st_size);
   
   // open file for read
   filePtr = fopen(filename,"r");
   if (!filePtr)
   return TRUE;
   
   // read file data to buffer
   fread(fileData, sizeof(char), stat_p.st_size, filePtr);
   
   fclose(filePtr);
   

   
   // find how many lines are in the file
   fileLineCnt = 1;
   for (i=0; i < stat_p.st_size; i++)
   {
      if (fileData[i] == 10) // new line
      fileLineCnt++;
   }
   
   
   
   // allocate memory to get the size of each variable
   varSz = (int *)malloc(fileLineCnt * sizeof(int));
   
   // allocate memory to get the size of each value
   valueSz = (int *)malloc(fileLineCnt * sizeof(int));
   
   
   // get the character length of each variable label and value
   i = 0;
   valueSz[0] = 0;
   varSz[0] = 0;
   ignoreTxt = FALSE;
   readValue = FALSE;
   readVar = TRUE;
   for (j=0; j < stat_p.st_size; j++)
   {
      if (fileData[j] == '#')
      ignoreTxt = TRUE;
      
      if (fileData[j] == 13)
      continue;
      
      if (fileData[j] == 10)
      {
         if (readVar)// if '=' was not found
         varSz[i] = 0;
         
         i++;
         valueSz[i] = 0;
         varSz[i] = 0;
         ignoreTxt = FALSE;
         readValue = FALSE;
         readVar = TRUE;
         continue;
      }
      
      if (ignoreTxt)
      continue;
      
      // count the length of each variable value
      if (readValue)
      valueSz[i]++;
      
      if (fileData[j] == '=')
      {
         readValue = TRUE;
         readVar = FALSE;
         continue;
      }
      
      if (isspace(fileData[j]))
      continue;
      
      // count the length of each variable label
      if (readVar)
      varSz[i]++;
   }
   
   
   
   // allocate memory for the variable list
   varList = (char **)malloc(fileLineCnt * sizeof(char *));
   
   for (i = 0; i < fileLineCnt; i++)
   {
      varList[i] = (char *)malloc((varSz[i]+1) * sizeof(char));
   }
   
   // allocate memory for the value list
   valueList = (char **)malloc(fileLineCnt * sizeof(char *));
   
   for (i = 0; i < fileLineCnt; i++)
   {
      valueList[i] = (char *)malloc((valueSz[i]+1) * sizeof(char));
   }
   
   
   // read the label and value of each variable
   i = 0;
   valueCharInc = 0;
   varCharInc = 0;
   ignoreTxt = FALSE;
   readValue = FALSE;
   readVar = TRUE;
   for (j=0; j < stat_p.st_size; j++)
   {
      if (fileData[j] == '#')
      ignoreTxt = TRUE;
      
      if (fileData[j] == 13)
      continue;
      
      if (fileData[j] == 10)
      {
         valueList[i][valueCharInc] = 0;
         valueCharInc = 0;
         
         varList[i][varCharInc] = 0;
         varCharInc = 0;
         
         i++;
         ignoreTxt = FALSE;
         readValue = FALSE;
         readVar = TRUE;
         continue;
      }
      
      if (ignoreTxt)
      continue;
      
      // read the value of each variable
      if (readValue)
      if (valueSz[i])
      {
         valueList[i][valueCharInc] = fileData[j];
         valueCharInc++;
      }
      
      if (fileData[j] == '=')
      {
         readValue = TRUE;
         readVar = FALSE;
         continue;
      }
      
      if (isspace(fileData[j]))
      continue;
      
      // read the label of each variable
      if (readVar)
      if (varSz[i])
      {
         varList[i][varCharInc] = fileData[j];
         varCharInc++;
      }
   }
   
   valueList[i][valueCharInc] = 0;
   varList[i][varCharInc] = 0;
   
   
   
   // allocate memory to note the variables that are not found
   varsNotFound = (int *)malloc(inputVarListCnt * sizeof(int));
   
   // find the variables the are not found in the file
   varsNotFoundCnt = 0;
   for (i=0; i < inputVarListCnt; i++)
   {
      varFound = FALSE;
      for (j=0; j < fileLineCnt; j++)
      {
         if ((varSz[j]) && (!strcmp(inputVarList[i], varList[j])))
         {
            varFound = TRUE;
            break;
         }
      }
      
      if (varFound == FALSE)
      {
         varsNotFound[i] = TRUE;
         varsNotFoundCnt++;
      }
      else
      varsNotFound[i] = FALSE;
   }
   
   
   
   // open file for write
   filePtr = fopen(filename,"w");
   if (!filePtr)
   return TRUE;
   
   
   // write the new file
   charInc = 0;
   varFound = FALSE;
   for (j=0; j < fileLineCnt; j++)
   {
      varFound = FALSE;
      for (i=0; i < inputVarListCnt; i++)
      {
         if ((varSz[j]) && (!strcmp(inputVarList[i], varList[j])))
         {
            varFound = TRUE;
            break;
         }
      }
      
      if (varFound)
      {
         fprintf(filePtr, "%s=%s\n", inputVarList[i], inputValueList[i]);// change \n to \n\r for windows
         
         while (charInc < stat_p.st_size)
         {
            if (fileData[charInc] == 10)
            {
               charInc++;
               break;
            }
            
            charInc++;
         }
      }
      else
      {
         while (charInc < stat_p.st_size)
         {
            if (fileData[charInc] == 13)
            {
               charInc++;
               continue;
            }
            
            putc(fileData[charInc], filePtr);
            
            if (fileData[charInc] == 10)
            {
               //putc(10, filePtr); // for windows
               charInc++;
               break;
            }
            
            charInc++;
         }
      }
   }
   
   
   // write the missing variables to file
   for (i=0; i < inputVarListCnt; i++)
   {
      if (varsNotFound[i])
      fprintf(filePtr, "\n%s=%s", inputVarList[i], inputValueList[i]);// change \n to \n\r for windows
   }
   
   
   fclose(filePtr);
   
   
   
   free(varsNotFound);
   
   for (i = 0; i < fileLineCnt; i++)
   {
      free(valueList[i]);
   }
   free(valueList);
   
   for (i = 0; i < fileLineCnt; i++)
   {
      free(varList[i]);
   }
   free(varList);
   
   free(valueSz);
   free(varSz);
   free(fileData);
   
   
   return FALSE;
}
