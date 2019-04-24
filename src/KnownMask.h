//
// Created by juanfra on 24/04/19.
//

void clearMask(int maskSize, float* weights);
void RotateMask(int maskSize, float* weights);
void setupRobertsHorizontal(int* maskSize, float* weights);
void setupRobertsVertical(int* maskSize, float* weights);
void setupPrewittHorizontal(int* maskSize, float* weights);
void setupPrewittVertical(int* maskSize, float* weights);
void setupSobelHorizontal(int* maskSize, float* weights);
void setupSobelVertical(int* maskSize, float* weights);
void setupKirsh(int* maskSize, float* weights);
void setupHouseMask(int* maskSize, float* weights);
