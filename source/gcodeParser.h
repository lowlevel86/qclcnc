#ifndef _GCODEPARSER_HEADER_H_
#define _GCODEPARSER_HEADER_H_

struct gcode {
int codeCnt;
int codeInc;
int *code;
double *value;
int *commSz;
char **comm;
};

int parseGcode(char *, struct gcode *);
void freeParsedGcode();

#endif // _GCODEPARSER_HEADER_H_
