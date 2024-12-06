/*
ALPAS

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
#include <time.h>
#include<float.h>
#include<conio.h>
#include<windows.h>
#include<stdarg.h>
#include "alpas-engine.c"
#include "alpas-keybinder.c"

// CONSTANTS
#define MAX_OBJECTS 10
#define TERMINAL_WIDTH 100

// Default Indexes
int carIndex = 5;
int planeIndex = 0;
int fenceIndex_1 = 1;
int fenceIndex_2 = 2;
int fenceIndex_3 = 3;
int fenceIndex_4 = 4;
int obstacleIndex_1 = 6;

// Default Values In-game
int speed = 4;
int startCheck = 0;
int throttle = 30;
int total = 0;
int total_2 = 0;
int obstDistanceTotal = 0;
int lastRand = 0;
int score = 1;
int lastScoreForSpeedIncrement = 0; // score tracking by 4

// Object Structure
typedef struct {
    Vec3 vertices[48];
    int vertexCount;
    int edges[72][2];
    int edgeCount;
    int faces[36][4];
    int faceCount;
    char fillChar;
} Object3D;

// WINDOWS API
void setConsoleSize(int width, int height) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // Set the screen buffer size
    COORD bufferSize = {width, height};
    SetConsoleScreenBufferSize(hConsole, bufferSize);

    // Set the console window size
    SMALL_RECT windowSize = {0, 0, width - 1, height - 1};
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
}

// IN REPLACEMENT OF CONIO'S CLRSCR
void clrscr()
{
    HANDLE hOut;
    COORD Position;

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    Position.X = 0;
    Position.Y = 0;
    SetConsoleCursorPosition(hOut, Position);
}

// FUNCTION PROTOTYPES
void mainMenu();
void winningMenu();
void gameOver();
void xprintc(const char *format, ...) ;
int randomNumber(int, int);
void calculateAABB(Object3D obj, Vec3 *min, Vec3 *max);
int collision(Object3D car, Object3D block);
void rotateVertices(Vec3 vertices[], int count, float angleX, float angleY, float angleZ);
void renderAllObjects(char screen[HEIGHT][WIDTH], Object3D objects[], int objectCount);
void translateObject(Object3D* object, float tx, float ty, float tz);
void performAnimations(Object3D objects[]);
void moveObstacles(Object3D objects[]);
int cameraStart();
int game();
void resetEverything();

// MAIN ENTRY POINT
int main() {
    setConsoleSize(WIDTH + 10, HEIGHT + 10);
    mainMenu();
}

// Random Number Generator
int randomNumber(int min, int max){
    return (rand() % (max - min + 1)) + min;
}

// Function to calculate the Axis-Aligned Bounding Box (AABB) of a 3D object.
// AABB is a rectangular box aligned with the coordinate axes that fully encloses the object.
// Input: (object, min placeholder, max placeholder)
// Output: None
// Side-effect: Modifies data in the address of min and max placeholders
void calculateAABB(Object3D obj, Vec3 *min, Vec3 *max) {
    // Initialize the minimum bounds to the largest possible floating-point value (FLT_MAX),
    // ensuring that any vertex of the object will be smaller and thus update `min`.
    min->x = min->y = min->z = FLT_MAX;

    // Initialize the maximum bounds to the smallest possible floating-point value (-FLT_MAX),
    // ensuring that any vertex of the object will be larger and thus update `max`.
    max->x = max->y = max->z = -FLT_MAX;

    // Loop through each vertex of the 3D object to determine the minimum and maximum bounds.
    for (int i = 0; i < obj.vertexCount; i++) {
        if (obj.vertices[i].x < min->x)
            min->x = obj.vertices[i].x;

        if (obj.vertices[i].y < min->y)
            min->y = obj.vertices[i].y;

        if (obj.vertices[i].z < min->z)
            min->z = obj.vertices[i].z;

        if (obj.vertices[i].x > max->x)
            max->x = obj.vertices[i].x;

        if (obj.vertices[i].y > max->y)
            max->y = obj.vertices[i].y;

        if (obj.vertices[i].z > max->z)
            max->z = obj.vertices[i].z;
    }
}

// Collision Detector
// Input: (car object, block object)
// Output: 1 if collided, 0 if not
int collision(Object3D car, Object3D block) {
    Vec3 carMin, carMax, blockMin, blockMax;

    // Calculate bounding boxes
    calculateAABB(car, &carMin, &carMax);
    calculateAABB(block, &blockMin, &blockMax);

    // Collision Logic (Comparison by Axis)
    if (carMax.x < blockMin.x || carMin.x > blockMax.x) return 0;
    if (carMax.y < blockMin.y || carMin.y > blockMax.y) return 0;
    if (carMax.z < blockMin.z || carMin.z > blockMax.z) return 0;

    return 1;
}

// Rotate vertices based on rotation matrix defined in documentation
// Input: (vertices array, count of vertices, x rotation, y rotation, z rotation)
// Output: None
// Side-effect: Modifies vertices projection
void rotateVertices(Vec3 vertices[], int count, float angleX, float angleY, float angleZ) {
    for (int i = 0; i < count; i++) {
        Vec3 v = vertices[i];

        float y = v.y * cos(angleX) - v.z * sin(angleX);
        float z = v.y * sin(angleX) + v.z * cos(angleX);
        v.y = y;
        v.z = z;

        float x = v.x * cos(angleY) + v.z * sin(angleY);
        z = -v.x * sin(angleY) + v.z * cos(angleY);
        v.x = x;
        v.z = z;

        x = v.x * cos(angleZ) - v.y * sin(angleZ);
        y = v.x * sin(angleZ) + v.y * cos(angleZ);
        v.x = x;
        v.y = y;

        vertices[i] = v;
    }
}

// Renders all Objects (Helper function)
// Input: (Screen Buffer, objects array, objects count)
// Output: None
// Side-effect: Calls renderObject to render each object
void renderAllObjects(char screen[HEIGHT][WIDTH], Object3D objects[], int objectCount) {
    for (int i = 0; i < objectCount; i++) {
        renderObject(screen, objects[i].vertices, objects[i].edges, objects[i].faces, objects[i].edgeCount, objects[i].faceCount, '.', objects[i].fillChar);
    }
}

// Translates an object by an additive numerical value (Helper function)
// Input: (Object, x change, y change, z change)
// Output: None
// Side-effect: Calls translateBox function
void translateObject(Object3D* object, float tx, float ty, float tz) {
    translateBox(object->vertices, object->vertexCount, tx, ty, tz);
}

// Performs default animations on fences
// Input: (objects array) / Reads indexes from global variables
// Output: None
// Side-effect: Manipulates object vertices each time it is called
void performAnimations(Object3D objects[]) {
    int resetDistance = 80;
    int padding = 40;
    // move object backward
    translateObject(&objects[fenceIndex_1], 0, 0, -speed);
    translateObject(&objects[fenceIndex_2], 0, 0, -speed);
    translateObject(&objects[fenceIndex_3], 0, 0, -speed);
    translateObject(&objects[fenceIndex_4], 0, 0, -speed);
    total += speed;
    total_2 += speed;

    if (total >= resetDistance) {
        // Reset location Left & Right Near
        translateObject(&objects[fenceIndex_1], 0, 0, resetDistance + padding);
        translateObject(&objects[fenceIndex_2], 0, 0, resetDistance + padding);

        // Give some padding on checker such that the offset it taken into account for next iteration
        total = -padding;
    }
    if( total_2 >= padding){
        // Reset location Left & Right Far
        translateObject(&objects[fenceIndex_3], 0, 0, resetDistance + padding);
        translateObject(&objects[fenceIndex_4], 0, 0, resetDistance + padding);

        // Give some padding on checker such that the offset it taken into account for next iteration
        total_2 = -padding - padding;
    }
}

// Moves objects (different from performAnimations only on fences)
// Input: Objects Array
// Output: None
// Side Effect: Manipulates object vertices
void moveObstacles(Object3D objects[]) {

    //Generate a random number
    int translationFactor = randomNumber(-10, 10);
    int resetDistance = 80;
    int padding = 40;

    // Check Collision based on collision() return value
   if(collision(objects[carIndex], objects[obstacleIndex_1])){
        gameOver();
   }

    // Translation
    translateObject(&objects[obstacleIndex_1], 0, 0, -speed);
    obstDistanceTotal += speed;

    // Increment level and speed for every time the score reaches multiple of 4
    if(score % 4 == 0 && score != lastScoreForSpeedIncrement) {
        if(speed <= 7) speed += 1; // Limit speed increase to 7 to keep game playable
        else {
            speed = speed;
        }
        // Make sure score is only increased one time
        lastScoreForSpeedIncrement = score;
    }

    // Check if object has traversed road, if so, reset
    if(obstDistanceTotal >= resetDistance) {
        if(lastRand < 0) {
            // Account for previous translation, then offset it
            translateObject(&objects[obstacleIndex_1], lastRand * -1, 0, 0);
        } else {
            translateObject(&objects[obstacleIndex_1], -lastRand, 0, 0);
        }
        lastRand = translationFactor;
        translateObject(&objects[obstacleIndex_1], translationFactor, 0, resetDistance + padding); // Back to far
        obstDistanceTotal = -padding;
        score += 1;
    }
}

// Initial Animation on game start
// Input: None
// Output: 1 when animation is done, 0 when not
int cameraStart(){
    if(!startCheck){
        camera.position.z -= 2;
        throttle -=2;
        if(throttle < 2){
            startCheck = 1;
        }
        return 0;
    }
    else {
        return startCheck;
    }
}


// Main Game
int game(){
    // Reset for each time game is reset or started
    resetEverything();
    // Initialize randomizer seed
    srand((unsigned int)time(NULL));
    // Initialize screen buffer
    char screen[HEIGHT][WIDTH];

    Object3D objects[MAX_OBJECTS];
    int objectCount = 7;

    // For road 3D data
    objects[0] = (Object3D) {
        .vertices = {
            {-20.0, -13.0, -20.0}, {-20.0, -13.0, 120.0},
            {20.0, -13.0, -20.0}, {20.0, -13.0, 120.0}
        },
        .vertexCount = 4,
        .edges = {
            {0, 1}, {1, 3}, {3, 2}, {2, 0}
        },
        .edgeCount = 4,
        .faces = {
            {0, 1, 3, 2}
        },
        .faceCount = 1,
        .fillChar = 'R'
    };

    // For Fence 1
    objects[1] = (Object3D) {
        .vertices = {
            {-20.0, -13.0, 0.0},
            {-20.0, -13.0, 10.0},
            {-20.0, -8.0, 0.0},
            {-20.0, -8.0, 10.0},
            {10.0, -13.0, 0.0},
            {10.0, -13.0, 10.0},
            {10.0, -8.0, 0.0},
            {10.0, -8.0, 10.0}
        },
        .vertexCount = 8,
        .edges = {
            {0, 1}, {1, 3}, {3, 2}, {2, 0},
            {4, 5}, {5, 7}, {7, 6}, {6, 4},
            {0, 4}, {1, 5}, {2, 6}, {3, 7}
        },
        .edgeCount = 12,
        .faces = {
            {0, 1, 3, 2}, {4, 5, 7, 6}, {0, 1, 5, 4},
            {2, 3, 7, 6}, {0, 2, 6, 4}, {1, 3, 7, 5}
        },
        .faceCount = 6,
        .fillChar = 'F'
    };

    // For Fence 2
    objects[2] = (Object3D) {
        .vertices = {
            {-20.0, -13.0, 0.0},
            {-20.0, -13.0, 10.0},
            {-20.0, -8.0, 0.0},
            {-20.0, -8.0, 10.0},
            {10.0, -13.0, 0.0},
            {10.0, -13.0, 10.0},
            {10.0, -8.0, 0.0},
            {10.0, -8.0, 10.0}
        },
        .vertexCount = 8,
        .edges = {
            {0, 1}, {1, 3}, {3, 2}, {2, 0},
            {4, 5}, {5, 7}, {7, 6}, {6, 4},
            {0, 4}, {1, 5}, {2, 6}, {3, 7}
        },
        .edgeCount = 12,
        .faces = {
            {0, 1, 3, 2}, {4, 5, 7, 6}, {0, 1, 5, 4},
            {2, 3, 7, 6}, {0, 2, 6, 4}, {1, 3, 7, 5}
        },
        .faceCount = 6,
        .fillChar = 'F'
    };

    // For Fence 3
    objects[3] = (Object3D) {
        .vertices = {
            {-20.0, -13.0, 0.0},
            {-20.0, -13.0, 10.0},
            {-20.0, -8.0, 0.0},
            {-20.0, -8.0, 10.0},
            {10.0, -13.0, 0.0},
            {10.0, -13.0, 10.0},
            {10.0, -8.0, 0.0},
            {10.0, -8.0, 10.0}
        },
        .vertexCount = 8,
        .edges = {
            {0, 1}, {1, 3}, {3, 2}, {2, 0},
            {4, 5}, {5, 7}, {7, 6}, {6, 4},
            {0, 4}, {1, 5}, {2, 6}, {3, 7}
        },
        .edgeCount = 12,
        .faces = {
            {0, 1, 3, 2}, {4, 5, 7, 6}, {0, 1, 5, 4},
            {2, 3, 7, 6}, {0, 2, 6, 4}, {1, 3, 7, 5}
        },
        .faceCount = 6,
        .fillChar = 'F'
    };

    // For Fence 4
    objects[4] = (Object3D) {
        .vertices = {
            {-20.0, -13.0, 0.0},
            {-20.0, -13.0, 10.0},
            {-20.0, -8.0, 0.0},
            {-20.0, -8.0, 10.0},
            {10.0, -13.0, 0.0},
            {10.0, -13.0, 10.0},
            {10.0, -8.0, 0.0},
            {10.0, -8.0, 10.0}
        },
        .vertexCount = 8,
        .edges = {
            {0, 1}, {1, 3}, {3, 2}, {2, 0},
            {4, 5}, {5, 7}, {7, 6}, {6, 4},
            {0, 4}, {1, 5}, {2, 6}, {3, 7}
        },
        .edgeCount = 12,
        .faces = {
            {0, 1, 3, 2}, {4, 5, 7, 6}, {0, 1, 5, 4},
            {2, 3, 7, 6}, {0, 2, 6, 4}, {1, 3, 7, 5}
        },
        .faceCount = 6,
        .fillChar = 'F'
    };

    // For CAR
    objects[5] = (Object3D) {
        .vertices = {
            {-1.0, -1.0, -1.0}, {-1.0, -1.0, 1.0},
            {-1.0, 1.0, -1.0}, {-1.0, 1.0, 1.0},
            {1.0, -1.0, -1.0}, {1.0, -1.0, 1.0},
            {1.0, 1.0, -1.0}, {1.0, 1.0, 1.0},
            {-2.1562888622283936, -9.606802940368652, -0.413601279258728},
            {-2.1562888622283936, -9.606802940368652, 3.1906843185424805},
            {-2.1562888622283936, -0.46978187561035156, -0.413601279258728},
            {-2.1562888622283936, -0.46978187561035156, 3.1906843185424805},
            {11.308062553405762, -9.606802940368652, -0.413601279258728},
            {11.308062553405762, -9.606802940368652, 3.1906843185424805},
            {11.308062553405762, -0.46978187561035156, -0.413601279258728},
            {11.308062553405762, -0.46978187561035156, 3.1906843185424805},
            {4.00819206237793, -7.550227165222168, 3.0195064544677734},
            {4.00819206237793, -7.550227165222168, 5.399611473083496},
            {4.00819206237793, -2.5663576126098633, 3.0195064544677734},
            {4.00819206237793, -2.5663576126098633, 5.399611473083496},
            {10.97590446472168, -7.550227165222168, 3.0195064544677734},
            {10.97590446472168, -7.550227165222168, 5.399611473083496},
            {10.97590446472168, -2.5663576126098633, 3.0195064544677734},
            {10.97590446472168, -2.5663576126098633, 5.399611473083496},
            {-0.9999997019767761, -11.261319160461426, -0.9999999403953552},
            {-0.9999997019767761, -11.261319160461426, 0.9999999403953552},
            {-0.9999997019767761, -9.261319160461426, -0.9999999403953552},
            {-0.9999997019767761, -9.261319160461426, 0.9999999403953552},
            {1.000000238418579, -11.261319160461426, -0.9999999403953552},
            {1.000000238418579, -11.261319160461426, 0.9999999403953552},
            {1.000000238418579, -9.261319160461426, -0.9999999403953552},
            {1.000000238418579, -9.261319160461426, 0.9999999403953552},
            {7.860820770263672, -0.9999999403953552, -0.9999999403953552},
            {7.860820770263672, -0.9999999403953552, 0.9999999403953552},
            {7.860820770263672, 0.9999999403953552, -0.9999999403953552},
            {7.860820770263672, 0.9999999403953552, 0.9999999403953552},
            {9.860820770263672, -0.9999999403953552, -0.9999999403953552},
            {9.860820770263672, -0.9999999403953552, 0.9999999403953552},
            {9.860820770263672, 0.9999999403953552, -0.9999999403953552},
            {9.860820770263672, 0.9999999403953552, 0.9999999403953552},
            {7.860820770263672, -11.179851531982422, -0.9999999403953552},
            {7.860820770263672, -11.179851531982422, 0.9999999403953552},
            {7.860820770263672, -9.179851531982422, -0.9999999403953552},
            {7.860820770263672, -9.179851531982422, 0.9999999403953552},
            {9.860820770263672, -11.179851531982422, -0.9999999403953552},
            {9.860820770263672, -11.179851531982422, 0.9999999403953552},
            {9.860820770263672, -9.179851531982422, -0.9999999403953552},
            {9.860820770263672, -9.179851531982422, 0.9999999403953552}
        },
        .vertexCount = 48,
        .edges = {
            {2, 0}, {0, 1}, {1, 3}, {3, 2},
            {6, 2}, {3, 7}, {7, 6}, {4, 6},
            {7, 5}, {5, 4}, {0, 4}, {5, 1},
            {10, 8}, {8, 9}, {9, 11}, {11, 10},
            {14, 10}, {11, 15}, {15, 14}, {12, 14},
            {15, 13}, {13, 12}, {8, 12}, {13, 9},
            {18, 16}, {16, 17}, {17, 19}, {19, 18},
            {22, 18}, {19, 23}, {23, 22}, {20, 22},
            {23, 21}, {21, 20}, {16, 20}, {21, 17},
            {26, 24}, {24, 25}, {25, 27}, {27, 26},
            {30, 26}, {27, 31}, {31, 30}, {28, 30},
            {31, 29}, {29, 28}, {24, 28}, {29, 25},
            {34, 32}, {32, 33}, {33, 35}, {35, 34},
            {38, 34}, {35, 39}, {39, 38}, {36, 38},
            {39, 37}, {37, 36}, {32, 36}, {37, 33},
            {42, 40}, {40, 41}, {41, 43}, {43, 42},
            {46, 42}, {43, 47}, {47, 46}, {44, 46},
            {47, 45}, {45, 44}, {40, 44}, {45, 41}
        },
        .edgeCount = 72,
        .faces = {
            {0, 1, 3, 2}, {2, 3, 7, 6}, {6, 7, 5, 4},
            {4, 5, 1, 0}, {2, 6, 4, 0}, {7, 3, 1, 5},
            {8, 9, 11, 10}, {10, 11, 15, 14}, {14, 15, 13, 12},
            {12, 13, 9, 8}, {10, 14, 12, 8}, {15, 11, 9, 13},
            {16, 17, 19, 18}, {18, 19, 23, 22}, {22, 23, 21, 20},
            {20, 21, 17, 16}, {18, 22, 20, 16}, {23, 19, 17, 21},
            {24, 25, 27, 26}, {26, 27, 31, 30}, {30, 31, 29, 28},
            {28, 29, 25, 24}, {26, 30, 28, 24}, {31, 27, 25, 29},
            {32, 33, 35, 34}, {34, 35, 39, 38}, {38, 39, 37, 36},
            {36, 37, 33, 32}, {34, 38, 36, 32}, {39, 35, 33, 37},
            {40, 41, 43, 42}, {42, 43, 47, 46}, {46, 47, 45, 44},
            {44, 45, 41, 40}, {42, 46, 44, 40}, {47, 43, 41, 45}
        },
        .faceCount = 36,
        .fillChar = 'P'
    };

    // For Object
    objects[6] = (Object3D) {
        .vertices = {
            {-20.0, -10.0, 0.0},
            {-20.0, -10.0, 2.0},
            {-20.0, -8.0, 0.0},
            {-20.0, -8.0, 2.0},
            {-18.0, -10.0, 0.0},
            {-18.0, -10.0, 2.0},
            {-18.0, -8.0, 0.0},
            {-18.0, -8.0, 2.0}
        },
        .vertexCount = 8,
        .edges = {
            {0, 1}, {1, 3}, {3, 2}, {2, 0},
            {4, 5}, {5, 7}, {7, 6}, {6, 4},
            {0, 4}, {1, 5}, {2, 6}, {3, 7}
        },
        .edgeCount = 12,
        .faces = {
            {0, 1, 3, 2}, {4, 5, 7, 6}, {0, 1, 5, 4},
            {2, 3, 7, 6}, {0, 2, 6, 4}, {1, 3, 7, 5}
        },
        .faceCount = 6,
        .fillChar = 'O'
    };


    // Rotate the car object initially, rotate x and y by pi/2 radians
    rotateVertices(objects[carIndex].vertices, objects[carIndex].vertexCount, M_PI / 2, M_PI / 2, 0);
    rotateVertices(objects[fenceIndex_1].vertices, objects[fenceIndex_1].vertexCount, M_PI / 2, M_PI / 2, 0);
    rotateVertices(objects[fenceIndex_2].vertices, objects[fenceIndex_2].vertexCount, M_PI / 2, M_PI / 2, 0);
    rotateVertices(objects[fenceIndex_3].vertices, objects[fenceIndex_3].vertexCount, M_PI / 2, M_PI / 2, 0);
    rotateVertices(objects[fenceIndex_4].vertices, objects[fenceIndex_4].vertexCount, M_PI / 2, M_PI / 2, 0);
    rotateVertices(objects[obstacleIndex_1].vertices, objects[obstacleIndex_1].vertexCount, M_PI / 2, M_PI / 2, 0);

    // Initial positions of objects
    translateObject(&objects[planeIndex], 0, 15, 0);
    translateObject(&objects[carIndex], 0, 0, -5);
    translateObject(&objects[fenceIndex_1], -10, 0, 40); //right fence
    translateObject(&objects[fenceIndex_2], 32, 0, 40); //left fence

    translateObject(&objects[fenceIndex_3], -10, 0, -20); //right fence 2
    translateObject(&objects[fenceIndex_4], 32, 0, -20); //left fence 2

    //remove bias
    translateObject(&objects[obstacleIndex_1], 10, 0,20 );

    //initial animation
    camera.position.z += 30;

    // Keep track of x traversal of player
    int xTraversal = 0;
    int breakAttempt_L = 0; // Break attempts for winning condition
    int breakAttempt_R = 0;

    while (1) {
        // Only move obstacles once camera is done with animation
        if (cameraStart()) moveObstacles(objects);

        // Winning condition
        if(breakAttempt_L >= 20 || breakAttempt_R >= 20){
            winningMenu();
        }

        // Call perform animations to start animation
        performAnimations(objects);
        clearScreen(screen);

        // Render all objects
        renderAllObjects(screen, objects, objectCount);
        renderScreen(screen);

        // Display controls
        xprintc("\033[0;32mScore: %d\033[0m", score);
        xprintc("\033[0;33mLevel: %d\033[0m", (score / 4) + 1);

        xprintc("[a] - left   [d] - right    [w] - forward    [s] - back    [q] - quit");

        // own getch to handle multiple cases
        char input = mygetch();
        int target = carIndex;

        // How much the player shifts based on keystroke
        int shift_val = 9;

        // Key bindings
        if (input == 'w') translateObject(&objects[target], 0, 0, shift_val);
        if (input == 's') translateObject(&objects[target], 0, 0, -shift_val);
        if (input == 'd' ) {
            // Check if allowed to move onto the right
            if(xTraversal <= 0){
                translateObject(&objects[target], -shift_val, 0, 0);
                breakAttempt_L = 0;
                xTraversal++;
            }
            else {
                breakAttempt_R++;
            }
        }
        if (input == 'a') {
            // Check if allowed to move onto the left
            if(xTraversal >= -1){
                translateObject(&objects[target], shift_val, 0, 0);
                breakAttempt_R = 0;
                xTraversal--;
            }
            else {
                breakAttempt_L++;
            }
        }

        // Camera Movements
        if (input == 'p') camera.position.z += 0.5;
        if (input == ';') camera.position.z -= 0.5;
        if (input == 'l') camera.position.y += 4;
        if (input == '\'') camera.position.y -= 4;

        // Main Menu
        if (input == 'q') mainMenu();

        Sleep(30);

        // Clear the terminal for rendering
        #ifdef _WIN32
            clrscr();
        #else
            //system("clear");
        #endif
    }

    return 0;
}

// Custom function for centering text
void xprintc(const char *format, ...) {
    char buffer[1024];
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    int text_length = strlen(buffer);
    if (text_length >= TERMINAL_WIDTH) {
        printf("%s\n", buffer);
        return;
    }

    int padding = (TERMINAL_WIDTH - text_length) / 2;

    for (int i = 0; i < padding; i++) {
        putchar(' ');
    }
    printf("%s\n", buffer);
}

// General clearing of terminal outside of game proper
void clearTerminal(){
    #ifdef _WIN32
            system("cls");
    #else
            system("clear");
    #endif
}

// Game over menu
void gameOver(){
    clearTerminal();
    printf("\n\n\n\n\n\n");
     xprintc("\033[0;36m       ________  ___       ________  ________  ________      \033[0m");
    xprintc("\033[0;36m      |\\   __  \\|\\  \\     |\\   __  \\|\\   __  \\|\\   ____\\     \033[0m");
    xprintc("\033[0;36m      \\ \\  \\|\\  \\ \\  \\    \\ \\  \\|\\  \\ \\  \\|\\  \\ \\  \\___|_    \033[0m");
    xprintc("\033[0;36m       \\ \\   __  \\ \\  \\    \\ \\   ____\\ \\   __  \\ \\_____  \\   \033[0m");
    xprintc("\033[0;36m        \\ \\  \\ \\  \\ \\  \\____\\ \\  \\___|\\ \\  \\ \\  \\|____|\\  \\  \033[0m");
    xprintc("\033[0;36m         \\ \\__\\ \\__\\ \\_______\\ \\_\\    \\ \\__\\ \\__\\____\\_\\  \\ \033[0m");
    xprintc("\033[0;36m          \\|__|\\|__|\\|_______|\\|__|     \\|__|\\|__|\\_________\\\033[0m");
    xprintc("\033[0;36m                                                 \\|_________|\033[0m");
    printf("\n");
    xprintc("Game over... Perhaps losing gives meaning to life?");
    xprintc("Do you really believe this is an endless game?");
    printf("\n\n\n\n\n");
    xprintc("[r] Restart      [x] Exit     [M] Menu");
    printf("\n\n\n");
    char choice;
    getchar();
    printf("Enter choice: ");
    scanf("%c", &choice);

    switch(choice){
        case 'r': {
            system("cls");
            game();
            break;
        }
        case 'x': {
            printf("Terminating...\n");
            exit(0);
            break;
        }
        default: {
            mainMenu();
        }
    }

}

// Winning menu
void winningMenu(){

    clearTerminal();
    printf("\n\n\n\n\n\n");
    xprintc("\033[0;36m       ________  ___       ________  ________  ________      \033[0m");
    xprintc("\033[0;36m      |\\   __  \\|\\  \\     |\\   __  \\|\\   __  \\|\\   ____\\     \033[0m");
    xprintc("\033[0;36m      \\ \\  \\|\\  \\ \\  \\    \\ \\  \\|\\  \\ \\  \\|\\  \\ \\  \\___|_    \033[0m");
    xprintc("\033[0;36m       \\ \\   __  \\ \\  \\    \\ \\   ____\\ \\   __  \\ \\_____  \\   \033[0m");
    xprintc("\033[0;36m        \\ \\  \\ \\  \\ \\  \\____\\ \\  \\___|\\ \\  \\ \\  \\|____|\\  \\  \033[0m");
    xprintc("\033[0;36m         \\ \\__\\ \\__\\ \\_______\\ \\_\\    \\ \\__\\ \\__\\____\\_\\  \\ \033[0m");
    xprintc("\033[0;36m          \\|__|\\|__|\\|_______|\\|__|     \\|__|\\|__|\\_________\\\033[0m");
    xprintc("\033[0;36m                                                 \\|_________|\033[0m");
    printf("\n");
    xprintc("\033[0;32m       Congratulations, you win!\033[0m");

    printf("\n");
    xprintc("You see, thinking outside the box allow us to overcome");
    xprintc("what may seem like an endless obstacle of life — but now what?");
    xprintc("__________________________");
    printf("\n");
    xprintc("\033[0;35m       The thrill of chasing victory fades when the pursuit becomes routine.\033[0m");
    xprintc("\033[0;35m       Winning without meaning can turn triumph into monotony.\033[0m");
    printf("\n\n\n\n\n");
    xprintc("[x] Exit     [M] Menu");
    printf("\n\n\n");
    char choice;
    getchar();
    printf("Enter choice: ");
    scanf("%c", &choice);

    switch(choice){
        case 'r': {
            system("cls");
            game();
            break;
        }
        case 'x': {
            printf("Terminating...\n");
            exit(0);
            break;
        }
        default: {
            mainMenu();
        }
    }

}

// Main menu
void mainMenu(){
    clearTerminal();
    printf("\n\n\n\n\n\n");
    xprintc("Welcome to");
    xprintc("\033[0;36m       ________  ___       ________  ________  ________      \033[0m");
    xprintc("\033[0;36m      |\\   __  \\|\\  \\     |\\   __  \\|\\   __  \\|\\   ____\\     \033[0m");
    xprintc("\033[0;36m      \\ \\  \\|\\  \\ \\  \\    \\ \\  \\|\\  \\ \\  \\|\\  \\ \\  \\___|_    \033[0m");
    xprintc("\033[0;36m       \\ \\   __  \\ \\  \\    \\ \\   ____\\ \\   __  \\ \\_____  \\   \033[0m");
    xprintc("\033[0;36m        \\ \\  \\ \\  \\ \\  \\____\\ \\  \\___|\\ \\  \\ \\  \\|____|\\  \\  \033[0m");
    xprintc("\033[0;36m         \\ \\__\\ \\__\\ \\_______\\ \\_\\    \\ \\__\\ \\__\\____\\_\\  \\ \033[0m");
    xprintc("\033[0;36m          \\|__|\\|__|\\|_______|\\|__|     \\|__|\\|__|\\_________\\\033[0m");
    xprintc("\033[0;36m                                                 \\|_________|\033[0m");
    printf("\n");
    xprintc("Your console has been initialized to suggested size.");
    xprintc("If graphics are weird, set fullscreen and zoom out. (Windows Limitation)");
    printf("\n\n\n\n\n");
    xprintc("[s] Start      [x] Exit     ");
    printf("\n\n\n");
    char choice;
    printf("Enter choice: ");
    scanf("%c", &choice);

    switch(choice){
        case 's': {
            system("cls");
            game();
            break;
        }
        case 'x': {
            printf("Terminating...\n");
            exit(0);
            break;
        }
        default: {
            mainMenu();
        }
    }
}

// Reset defaults
void resetEverything(){
    camera.position.x = 0;
    camera.position.y = -20;
    camera.position.z = -30;
    speed = 4;
    startCheck = 0;
    throttle = 30;
    total = 0;
    total_2 = 0;
    obstDistanceTotal = 0;
    lastRand = 0;
    score = 1;
    lastScoreForSpeedIncrement = 0;// score tracking by 4
}
