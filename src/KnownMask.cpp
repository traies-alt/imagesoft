//
// Created by juanfra on 24/04/19.
//
#include "KnownMask.h"

void setupRobertsHorizontal(int* maskSize, float* weights) {
    int _maskSize = 2;
    (*maskSize) = _maskSize;
    for (int i = 0; i < _maskSize; i++) {
        for (int j = 0; j < _maskSize; j++) {
            if(j!=i) {
                weights[i * _maskSize + j] = 0;
            } else if(i==0){
                weights[i * _maskSize + j] = 1;
            } else {
                weights[i * _maskSize + j] = -1;
            }
        }
    }
}


void setupRobertsVertical(int* maskSize, float* weights) {
    int _maskSize = 2;
    (*maskSize) = _maskSize;
    for (int i = 0; i < _maskSize; i++) {
        for (int j = 0; j < _maskSize; j++) {
            if(j==i) {
                weights[i * _maskSize + j] = 0;
            } else if(i==0){
                weights[i * _maskSize + j] = 1;
            } else {
                weights[i * _maskSize + j] = -1;
            }
        }
    }
}
void setupPrewittHorizontal(int* maskSize, float* weights) {
    int _maskSize = 3;
    (*maskSize) = _maskSize;
    for (int i = 0; i < _maskSize; i++) {
        for (int j = 0; j < _maskSize; j++) {
            weights[i * _maskSize + j] = i - 1;
        }
    }
}
void setupPrewittVertical(int* maskSize, float* weights) {
    int _maskSize = 3;
    (*maskSize) = _maskSize;
    for (int i = 0; i < _maskSize; i++) {
        for (int j = 0; j < _maskSize; j++) {
            weights[i * _maskSize + j] = j - 1;
        }
    }
}
void setupSobelHorizontal(int* maskSize, float* weights) {
    int _maskSize = 3;
    (*maskSize) = _maskSize;
    for (int i = 0; i < _maskSize; i++) {
        for (int j = 0; j < _maskSize; j++) {
            if(j==1) {
                weights[i * _maskSize + j] = (i - 1)*2;
            } else {
                weights[i * _maskSize + j] = i - 1;
            }
        }
    }
}
void setupSobelVertical(int* maskSize, float* weights) {
    int _maskSize = 3;
    (*maskSize) = _maskSize;
    for (int i = 0; i < _maskSize; i++) {
        for (int j = 0; j < _maskSize; j++) {
            if(i==1) {
                weights[i * _maskSize + j] = (j - 1)*2;
            } else {
                weights[i * _maskSize + j] = j - 1;
            }
        }
    }
}

void clearMask(int maskSize, float* weights) {
    for (int i = 0; i < maskSize; i++) {
        for (int j = 0; j < maskSize; j++) {
            weights[i * maskSize + j] = 0;
        }
    }
}

