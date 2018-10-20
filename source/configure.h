#ifndef _CONFIGURE_HEADER_H_
#define _CONFIGURE_HEADER_H_

#include "encoder.h"

struct config {
int X_ENCODER_I;
int X_ENCODER_A;
int X_ENCODER_B;
int Y_ENCODER_I;
int Y_ENCODER_A;
int Y_ENCODER_B;
int Z_ENCODER_I;
int Z_ENCODER_A;
int Z_ENCODER_B;

double X_MECHANICAL_DEGREES_PER_MM;
double Y_MECHANICAL_DEGREES_PER_MM;
double Z_MECHANICAL_DEGREES_PER_MM;

int X_MOTOR_ENABLE;
int X_MOTOR_DIRECT;
int X_MOTOR_STEP;
int Y_MOTOR_ENABLE;
int Y_MOTOR_DIRECT;
int Y_MOTOR_STEP;
int Z_MOTOR_ENABLE;
int Z_MOTOR_DIRECT;
int Z_MOTOR_STEP;

double X_PULSES_PER_MM;
double Y_PULSES_PER_MM;
double Z_PULSES_PER_MM;

double POSITIONING_SPEED;
double JOG_SPEED;
double JOG_DISTANCE;

double X_MAX_LIMIT;
double X_MIN_LIMIT;
double Y_MAX_LIMIT;
double Y_MIN_LIMIT;
double Z_MAX_LIMIT;
double Z_MIN_LIMIT;

double X_LOC;
double Y_LOC;
double Z_LOC;

double X_INDEX_OFFSET;
double Y_INDEX_OFFSET;
double Z_INDEX_OFFSET;

int X_FLIP;
int Y_FLIP;
int Z_FLIP;
};

int loadConfigFile(char *, struct config *);
int saveToConfigFile(char *, struct config);

#endif // _CONFIGURE_HEADER_H_
