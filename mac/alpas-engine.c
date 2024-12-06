/*
ALPAS - ENGINE HELPER

Uyvico, Von Zedrich M.
Valeriano, Lander
Capawing, Armeliz
Marces, Rey Ann

December 6, 2024

*/

// LIBRARIES
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// CONSTANTS
#define WIDTH 100
#define HEIGHT 60
#define FOV 90

// Structures
typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    Vec3 position;
    float pitch;
} Camera;

// Initialize camera position and tilt
Camera camera = {{0, -20, -30}, -0.3f}; 

// Clear Screen Buffer
void clearScreen(char screen[HEIGHT][WIDTH]) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            screen[y][x] = ' ';
        }
    }
}

// Render Screen based on fillChar stored on Object3
// Associate ANSI codes with fillChar value
void renderScreen(char screen[HEIGHT][WIDTH]) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (screen[y][x] == 'C') {
                printf("\033[1;32m%c\033[0m", '*'); 
            }
            else if(screen[y][x] == 'P'){
                printf("\033[1;35m%c\033[0m", '*'); 
            } 
            else if(screen[y][x] == 'R'){
                printf("\033[1;34m%c\033[0m", '*'); 
            } 
            else if(screen[y][x] == 'F'){
                printf("\033[1;36m%c\033[0m", '*'); 
            } 
            else if(screen[y][x] == 'O'){
                printf("\033[1;31m%c\033[0m", '*'); 
            } 
            else {
                printf("\033[1;30m%c\033[0m", screen[y][x]); 
                //putchar(screen[y][x]);
            }
        }
        putchar('\n');
    }
}

// Applies camera tilt based on rotation matrix
Vec3 applyCameraTilt(Vec3 point, float pitch) {
    Vec3 result;
    result.x = point.x;
    result.z = point.z * cosf(pitch) - point.y * sinf(pitch);
    result.y = point.z * sinf(pitch) + point.y * cosf(pitch);
    return result;
}

// Calculates projected 2D (x,y) ordered pair based on (x,y,z)
Vec3 project(Vec3 point) {
    float aspectRatio = 1;//(float)WIDTH / (float)HEIGHT;
    float fovFactor = 1.0f / tanf((FOV * 0.5f) * (M_PI / 180.0f));
    Vec3 projected;

    projected.x = point.x / -point.z * fovFactor * aspectRatio;
    projected.y = point.y / -point.z * fovFactor;
    projected.z = point.z;

    return projected;
}

// Helper function
// Maps out projected (x,y) to the constraints of the screen
void toScreen(Vec3 projected, int *x, int *y) {
    *x = (int)((projected.x + 1) * 0.5f * WIDTH);
    *y = (int)((1 - projected.y) * 0.5f * HEIGHT);
}

// Function to draw a straight line on a 2D screen using Bresenham's line algorithm.
// `screen` is a 2D character array representing the screen.
// `x0`, `y0` are the starting coordinates of the line.
// `x1`, `y1` are the ending coordinates of the line.
// `charType` is the character used to draw the line.
void drawLine(char screen[HEIGHT][WIDTH], int x0, int y0, int x1, int y1, char charType) {
    // Calculate the absolute differences between the starting and ending points.
    int dx = abs(x1 - x0); // Change in x
    int sx = x0 < x1 ? 1 : -1; // Step direction for x (1 if increasing, -1 if decreasing)

    int dy = -abs(y1 - y0); // Change in y (negative because of how the algorithm works)
    int sy = y0 < y1 ? 1 : -1; // Step direction for y (1 if increasing, -1 if decreasing)

    // Initialize the error term, which determines when to step in the x or y direction.
    int err = dx + dy; // Sum of dx and dy as the initial error term.
    int e2; // Temporary variable to store the scaled error term.

    // Loop until the line's end point is reached.
    while (1) {
        // Plot the point if it's within the screen bounds.
        if (x0 >= 0 && x0 < WIDTH && y0 >= 0 && y0 < HEIGHT) {
            screen[y0][x0] = charType; // Set the screen pixel to the specified character.
        }

        // Break the loop if the current point matches the end point.
        if (x0 == x1 && y0 == y1) 
            break;

        // Update the error term and coordinates to move towards the end point.
        e2 = 2 * err; // Scale the error term to determine the next step.

        // If the error term suggests a horizontal move is needed, adjust x and the error term.
        if (e2 >= dy) { 
            err += dy; 
            x0 += sx; 
        }

        // If the error term suggests a vertical move is needed, adjust y and the error term.
        if (e2 <= dx) { 
            err += dx; 
            y0 += sy; 
        }
    }
}


// Function to fill the face of a 3D object using the Scanline Fill Algorithm.
// `screen` is a 2D character array representing the display screen.
// `vertices` is an array of 3D vertices for the object.
// `vertexIndices` is an array of indices representing the vertices that form the face to fill.
// `numVertices` is the number of vertices in the face.
// `fillChar` is the character used to fill the face.
void fillFace(char screen[HEIGHT][WIDTH], Vec3 vertices[], int vertexIndices[], int numVertices, char fillChar) {
    // Arrays to hold the 2D screen coordinates of the face's vertices.
    int screenX[numVertices], screenY[numVertices];

    // Transform each 3D vertex to 2D screen space.
    for (int i = 0; i < numVertices; i++) {
        Vec3 vertex = vertices[vertexIndices[i]]; // Get the vertex using its index.

        // Apply camera transformations to the vertex.
        vertex.x -= camera.position.x;  // Translate by the camera's position.
        vertex.y -= camera.position.y;
        vertex.z -= camera.position.z;

        // Apply camera tilt (rotation around the horizontal axis).
        vertex = applyCameraTilt(vertex, camera.pitch);

        // Only process vertices in front of the camera (positive z-coordinate).
        if (vertex.z > 0) {
            // Project the 3D vertex into 2D space.
            Vec3 projected = project(vertex);

            // Convert the 2D projected coordinates into screen coordinates.
            toScreen(projected, &screenX[i], &screenY[i]);
        } else {
            // If any vertex is behind the camera, stop filling this face.
            return;
        }
    }

    // Begin the Scanline Fill Algorithm.
    for (int y = 0; y < HEIGHT; y++) {
        // Initialize the minimum and maximum x-coordinates for the current scanline.
        int minX = WIDTH, maxX = 0;

        // Iterate through each edge of the face to determine intersections with the scanline.
        for (int i = 0; i < numVertices; i++) {
            // Get the start and end points of the current edge.
            int j = (i + 1) % numVertices; // The next vertex index, wrapping around to the start.

            int x1 = screenX[i], y1 = screenY[i];
            int x2 = screenX[j], y2 = screenY[j];

            // Check if the scanline intersects the current edge.
            if ((y1 < y && y2 >= y) || (y2 < y && y1 >= y)) {
                // Calculate the x-coordinate of the intersection using linear interpolation.
                int x = x1 + (y - y1) * (x2 - x1) / (y2 - y1);

                // Update the minimum and maximum x-coordinates for the scanline.
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
            }
        }

        // Fill the pixels between minX and maxX on the current scanline.
        for (int x = minX; x <= maxX; x++) {
            // Ensure the pixel is within the screen bounds.
            if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                screen[y][x] = fillChar; // Set the pixel to the fill character.
            }
        }
    }
}

// Translate Box by a certain additive value
void translateBox(Vec3 box[], int numVertices, float dx, float dy, float dz) {
    for (int i = 0; i < numVertices; i++) {
        box[i].x += dx;
        box[i].y += dy;
        box[i].z += dz;
    }
}


// Render the cube's edges and faces
void renderObject(char screen[HEIGHT][WIDTH], Vec3 points[], int edges[][2], int faces[][4], int numEdges, int numFaces, char fillChar, char fillChar2) {
    for (int i = 0; i < numFaces; i++) {
        fillFace(screen, points, faces[i], 4, fillChar);
    }

    for (int i = 0; i < numEdges; i++) {
        Vec3 p1 = points[edges[i][0]];
        Vec3 p2 = points[edges[i][1]];

        p1.x -= camera.position.x;
        p1.y -= camera.position.y;
        p1.z -= camera.position.z;
        p2.x -= camera.position.x;
        p2.y -= camera.position.y;
        p2.z -= camera.position.z;

        // Apply camera pitch (tilt)
        p1 = applyCameraTilt(p1, camera.pitch);
        p2 = applyCameraTilt(p2, camera.pitch);

        if (p1.z > 0 && p2.z > 0) { 
            Vec3 projP1 = project(p1);
            Vec3 projP2 = project(p2);

            int x1, y1, x2, y2;
            toScreen(projP1, &x1, &y1);
            toScreen(projP2, &x2, &y2);

            drawLine(screen, x1, y1, x2, y2, fillChar2);
        }
    }
}

