# UTeM-Stadium-Simulation
A 3D interactive stadium environment built using C++ and OpenGL (GLUT).

<img width="988" height="617" alt="screenshot" src="https://github.com/user-attachments/assets/03038c78-7462-4a89-a930-92591cccee04" />

## Features
- Dynamic Stadium Geometry: Procedurally generated stands, seating rows, stairs, lighting poles, and a multi-lane running track.
- Humanoid Controller: A playable character with integrated physics (gravity/jumping) and limb-swinging animations that scale with movement.
- Custom Collision Engine: Static environment colliders using AABB logic to prevent the player from walking through walls or stands.
- Hybrid Camera System: 3rd-person orbital camera supporting both keyboard rotation and smooth mouse-look functionality (X and Y axis).
- Debug Mode: Real-time visualization of collision boxes for environment testing.
- Lighting & Materials: Global ambient and point-source lighting with material properties applied to different stadium surfaces.

## Controls
| Key | Action |
| -------- | -------- |
| W/S   | Move Forward/Backward   |
| A/D   | Turn Camera Left / Right  |
| Space  | Jump  |
| Mouse   | Look Around (Orbital Pitch & Yaw)  |
| M | Toggle Mouse Lock  |
| B   | Toggle Debug Collision Boxes  |
| F   | Toggle Fullscreen Mode  |
| ESC   | Exit Application  |

## Setup
1. Set up OpenGL: https://www.wikihow.com/Set-Up-OpenGL%E2%80%90GLFW%E2%80%90GLAD-on-a-Project-with-Visual-Studio
2. Clone the repository into the GLP folder
3. Run with x86 debug mode
