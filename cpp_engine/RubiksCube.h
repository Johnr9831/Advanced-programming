#ifndef RUBIKS_CUBE_H
#define RUBIKS_CUBE_H

#include <array>

enum class CubeColor { White, Yellow, Red, Orange, Green, Blue };
enum Face { U = 0, D, F, B, L, R };

using FaceMatrix = std::array<std::array<CubeColor, 3>, 3>;

class RubiksCube {
public:
    std::array<FaceMatrix, 6> cubeState;

    // Métodos auxiliares privados
    void rotateMatrix(FaceMatrix& matrix, bool clockwise);
    void shiftEdgesFront(bool clockwise);
    void shiftEdgesRight(bool clockwise);
    void shiftEdgesLeft(bool clockwise);
    void shiftEdgesUp(bool clockwise);
    void shiftEdgesDown(bool clockwise);
    void shiftEdgesBack(bool clockwise);
    void shiftSliceE(bool leftToRight);
    void shiftSliceM(bool down);
public:
    RubiksCube(); 
    void rotateFace(Face face, bool clockwise);
    
};

#endif