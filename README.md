# Gesture-Controlled 3D Rubik's Cube

A webcam-based prototype that combines hand tracking in Python with a virtual Rubik's cube rendered in C++. Users pinch their thumb and index finger and drag their hand to request layer movements.

**Project status:** the main interaction pipeline is implemented in the source, but cube-turn correctness and communication robustness still require fixes. This is a progress prototype, not a complete cube simulator or automatic solver.

## Project overview

The application has two programs:

1. **Python client:** OpenCV captures and mirrors webcam frames. MediaPipe detects one hand. A pinch-and-drag interpreter generates movement commands.
2. **C++ engine:** a TCP listener receives commands, a `RubiksCube` object stores sticker colors, and raylib draws a 3D cube over the latest camera image.

Commands travel through TCP port **65432**. Camera images follow a separate path: Python replaces `cpp_engine/fondo_ar.jpg`, and C++ loads that JPEG as its background texture.

The video overlay has no physical-world anchoring, depth-aware occlusion, or recognition of a physical Rubik's cube.

## Implemented components

| Component | Current implementation |
| --- | --- |
| Camera input | Webcam index 0, mirrored frames, and an OpenCV tracking window. |
| Hand tracking | One hand; detection and tracking confidence thresholds of 0.7. |
| Gesture recognition | Pinch distance below 40 pixels and drag displacement above 60 pixels. |
| Command transmission | UTF-8, newline-terminated text through a local TCP connection. |
| Cube representation | Six fixed-size 3×3 color matrices, initialized to a solved state. |
| State operations | Face-matrix rotation, neighboring-edge transfers, and E/M middle-slice functions. |
| Rendering | 800×600 window, perspective projection, orbital camera, gray core, and 54 colored stickers. |
| Camera background | Temporary JPEG replaced by `fondo_ar.jpg` and loaded by the C++ renderer. |

The renderer sets a target of **60 FPS**; this is a configuration value, not a measured performance result. Sticker colors update immediately, without animated layer turns.

## Main source files

| Path | Responsibility |
| --- | --- |
| `python_client/hand_tracker.py` | Camera capture, landmarks, gestures, TCP client, and JPEG export. |
| `cpp_engine/main.cpp` | TCP server, command dispatch, and graphics loop. |
| `cpp_engine/RubiksCube.h` | Face/color definitions, fixed-size arrays, and class interface. |
| `cpp_engine/RubiksCube.cpp` | Cube initialization and state-transformation functions. |
| `cpp_engine/raylib/` | Third-party graphics library, when included in the repository. |

## Requirements

- Linux desktop environment with an accessible webcam and OpenGL support.
- C++ compiler, Make, and the raylib build dependencies.
- Python with OpenCV and a MediaPipe release that provides `mp.solutions.hands`.

**Native Windows execution is not currently supported by the supplied C++ networking code.** It uses POSIX headers such as `sys/socket.h`, `netinet/in.h`, and `unistd.h`. Git Bash can upload the project to GitHub, but does not make that code a native Windows application. The included VS Code Windows build task also lacks the complete multi-file/raylib build configuration.

The instructions below provide a suggested Ubuntu setup. They have not been verified through a live webcam demonstration during documentation preparation. The original environment's exact dependency versions were not established.

## Installation on Ubuntu

### 1. Install build dependencies

```bash
sudo apt update
sudo apt install build-essential python3-venv \
  libasound2-dev libgl1-mesa-dev libx11-dev libxrandr-dev \
  libxi-dev libxcursor-dev libxinerama-dev
```

Consult the [official raylib Linux guide](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux) if your distribution requires additional packages.

### 2. Create the Python environment

From the project root, use a Python 3.10–3.12 installation for the suggested legacy MediaPipe baseline:

```bash
python3 --version
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install "mediapipe==0.10.21"
```

This pin is a suggested compatibility baseline, not a recovered version from the original project. MediaPipe's dependencies provide an OpenCV distribution with `cv2`; do not install several competing OpenCV packages in the same environment.

Verify the APIs used by the client:

```bash
python -c "import cv2; import mediapipe as mp; print(cv2.__version__); print(mp.__version__); print(mp.solutions.hands.Hands)"
```

If this fails, resolve the Python/API compatibility issue before running the client. Package availability depends on Python version and platform.

### 3. Build raylib and the C++ engine

If `cpp_engine/raylib` contains the supplied raylib source, rebuild it for your machine instead of relying on binary files from the archive:

```bash
cd cpp_engine
make -C raylib/src PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=STATIC

g++ -std=c++14 main.cpp RubiksCube.cpp -o motor_rubik \
  -Iraylib/src -Lraylib/src \
  -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```

These commands assume the bundled raylib Makefile supports `PLATFORM_DESKTOP`. If you use a different release or backend, follow that release's build instructions and adjust the include/library paths accordingly.

If raylib is missing, obtain it from the [official repository](https://github.com/raysan5/raylib). Keep its license and attribution. Do not commit its nested `.git` directory, virtual environments, or compiled binaries.

## Running the application

Use **two terminals**. The working directories matter because the source uses relative image paths.

### Terminal 1: C++ engine

From the project root:

```bash
cd cpp_engine
./motor_rubik
```

Start this program first so the server is available when Python connects.

### Terminal 2: Python client

From the project root:

```bash
source .venv/bin/activate
python python_client/hand_tracker.py
```

Run Python from the **project root**, not from `python_client`, so its `cpp_engine/` output paths resolve correctly. The client connects to `127.0.0.1:65432`; the programs are configured to run on the same computer.

## Gesture controls

1. Present one hand to the webcam.
2. Bring the thumb and index fingertip together: a distance below 40 pixels activates a pinch.
3. Drag the midpoint more than 60 pixels horizontally or vertically.
4. Release the pinch to clear the gesture state.

| Drag direction | Current midpoint region | Selected layer |
| --- | --- | --- |
| Horizontal | Top third | U: upper layer |
| Horizontal | Middle third | E: middle horizontal slice |
| Horizontal | Bottom third | D: lower layer |
| Vertical | Left third | L: left layer |
| Vertical | Middle third | M: middle vertical slice |
| Vertical | Right third | R: right layer |

Positive horizontal displacement sends a `CW` suffix; negative displacement sends `CCW`. For vertical dragging, positive displacement means moving down in image coordinates and also sends `CW`.

Horizontal recognition takes priority when both axes exceed the threshold. After sending a command, the client updates the reference midpoint, so a sustained pinch can trigger multiple moves. Region selection uses the camera grid rather than the projected cube geometry. Command suffixes do not guarantee correct standard cube turns in the current implementation.

Press **Escape in the Python window** to stop the client. Close the C++ window to end the graphics application.

## Known limitations

- Outer-layer gesture commands call edge-transfer functions without consistently rotating the selected face matrix.
- `R_CCW` and `L_CCW` also match later command branches, causing an additional edge transfer.
- The counterclockwise branch of `shiftEdgesFront` is empty.
- The network thread modifies cube state while the rendering thread reads it without synchronization, introducing a C++ data race.
- The TCP receiver assumes too much about read boundaries: it does not correctly buffer fragmented commands or process every line in a combined read.
- The server accepts one connection and does not implement a reconnection loop.
- Fixed pixel thresholds depend on resolution and apparent hand size. No recognition-accuracy measurements are available.
- JPEG writing/loading and texture recreation have not been benchmarked.
- Automatic solving, scrambling, reset controls, and move history are not implemented in the reviewed source.

## Troubleshooting

| Problem | What to check |
| --- | --- |
| Python cannot connect | Start C++ first; ensure port 65432 is available. After a client disconnects, restart the engine. |
| No webcam image | Check camera availability and permissions. Camera index 0 may need adjustment. |
| Cube has no live background | Check working directories: C++ in `cpp_engine`, Python in the project root. |
| MediaPipe has no `solutions` attribute | Check that the environment contains a compatible legacy MediaPipe API, not an incompatible newer release or a local file named `mediapipe.py`. |
| Missing raylib headers or library | Build raylib and verify the compiler's `-I` and `-L` paths. |
| Cube turns are inconsistent | Review the known transformation and dispatch defects; this is not necessarily a tracking problem. |

## Next steps

1. Correct and centralize outer-face command dispatch.
2. Validate each move against known sticker permutations, including inverse moves and four repeated quarter turns.
3. Process commands through a synchronized queue in the graphics thread.
4. Add buffered TCP framing, error handling, reconnection, and controlled shutdown.
5. Measure gesture reliability, frame rate, and command-to-display latency.
6. Improve video exchange and calibrate interaction regions to the rendered view.

## Attribution and references

- [raylib](https://github.com/raysan5/raylib): graphics library; preserve its upstream license.
- [OpenCV](https://opencv.org/): webcam processing and image I/O.
- [MediaPipe](https://github.com/google-ai-edge/mediapipe): hand landmarks.
- [MediaPipe 0.10.21 package](https://pypi.org/project/mediapipe/0.10.21/): suggested legacy API baseline.

No license for the team's application code was supplied with the reviewed source. Obtain the authors' agreement before assigning one.
