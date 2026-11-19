# Presentation Script

English speaking script for [A 3D First-Person Shooter in OpenGL](OpenGL-FPS-White-Blue-EN.pptx). The timestamps give a target of **12 minutes across 11 slides**. The timings are cumulative and include the short walkthrough on Slide 11.

## Slide 1 · A 3D First-Person Shooter in OpenGL (0:00–0:45)

Look at the frame on the right. The blue robot has a shadow on the floor, and its health bar faces us. At the bottom, the ammunition display stays in the same corner wherever we look. Our project for the computer graphics course was to make all of that work in a playable first-person shooter. We used C++ and OpenGL to write the rendering code ourselves. I’ll start with what happens in one round, then follow a robot, a light and a single shot through the program. At the end, we’ll look at how we tested the result.

## Slide 2 · The first wave introduces both robots (0:45–1:35)

A round starts in an enclosed arena. In the first wave, the orange runner moves faster and has fifty health points. The blue robot moves more slowly and has a hundred. Defeating both earns three hundred points in total. After a short delay, the next wave starts and the player receives more ammunition. The health, score and wave number make those changes visible. If the player loses all their health, the final score appears and Enter starts a fresh round. This gives us a simple test for the whole game. We can follow a player from the main menu all the way back to a new round.

## Slide 3 · The technologies behind one frame (1:35–2:50)

The project divides the work among a few small libraries. GLFW creates the window and the OpenGL context. It also gives us keyboard events and mouse movement. Once the context exists, GLAD loads the OpenGL functions that the graphics driver provides. Our C++ code can then upload a mesh, select a shader and issue a draw call. GLSL is the language we use for the programs that run on the GPU. GLM supplies the vectors and matrices for the camera and objects. The image loader, stb_image, reads optional texture files. If one is missing, our code generates a usable surface pattern. Finally, CMake builds the executable and the tests. It fetches GLFW and GLM, while GLAD and stb_image ship with the repository.

## Slide 4 · How a robot is built from simple meshes (2:50–3:50)

Take the blue robot as an example. Its head and legs use cubes, and its arms use cylinders. We can draw the same mesh many times by changing its model matrix. For a single cube, the GPU receives positions, normals and texture coordinates. An index buffer tells it which vertices form each triangle. To animate the robot, we put a rotation at each shoulder and hip. We then place the limb relative to that joint. The joint moves with the robot when its body turns, and the arm still swings around the shoulder. The walking phase follows distance travelled, so the movement of the limbs stays connected to the movement across the floor.

## Slide 5 · How the player sees the arena (3:50–4:55)

Imagine choosing the top corner of a crate. At first that point only has a position inside the unit cube. The model matrix scales the cube and puts the crate in the arena. The view matrix expresses the same point from the player’s eye. We use GLM’s lookAt function to build it. Finally, a perspective matrix produces clip coordinates. After the perspective divide, a distant crate takes up less space on screen. The vertical field of view is seventy-five degrees. Moving the mouse changes yaw and pitch, and pitch stops at eighty-nine degrees in either direction. Walking uses the horizontal direction only. This lets us look up at a column while still moving along the floor.

## Slide 6 · How light reveals a surface (4:55–6:05)

These three spheres use the same shape. The first shows ambient light alone, so its outline is easy to see but its curvature is hard to read. In the middle, diffuse lighting makes the side facing the light brighter. The shader calculates this from the surface normal and the direction to the light. The last sphere adds a highlight. Blinn-Phong uses the halfway direction between the light and the eye, so the highlight moves when the player changes position. The game multiplies the surface texture by its material color. Repeating texture coordinates let one pattern cover a long wall, and mipmaps reduce shimmer at a distance. The sun lights the arena from one direction. Pressing F adds a spotlight that follows the camera.

## Slide 7 · The closest hit decides what a shot touches (6:05–7:15)

The renderer knows the center and size of each box. The game uses those same dimensions to calculate an axis-aligned bounding box, or AABB. For movement, the player checks one axis at a time. If the side of a crate blocks one component of a diagonal step, the other component can still move the player along its edge. A shot uses a different test on the same geometry. The ray starts at the player’s eye. Each pair of box faces gives a range of distances along the ray. If all three ranges overlap in front of the eye, the ray hits the box. The game chooses the closest valid intersection across the arena and the robots. In this example, the crate receives the impact and the robot behind it keeps its health.

## Slide 8 · A second viewpoint creates the shadows (7:15–8:20)

To see where the shadow comes from, imagine looking down at the arena from the sun. During the first pass, the renderer draws the boxes and robot parts into a depth texture. At each sample, it keeps the closest surface to the light. During the camera pass, the shader takes a visible point on the floor and finds its coordinates in that same light view. If another surface was closer, the floor point receives less direct sunlight. The renderer reuses the robot’s animated transforms in both passes, which keeps the shadow aligned with the moving limbs. In a demo, pausing beside a pillar and pressing F6 makes the contribution of this pass easy to see.

## Slide 9 · One shot involves three drawing stages (8:20–9:25)

One shot produces several images with different depth rules. The tracer and impact sparks belong to the arena. Objects closer to the camera can hide them. Each spark burst draws eighteen copies of a small quad in one instanced draw. The shader gives each copy its own motion, and additive blending makes overlapping sparks look brighter. Next the renderer clears depth before it draws the first-person weapon, so the arena wall cannot cut through the model. Finally, the heads-up display uses an orthographic projection. The crosshair stays at the center, and the ammo count stays near the corner. In the picture, one shot has changed the magazine from twelve rounds to eleven. That is the visible link between the game logic and the last drawing pass.

## Slide 10 · What the Windows build achieved (9:25–10:40)

We checked correctness as well as rendering speed. The project records passing physics, combat and wave tests in both Release and Debug builds on Windows. One optimization was to draw letters in the same text run together. In the recorded wave-seven scene this reduced HUD draw calls from fifty-six to twenty. We also compared eight pairs of images, covering four game screens at two sizes. Each pair matched pixel for pixel. The timing figures on the slide come from a separate render benchmark on Intel UHD Graphics 770. With eight robots, shadows and sparks enabled, the median was about two point six milliseconds, and the ninety-fifth percentile was about three point eight. That benchmark waits for the GPU to finish. During a separate live check, the project recorded sixty frames per second at the same resolution.

## Slide 11 · A short walkthrough of the finished game (10:40–12:00)

For a short demonstration, press Enter and move around the nearest crate. Watch its edges shift under perspective while the crosshair stays fixed. Turn toward a wall and press F to show how the flashlight follows the camera. Next, fire once at the practice target and look at the ammunition counter. Press R and watch the magazine refill from the reserve. To show the shadow pass, pause with a column in view and toggle F6. With the game paused, the viewpoint stays identical, so the audience can see exactly what changes. Here in the wave-seven frame, the reload indicator is still readable while several robots occupy different depths in the arena. Our main lesson was that a convincing frame depends on matching the coordinate system and depth rules to each object we draw. Thank you. What questions do you have?

### Presenter cues for the walkthrough

- With the live game, start from the main menu and follow the four controls on Slide 11. Stay on the same camera angle when toggling shadows in the paused view.

- For a slide-only walkthrough, point to the reload indicator and robots in the Slide 11 screenshot. Use the side-by-side frames on Slide 8 to illustrate the shadow toggle.
