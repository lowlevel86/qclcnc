#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>

#define TRUE 1
#define FALSE 0


char *strToUpper(char *str)
{
   int i = 0;
   
   while (str[i])
   {
      str[i] = toupper(str[i]);
      i++;
   }
   
   return str;
}

char *getFileExt(char *filename)
{
   char *dot = strrchr(filename, '.');
   
   if ((!dot) || (dot == filename))
   return "";
   
   return dot + 1;
}


char **fileList;
int fileCnt;
int fileListAllocated = FALSE;

void freeFileList()
{
   int i;
   
   if (fileListAllocated)
   {
      for (i = 0; i < fileCnt; i++)
      free(fileList[i]);
      free(fileList);
      
      fileListAllocated = FALSE;
   }
}

int getFileList(char *dir, char *fileExt, int *files, char ***fileListPtr)
{
   int i;
   DIR *dirHND;
   struct dirent *ep;
   char filenameBuff[256];
   char extBuff[256];
   
   
   dirHND = opendir(dir);

   if (dirHND == NULL)
   return TRUE;
   
   
   fileCnt = 0;
   strcpy(extBuff, fileExt);
   
   
   while ((ep = readdir(dirHND)))
   {
      strcpy(filenameBuff, ep->d_name);
      
      if (!strcmp(strToUpper(getFileExt(filenameBuff)), strToUpper(extBuff)))
      fileCnt += 1;
   }
   
   
   freeFileList();
   
   fileList = (char **)malloc(fileCnt * sizeof(char *));
   
   for (i = 0; i < fileCnt; i++)
   {
      fileList[i] = (char *)malloc(256 * sizeof(char));
   }
   fileListAllocated = TRUE;
   
   
   rewinddir(dirHND);
   
   i = 0;
   while ((ep = readdir(dirHND)))
   {
      strcpy(filenameBuff, ep->d_name);
      
      if (!strcmp(strToUpper(getFileExt(filenameBuff)), strToUpper(extBuff)))
      {
         strcpy(fileList[i], ep->d_name);
         i++;
      }
   }
   
   
   // return variables
   *fileListPtr = (char **)fileList;
   *files = fileCnt;
   
   closedir(dirHND);
   return FALSE;
}
