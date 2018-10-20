#ifndef _ENCODER_HEADER_H_
#define _ENCODER_HEADER_H_

// encoder states
#define A_HIGH 1
#define B_HIGH 2
#define AB_HIGH 3
#define AB_LOW 0

struct encoder {
   int loc;
   int currState;
   int priorState;
   int error;
};

void getEncoderLoc(struct encoder *);
void getFlippedEncoderLoc(struct encoder *);

#endif // _ENCODER_HEADER_H_
