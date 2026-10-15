#ifndef WEIGHTS_H
#define WEIGHTS_H

// Dummy C weights to verify compiler pipeline
const float B1[64] = {0.0f};
const float W1[64][3] = {{0.1f, 0.1f, 0.1f}};
const float B2[64] = {0.0f};
const float W2[64][64] = {{0.1f}};
const float B_OUT[4] = {0.0f, 0.0f, 0.0f, 0.0f};
const float W_OUT[4][64] = {{0.1f}};

#endif
