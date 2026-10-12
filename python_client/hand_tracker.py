import cv2
import mediapipe as mp
import math
import socket
import os

# 1. Configuración del Socket (Conexión con tu motor C++)
host = '127.0.0.1'
port = 65432
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    client_socket.connect((host, port))
    print("Conectado al motor C++ con éxito.")
except:
    print("Error: Asegúrate de ejecutar primero el motor en C++ (./motor_rubik).")
    exit()

# 2. Configuración de MediaPipe
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(
    static_image_mode=False,
    max_num_hands=1,
    min_detection_confidence=0.7,
    min_tracking_confidence=0.7
)
mp_draw = mp.solutions.drawing_utils

# 3. Inicializar Cámara
cap = cv2.VideoCapture(0)

print("Cámara iniciada. Haz el gesto de pellizcar con el índice y el pulgar.")

# Variables para la memoria del arrastre
is_pinching = False
start_x, start_y = 0, 0
umbral_arrastre = 60 # Píxeles que debes mover el dedo para que cuente como giro

while cap.isOpened():
    success, img = cap.read()
    if not success: break
    img = cv2.flip(img, 1)
    img_clean = img.copy()
    h, w, _ = img.shape
    
    img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    results = hands.process(img_rgb)

    # Dibujar líneas guía para que veas las zonas del cubo (opcional, ayuda visual)
    cv2.line(img, (0, h//3), (w, h//3), (200, 200, 200), 1)
    cv2.line(img, (0, 2*h//3), (w, 2*h//3), (200, 200, 200), 1)
    cv2.line(img, (w//3, 0), (w//3, h), (200, 200, 200), 1)
    cv2.line(img, (2*w//3, 0), (2*w//3, h), (200, 200, 200), 1)

    if results.multi_hand_landmarks:
        # Extraemos directamente la única mano detectada sin usar bucle
        hand_landmarks = results.multi_hand_landmarks[0]
        
        mp_draw.draw_landmarks(img, hand_landmarks, mp_hands.HAND_CONNECTIONS)
        x4, y4 = int(hand_landmarks.landmark[4].x * w), int(hand_landmarks.landmark[4].y * h)
        x8, y8 = int(hand_landmarks.landmark[8].x * w), int(hand_landmarks.landmark[8].y * h)

        distancia = math.hypot(x8 - x4, y8 - y4)
        cx, cy = (x4 + x8) // 2, (y4 + y8) // 2

        if distancia < 40: # PELLIZCO DETECTADO
            cv2.circle(img, (cx, cy), 15, (0, 255, 0), cv2.FILLED)
            
            if not is_pinching:
                is_pinching = True
                start_x, start_y = cx, cy
            else:
                dx = cx - start_x
                dy = cy - start_y
                comando = ""

                # ARRASTRE HORIZONTAL (Izquierda a Derecha)
                if abs(dx) > umbral_arrastre: 
                    if cy < h // 3: 
                        comando = "U_CW\n" if dx > 0 else "U_CCW\n"
                    elif cy > 2 * h // 3: 
                        comando = "D_CW\n" if dx > 0 else "D_CCW\n"
                    else: 
                        comando = "E_CW\n" if dx > 0 else "E_CCW\n"
                    
                    if comando:
                        client_socket.sendall(comando.encode('utf-8'))
                        start_x, start_y = cx, cy

                # ARRASTRE VERTICAL (Arriba a Abajo)
                elif abs(dy) > umbral_arrastre:
                    if cx < w // 3: 
                        comando = "L_CW\n" if dy > 0 else "L_CCW\n"
                    elif cx > 2 * w // 3: 
                        comando = "R_CW\n" if dy > 0 else "R_CCW\n"
                    else: 
                        comando = "M_CW\n" if dy > 0 else "M_CCW\n"
                    
                    if comando:
                        client_socket.sendall(comando.encode('utf-8'))
                        start_x, start_y = cx, cy 
        else:
            is_pinching = False # Soltó el cubo
    else:
        is_pinching = False # No hay manos en pantalla


    # 2. Exportar la foto limpia directamente a la carpeta de C++
    cv2.imwrite("cpp_engine/temp_bg.jpg", img_clean)
    try:
        os.replace("cpp_engine/temp_bg.jpg", "cpp_engine/fondo_ar.jpg")
    except:
        pass

    cv2.imshow("Camara - Interaccion Tactil", img)
    if cv2.waitKey(1) & 0xFF == 27: break

cap.release()
cv2.destroyAllWindows()
client_socket.close()