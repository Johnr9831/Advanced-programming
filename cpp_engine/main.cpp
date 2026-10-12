#include <iostream>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <raylib.h>
#include <cstdio>
#include "RubiksCube.h" // Importamos la clase del cubo

Color ConvertirColor(CubeColor c) {
    if (c == CubeColor::White) return WHITE;
    if (c == CubeColor::Yellow) return YELLOW;
    if (c == CubeColor::Red) return RED;
    if (c == CubeColor::Orange) return ORANGE;
    if (c == CubeColor::Green) return GREEN;
    if (c == CubeColor::Blue) return BLUE;
    return BLACK;
}

// Instancia global del cubo.
// Al ser global, el hilo de red puede modificarla y, en el futuro, 
// el bucle principal de gráficos podrá leerla para dibujarla.
RubiksCube cubo;
float cubePosX = 0.0f; // NUEVO: Posición X del cubo
float cubePosY = 0.0f; // NUEVO: Posición Y del cubo

void listenToMediaPipe() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(65432);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);
    
    std::cout << "Motor C++ iniciado. Esperando conexión de MediaPipe..." << std::endl;
    
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    std::cout << "MediaPipe conectado. Listo para recibir gestos." << std::endl;
    
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(new_socket, buffer, 1024);
        
        if (valread > 0) {
            std::string comando(buffer);
            comando = comando.substr(0, comando.find('\n')); // Limpiar el salto de línea
            
            // Reaccionamos a los gestos de arrastre táctil de Python
            if (comando == "U_CW") cubo.shiftEdgesUp(true);
            else if (comando == "U_CCW") cubo.shiftEdgesUp(false);
            
            else if (comando == "D_CW") cubo.shiftEdgesDown(true);
            else if (comando == "D_CCW") cubo.shiftEdgesDown(false);
            
            else if (comando == "L_CW") cubo.shiftEdgesLeft(true);
            else if (comando == "L_CCW") cubo.shiftEdgesLeft(false);
            
            else if (comando == "R_CW") cubo.shiftEdgesRight(true);
            else if (comando == "R_CCW") cubo.shiftEdgesRight(false);

            else if (comando == "E_CW") cubo.shiftSliceE(true);
            else if (comando == "E_CCW") cubo.shiftSliceE(false);
            
            else if (comando == "M_CW") cubo.shiftSliceM(true);
            else if (comando == "M_CCW") cubo.shiftSliceM(false);
            
            if (comando.find("F_CLK") != std::string::npos) {
                cubo.rotateFace(Face::F, true);
                std::cout << "Accion: Cara FRONTAL (F) rotada en sentido HORARIO.\n";
            } else if (comando.find("F_CCW") != std::string::npos) {
                cubo.rotateFace(Face::F, false);
                std::cout << "Accion: Cara FRONTAL (F) rotada en sentido ANTI-HORARIO.\n";
            } else if (comando.find("R_CLK") != std::string::npos) {
                cubo.rotateFace(Face::R, true); 
                std::cout << "Accion: Cara DERECHA (R) rotada en sentido HORARIO.\n";
            } else if (comando.find("R_CCW") != std::string::npos) {
                cubo.rotateFace(Face::R, false);
                std::cout << "Accion: Cara DERECHA (R) rotada en sentido ANTI-HORARIO.\n";
            } else if (comando.find("L_CLK") != std::string::npos) {
                cubo.rotateFace(Face::L, true);
                std::cout << "Accion: Cara IZQUIERDA (L) rotada en sentido HORARIO.\n";
            } else if (comando.find("L_CCW") != std::string::npos) {
                cubo.rotateFace(Face::L, false);
                std::cout << "Accion: Cara IZQUIERDA (L) rotada en sentido ANTI-HORARIO.\n";
            }
            
        } else if (valread == 0) {
            std::cout << "Python se ha desconectado." << std::endl;
            break;
        }
    }
    close(new_socket);
    close(server_fd);
}

int main() {
    // 1. Lanzar el servidor TCP en un hilo secundario para que no bloquee los gráficos
    std::thread serverThread(listenToMediaPipe);
    serverThread.detach(); // Le decimos que corra de forma independiente en el fondo

    // 2. Inicializar la ventana de Raylib (Hilo principal)
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Controlador de Rubik 3D - Avanzado");
    
    // Configurar la cámara 3D para que sea FRONTAL y estática
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 0.0f, 16.0f }; // Alejar la cámara en el eje Z
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    SetTargetFPS(60);

    SetTraceLogLevel(LOG_WARNING);

    // Variable para almacenar el video del mundo real
    Texture2D bgTexture = { 0 };

    // 3. Bucle Gráfico Principal
    while (!WindowShouldClose()) { 

        UpdateCamera(&camera, CAMERA_ORBITAL); //Borrar en caso de que no funcione
        
        // 1. LEER LA CÁMARA DESDE PYTHON (Ahora busca directamente en su propia carpeta)
        Image bgImg = LoadImage("fondo_ar.jpg"); 
        if (bgImg.data != NULL) {
            if (bgTexture.id != 0) UnloadTexture(bgTexture);
            bgTexture = LoadTextureFromImage(bgImg);
            UnloadImage(bgImg);
        }

        BeginDrawing();
            ClearBackground(RAYWHITE); 

            // 2. DIBUJAR EL MUNDO REAL (Fondo de cámara)
            if (bgTexture.id != 0) {
                DrawTexturePro(bgTexture,
                    (Rectangle){0, 0, (float)bgTexture.width, (float)bgTexture.height},
                    (Rectangle){0, 0, (float)screenWidth, (float)screenHeight},
                    (Vector2){0, 0}, 0.0f, WHITE);
            }

            // 3. DIBUJAR EL HOLOGRAMA (El cubo completo con sus colores)
            BeginMode3D(camera);
                // Núcleo gris central
                DrawCube((Vector3){cubePosX, cubePosY, 0.0f}, 2.9f, 2.9f, 2.9f, DARKGRAY);

                // Pegatinas de colores
                for(int r = 0; r < 3; r++) {
                    for(int c = 0; c < 3; c++) {
                        float x, y, z;
                        float tam = 0.9f; 
                        float gr = 0.1f;  

                        // FRENTE
                        x = (c - 1) * 1.0f + cubePosX; y = (1 - r) * 1.0f + cubePosY; z = 1.5f;
                        DrawCube((Vector3){x, y, z}, tam, tam, gr, ConvertirColor(cubo.cubeState[Face::F][r][c]));
                        DrawCubeWires((Vector3){x, y, z}, tam, tam, gr, BLACK);

                        // ATRÁS
                        x = (1 - c) * 1.0f + cubePosX; y = (1 - r) * 1.0f + cubePosY; z = -1.5f;
                        DrawCube((Vector3){x, y, z}, tam, tam, gr, ConvertirColor(cubo.cubeState[Face::B][r][c]));
                        DrawCubeWires((Vector3){x, y, z}, tam, tam, gr, BLACK);

                        // DERECHA
                        x = 1.5f + cubePosX; y = (1 - r) * 1.0f + cubePosY; z = (1 - c) * 1.0f;
                        DrawCube((Vector3){x, y, z}, gr, tam, tam, ConvertirColor(cubo.cubeState[Face::R][r][c]));
                        DrawCubeWires((Vector3){x, y, z}, gr, tam, tam, BLACK);

                        // IZQUIERDA
                        x = -1.5f + cubePosX; y = (1 - r) * 1.0f + cubePosY; z = (c - 1) * 1.0f;
                        DrawCube((Vector3){x, y, z}, gr, tam, tam, ConvertirColor(cubo.cubeState[Face::L][r][c]));
                        DrawCubeWires((Vector3){x, y, z}, gr, tam, tam, BLACK);

                        // ARRIBA
                        x = (c - 1) * 1.0f + cubePosX; y = 1.5f + cubePosY; z = (r - 1) * 1.0f;
                        DrawCube((Vector3){x, y, z}, tam, gr, tam, ConvertirColor(cubo.cubeState[Face::U][r][c]));
                        DrawCubeWires((Vector3){x, y, z}, tam, gr, tam, BLACK);

                        // ABAJO
                        x = (c - 1) * 1.0f + cubePosX; y = -1.5f + cubePosY; z = (1 - r) * 1.0f;
                        DrawCube((Vector3){x, y, z}, tam, gr, tam, ConvertirColor(cubo.cubeState[Face::D][r][c]));
                        DrawCubeWires((Vector3){x, y, z}, tam, gr, tam, BLACK);
                    }
                }
            EndMode3D();
        EndDrawing();
    }

    // 4. Cerrar todo al terminar
    CloseWindow();
    return 0;
}