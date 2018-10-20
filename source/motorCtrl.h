#include "encoder.h"
#include "gcodeParser.h"
#include "configure.h"

#define ERR_NO_FEED_RATE 1
#define PROGRAM_PAUSE 2
#define PROGRAM_END 3
#define END_OF_JOG 3
#define OUT_OF_BOUNDS 4

void motorCtrlReset();
int executeGcode(struct gcode *, struct encoder, struct encoder, struct encoder, struct config);
int jog(double, double, double, double, struct encoder, struct encoder, struct encoder, struct config, int);
