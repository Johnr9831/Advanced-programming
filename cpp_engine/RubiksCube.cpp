#include "RubiksCube.h"

// Constructor: Inicializamos el cubo resuelto con colores estándar
RubiksCube::RubiksCube() {
    CubeColor initialColors[6] = {CubeColor::White, CubeColor::Yellow, CubeColor::Red, CubeColor::Orange, CubeColor::Green, CubeColor::Blue};
    for(int f = 0; f < 6; f++) {
        for(int i = 0; i < 3; i++) {
            for(int j = 0; j < 3; j++) {
                cubeState[f][i][j] = initialColors[f];
            }
        }
    }
}

// 1. Algoritmo universal para rotar cualquier matriz 3x3
void RubiksCube::rotateMatrix(FaceMatrix& matrix, bool clockwise) {
    FaceMatrix temp = matrix;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (clockwise) {
                // Sentido horario: Transponer y revertir filas
                matrix[i][j] = temp[2 - j][i];
            } else {
                // Sentido anti-horario: Transponer y revertir columnas
                matrix[i][j] = temp[j][2 - i];
            }
        }
    }
}

// 2. Desplazamiento de los bordes adyacentes para la cara F (Frente)
void RubiksCube::shiftEdgesFront(bool clockwise) {
    if (clockwise) {
        // Guardamos temporalmente la fila inferior de UP
        CubeColor temp0 = cubeState[Face::U][2][0];
        CubeColor temp1 = cubeState[Face::U][2][1];
        CubeColor temp2 = cubeState[Face::U][2][2]; 

        // LEFT (columna derecha) va hacia UP (fila inferior)
        cubeState[U][2][0] = cubeState[L][2][2];
        cubeState[U][2][1] = cubeState[L][1][2];
        cubeState[U][2][2] = cubeState[L][0][2];

        // DOWN (fila superior) va hacia LEFT (columna derecha)
        cubeState[L][0][2] = cubeState[D][0][0];
        cubeState[L][1][2] = cubeState[D][0][1];
        cubeState[L][2][2] = cubeState[D][0][2];

        // RIGHT (columna izquierda) va hacia DOWN (fila superior)
        cubeState[D][0][0] = cubeState[R][2][0];
        cubeState[D][0][1] = cubeState[R][1][0];
        cubeState[D][0][2] = cubeState[R][0][0];

        // El UP temporal va hacia RIGHT (columna izquierda)
        cubeState[R][0][0] = temp0;
        cubeState[R][1][0] = temp1;
        cubeState[R][2][0] = temp2;
    } else {
        // Aquí iría la lógica anti-horaria (que es exactamente a la inversa)
    }
}

// 3. El método principal que unifica todo
void RubiksCube::rotateFace(Face face, bool clockwise) {
    // Primero, rotamos la matriz de la cara que el usuario está viendo
    rotateMatrix(cubeState[face], clockwise);

    // Segundo, desplazamos los anillos adyacentes dependiendo de qué cara sea
    switch (face) {
        case Face::F: shiftEdgesFront(clockwise); break;
        case Face::R: shiftEdgesRight(clockwise); break;
        case Face::L: shiftEdgesLeft(clockwise); break;
        case Face::U: shiftEdgesUp(clockwise); break;
        case Face::D: shiftEdgesDown(clockwise); break;
        case Face::B: shiftEdgesBack(clockwise); break;
        // case Face::U: shiftEdgesUp(clockwise); break;
        // case Face::R: shiftEdgesRight(clockwise); break;
        // ... (Deberás crear las funciones para las otras 5 caras)
        default: break;
    }
}

// Función para desplazar los bordes al rotar la cara DERECHA (R)
void RubiksCube::shiftEdgesRight(bool clockwise) {
    CubeColor temp[3];
    if (clockwise) {
        // Guardamos la columna derecha (índice 2) de la cara Arriba (U)
        for(int i = 0; i < 3; i++) temp[i] = cubeState[Face::U][i][2];
        
        // Frente (F) pasa a Arriba (U)
        for(int i = 0; i < 3; i++) cubeState[Face::U][i][2] = cubeState[Face::F][i][2];
        // Abajo (D) pasa a Frente (F)
        for(int i = 0; i < 3; i++) cubeState[Face::F][i][2] = cubeState[Face::D][i][2];
        // Atrás (B) pasa a Abajo (D). Nota: La cara trasera está invertida espacialmente
        for(int i = 0; i < 3; i++) cubeState[Face::D][i][2] = cubeState[Face::B][2-i][0]; 
        // El temporal (Arriba) pasa a Atrás (B)
        for(int i = 0; i < 3; i++) cubeState[Face::B][2-i][0] = temp[i];
    } else {
        // Sentido Anti-horario (proceso inverso)
        for(int i = 0; i < 3; i++) temp[i] = cubeState[Face::U][i][2];
        for(int i = 0; i < 3; i++) cubeState[Face::U][i][2] = cubeState[Face::B][2-i][0];
        for(int i = 0; i < 3; i++) cubeState[Face::B][2-i][0] = cubeState[Face::D][i][2];
        for(int i = 0; i < 3; i++) cubeState[Face::D][i][2] = cubeState[Face::F][i][2];
        for(int i = 0; i < 3; i++) cubeState[Face::F][i][2] = temp[i];
    }
}

// Función para desplazar los bordes al rotar la cara IZQUIERDA (L)
void RubiksCube::shiftEdgesLeft(bool clockwise) {
    CubeColor temp[3];
    if (clockwise) {
        // Guardamos la columna izquierda (índice 0) de la cara Arriba (U)
        for(int i = 0; i < 3; i++) temp[i] = cubeState[Face::U][i][0];
        
        // Atrás (B) pasa a Arriba (U). Inversión espacial.
        for(int i = 0; i < 3; i++) cubeState[Face::U][i][0] = cubeState[Face::B][2-i][2];
        // Abajo (D) pasa a Atrás (B)
        for(int i = 0; i < 3; i++) cubeState[Face::B][2-i][2] = cubeState[Face::D][i][0];
        // Frente (F) pasa a Abajo (D)
        for(int i = 0; i < 3; i++) cubeState[Face::D][i][0] = cubeState[Face::F][i][0];
        // El temporal (Arriba) pasa a Frente (F)
        for(int i = 0; i < 3; i++) cubeState[Face::F][i][0] = temp[i];
    } else {
        // Sentido Anti-horario
        for(int i = 0; i < 3; i++) temp[i] = cubeState[Face::U][i][0];
        for(int i = 0; i < 3; i++) cubeState[Face::U][i][0] = cubeState[Face::F][i][0];
        for(int i = 0; i < 3; i++) cubeState[Face::F][i][0] = cubeState[Face::D][i][0];
        for(int i = 0; i < 3; i++) cubeState[Face::D][i][0] = cubeState[Face::B][2-i][2];
        for(int i = 0; i < 3; i++) cubeState[Face::B][2-i][2] = temp[i];
    }
}

// Función para desplazar los bordes al rotar la cara ARRIBA (U)
void RubiksCube::shiftEdgesUp(bool clockwise) {
    CubeColor temp[3];
    if (clockwise) {
        // En la cara superior, interactúan las filas superiores (índice 0) de las 4 caras laterales
        for(int i = 0; i < 3; i++) temp[i] = cubeState[Face::F][0][i];
        
        for(int i = 0; i < 3; i++) cubeState[Face::F][0][i] = cubeState[Face::R][0][i];
        for(int i = 0; i < 3; i++) cubeState[Face::R][0][i] = cubeState[Face::B][0][i];
        for(int i = 0; i < 3; i++) cubeState[Face::B][0][i] = cubeState[Face::L][0][i];
        for(int i = 0; i < 3; i++) cubeState[Face::L][0][i] = temp[i];
    } else {
        for(int i = 0; i < 3; i++) temp[i] = cubeState[Face::F][0][i];
        for(int i = 0; i < 3; i++) cubeState[Face::F][0][i] = cubeState[Face::L][0][i];
        for(int i = 0; i < 3; i++) cubeState[Face::L][0][i] = cubeState[Face::B][0][i];
        for(int i = 0; i < 3; i++) cubeState[Face::B][0][i] = cubeState[Face::R][0][i];
        for(int i = 0; i < 3; i++) cubeState[Face::R][0][i] = temp[i];
    }
}

// Función para desplazar los bordes al rotar la cara ABAJO (D)
void RubiksCube::shiftEdgesDown(bool clockwise) {
    CubeColor temp[3];
    if (clockwise) {
        // En la cara inferior, interactúan las filas inferiores (índice 2)
        for(int i = 0; i < 3; i++) temp[i] = cubeState[Face::F][2][i];
        
        for(int i = 0; i < 3; i++) cubeState[Face::F][2][i] = cubeState[Face::L][2][i];
        for(int i = 0; i < 3; i++) cubeState[Face::L][2][i] = cubeState[Face::B][2][i];
        for(int i = 0; i < 3; i++) cubeState[Face::B][2][i] = cubeState[Face::R][2][i];
        for(int i = 0; i < 3; i++) cubeState[Face::R][2][i] = temp[i];
    } else {
        for(int i = 0; i < 3; i++) temp[i] = cubeState[Face::F][2][i];
        for(int i = 0; i < 3; i++) cubeState[Face::F][2][i] = cubeState[Face::R][2][i];
        for(int i = 0; i < 3; i++) cubeState[Face::R][2][i] = cubeState[Face::B][2][i];
        for(int i = 0; i < 3; i++) cubeState[Face::B][2][i] = cubeState[Face::L][2][i];
        for(int i = 0; i < 3; i++) cubeState[Face::L][2][i] = temp[i];
    }
}

// Función para desplazar los bordes al rotar la cara ATRÁS (B)
void RubiksCube::shiftEdgesBack(bool clockwise) {
    CubeColor temp[3];
    if (clockwise) {
        // La cara trasera es la más compleja por la inversión espacial de los bordes
        for(int i = 0; i < 3; i++) temp[i] = cubeState[Face::U][0][2-i];
        
        for(int i = 0; i < 3; i++) cubeState[Face::U][0][2-i] = cubeState[Face::L][2-i][0];
        for(int i = 0; i < 3; i++) cubeState[Face::L][2-i][0] = cubeState[Face::D][2][2-i];
        for(int i = 0; i < 3; i++) cubeState[Face::D][2][2-i] = cubeState[Face::R][i][2];
        for(int i = 0; i < 3; i++) cubeState[Face::R][i][2] = temp[i];
    } else {
        for(int i = 0; i < 3; i++) temp[i] = cubeState[Face::U][0][2-i];
        for(int i = 0; i < 3; i++) cubeState[Face::U][0][2-i] = cubeState[Face::R][i][2];
        for(int i = 0; i < 3; i++) cubeState[Face::R][i][2] = cubeState[Face::D][2][2-i];
        for(int i = 0; i < 3; i++) cubeState[Face::D][2][2-i] = cubeState[Face::L][2-i][0];
        for(int i = 0; i < 3; i++) cubeState[Face::L][2-i][0] = temp[i];
    }
}

// Capa E (Horizontal Central) - Involucra Frente, Derecha, Atrás e Izquierda
void RubiksCube::shiftSliceE(bool leftToRight) {
    for (int c = 0; c < 3; c++) {
        CubeColor temp = cubeState[(int)Face::F][1][c];
        if (leftToRight) { // Arrastre a la derecha: F -> R -> B -> L -> F
            cubeState[(int)Face::F][1][c] = cubeState[(int)Face::L][1][c];
            cubeState[(int)Face::L][1][c] = cubeState[(int)Face::B][1][c];
            cubeState[(int)Face::B][1][c] = cubeState[(int)Face::R][1][c];
            cubeState[(int)Face::R][1][c] = temp;
        } else { // Arrastre a la izquierda: F -> L -> B -> R -> F
            cubeState[(int)Face::F][1][c] = cubeState[(int)Face::R][1][c];
            cubeState[(int)Face::R][1][c] = cubeState[(int)Face::B][1][c];
            cubeState[(int)Face::B][1][c] = cubeState[(int)Face::L][1][c];
            cubeState[(int)Face::L][1][c] = temp;
        }
    }
}

// Capa M (Vertical Central) - Involucra Frente, Abajo, Atrás y Arriba
void RubiksCube::shiftSliceM(bool down) {
    CubeColor tempF[3];
    for (int r=0; r<3; r++) tempF[r] = cubeState[(int)Face::F][r][1];

    if (down) { // Arrastre hacia abajo: U -> F -> D -> B -> U
        for(int r=0; r<3; r++) cubeState[(int)Face::F][2-r][1] = cubeState[(int)Face::U][r][1];
        for(int r=0; r<3; r++) cubeState[(int)Face::U][r][1] = cubeState[(int)Face::B][r][1];
        for(int r=0; r<3; r++) cubeState[(int)Face::B][r][1] = cubeState[(int)Face::D][r][1];
        for(int r=0; r<3; r++) cubeState[(int)Face::D][r][1] = tempF[r];
    } else { // Arrastre hacia arriba: D -> F -> U -> B -> D
        for(int r=0; r<3; r++) cubeState[(int)Face::F][r][1] = cubeState[(int)Face::D][r][1];
        for(int r=0; r<3; r++) cubeState[(int)Face::D][r][1] = cubeState[(int)Face::B][r][1];
        for(int r=0; r<3; r++) cubeState[(int)Face::B][r][1] = cubeState[(int)Face::U][r][1];
        for(int r=0; r<3; r++) cubeState[(int)Face::U][r][1] = tempF[2-r];
    }
}
