#include <list>
#include <vector> 
#include <gl/glut.h>
#include <iostream>
#include <cmath> 

using namespace std;

#define BOTTOM 0
#define TOP    1
#define LEFT   2
#define RIGHT  3
#define BACK   4
#define FRONT  5

// --- COLLISION STRUCTURES ---
struct AABB {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
};

std::vector<AABB> staticColliders; // List of all static environment boxes
bool showColliders = false;        // Toggle for debug view
bool keyB = false;                 // Toggle key state

// stadium constants
const GLfloat standLength = 20.0;
const GLfloat standWidth = 5.0;
const GLfloat standBaseHeight = 5.0;
const GLfloat standRowHeight = 1.0;
const GLfloat standRowWidth = 1.5;
const GLfloat standsSeparationDistance = 4.0;
const int standRowCount = 5;
const GLfloat wallLength = 1.0;
const GLfloat wallHeight = standBaseHeight + standRowCount * standRowHeight + 8.0;
const GLfloat platformLength = standsSeparationDistance + 1.0;
const GLfloat platformWidth = 12.0;
const GLfloat stairsLength = 1.0;
const GLfloat stairsWidth = (platformWidth - standWidth) / 2;
const int stairsRowCount = 5;
const int seatInARowCount = 18;
const GLfloat seatGap = 0.2;
const GLfloat seatLength = ((standLength - 2.0) - (seatInARowCount - 1) * seatGap) / seatInARowCount;
const GLfloat seatHeight = 0.7;
const GLfloat seatWidth = 0.5;
const GLfloat roofLength = 2 * standLength + standsSeparationDistance + 4.0;
const GLfloat roofHeight = 1.0;
const GLfloat roofWidth = platformWidth + 2.0;
const GLfloat roofAngle = -10.0;
const GLfloat outerWallWidth = standRowWidth;
const GLfloat blockLength = standsSeparationDistance - 2 * wallLength;
const GLfloat blockHeight = (standRowCount + 3) * standRowHeight + 5.0;
const int poleCount = 2;
const GLfloat poleRadius = 0.5;
const GLfloat poleHeight = wallHeight + 3.0;
const GLint poleSlices = 32;
const GLint poleVSubDiv = 1;
const GLfloat frameAngleAroundY = 30.0;
const GLfloat frameAngleAroundX = 15.0;
const GLfloat frameLength = 7.0;
const GLfloat frameHeight = 4.0;
const GLfloat frameWidth = poleRadius;
const int lightRowCount = 2;
const GLfloat lightGap = 0.5;
const GLfloat lightLength = frameLength - 2.0;
const GLfloat lightHeight = 1.0;
const GLfloat lightWidth = 0.2;
const int laneCount = 5;
const GLfloat laneWidth = 1.0;
const int laneLineCount = laneCount + 1;
const GLfloat laneLineWidth = 0.5;
const GLfloat outermostLaneLineRadiusX = roofLength / 2;
const GLfloat outermostLaneLineRadiusZ = outermostLaneLineRadiusX / 1.5;
const GLfloat centerPitchRadiusX = outermostLaneLineRadiusX - laneCount * laneWidth - laneLineCount * laneLineWidth;
const GLfloat centerPitchRadiusZ = outermostLaneLineRadiusZ - laneCount * laneWidth - laneLineCount * laneLineWidth;

// ground
const GLfloat groundLength = 60.0;
const GLfloat groundWidth = 60.0;

// boundaries
const GLfloat worldMinX = -groundLength / 2;
const GLfloat worldMaxX = groundLength / 2;
const GLfloat worldMinZ = -groundWidth / 2;
const GLfloat worldMaxZ = groundWidth / 2;
const GLfloat worldMinY = 0.0f;

// init 1x1x1 cuboid for duplication
GLfloat unitCuboidVertices[8][3] = {
    {-0.5f, 0.5f, -0.5f}, {-0.5f,  0.5f, 0.5f}, {-0.5f,  -0.5f,  0.5f}, {-0.5f, -0.5f,  -0.5f},
    { 0.5f, 0.5f, 0.5f},  { 0.5f,  0.5f, -0.5f}, { 0.5f,  -0.5f,  -0.5f}, { 0.5f, -0.5f,  0.5f}
};

// positions
GLfloat studentX = 0.0, studentY = 0.0, studentZ = 0.0;
GLfloat camX = 0.0, camY = 0.0, camZ = 0.0;
GLfloat speed = 0.03f;

// rotations
GLint studentRotIndex = 0;
GLint camRotIndex = 1;
GLfloat rotAboutY = 0.0;
GLfloat rotAboutX = 0.0;
const float MAX_LOOK_UP = 45.0f;
const float MAX_LOOK_DOWN = -20.0f;

bool keyW = false, keyS = false;
bool keyTurnLeft = false, keyTurnRight = false;     // A and D 
bool isFullScreen = false;                          // F 
bool isMouseEnabled = true;
int lastMouseX = 400;



// humanoid dimensions
const float HEAD_SIZE = 0.4f;
const float BODY_WIDTH = 0.6f;
const float BODY_HEIGHT = 0.7f;
const float BODY_DEPTH = 0.3f;
const float LIMB_WIDTH = 0.25f;
const float LIMB_HEIGHT = 0.7f;
const float LIMB_DEPTH = 0.25f;


// animation state
float limbSwingAngle = 0.0f;
bool isMoving = false;

const float MAX_SWING_ANGLE = 45.0f; // maximum swing angle for limbs
const float ANIM_SPEED = 3.0f;      // speed of animation

// jump state
float verticalVelocity = 0.0f;
const float GRAVITY = 0.0005f;
const float JUMP_FORCE = 0.03f;
bool isAirborne = false;

// --- COLLISION HELPER FUNCTIONS ---

// Helper to check if two boxes overlap
bool checkCollision(AABB box1, AABB box2) {
    return (box1.minX <= box2.maxX && box1.maxX >= box2.minX) &&
        (box1.minY <= box2.maxY && box1.maxY >= box2.minY) &&
        (box1.minZ <= box2.maxZ && box1.maxZ >= box2.minZ);
}

// Helper to create a static collider at a specific location
void addCollider(float x, float y, float z, float sx, float sy, float sz) {
    AABB box;
    box.minX = x - sx / 2.0f;
    box.maxX = x + sx / 2.0f;
    box.minY = y - sy / 2.0f;
    box.maxY = y + sy / 2.0f;
    box.minZ = z - sz / 2.0f;
    box.maxZ = z + sz / 2.0f;
    staticColliders.push_back(box);
}

void initColliders() {
    float zOffset = -15.0f; // Stands are translated by -15.0 in draw()

    // 1. Stand Base
    for (int i = 0; i < 2; i++) {
        float x = (i == 0) ? -((standLength + standsSeparationDistance) / 2) : ((standLength + standsSeparationDistance) / 2);
        float y = standBaseHeight / 2;
        addCollider(x, y, zOffset, standLength, standBaseHeight, standWidth);
    }

    // 2. Stand Rows (Seating areas)
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < standRowCount; j++) {
            float x = (i == 0) ? -((standLength + standsSeparationDistance) / 2) : ((standLength + standsSeparationDistance) / 2);
            float y = standBaseHeight + standRowHeight / 2 + j;
            float z = zOffset + (standWidth / 5 - (j + 1.0));
            addCollider(x, y, z, standLength, standRowHeight, standRowWidth);
        }
    }

    // 3. Outer Walls
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            float x = (i == 0) ? -(standsSeparationDistance / 2) - standLength - wallLength / 2 : standsSeparationDistance / 2 + standLength + wallLength / 2;
            float y = (standBaseHeight + j * standRowHeight) / 2;
            float z = zOffset + (standWidth / 5 - (j + 1.0));
            addCollider(x, y, z, wallLength, standBaseHeight + j * standRowHeight, outerWallWidth);
        }
    }

    // 4. Walls between stands
    for (int i = 0; i < 2; i++) {
        float x = (i == 0) ? -(standsSeparationDistance / 2) + wallLength / 2 : standsSeparationDistance / 2 - wallLength / 2;
        float y = wallHeight / 2;
        float z = zOffset - standRowWidth;
        addCollider(x, y, z, wallLength, wallHeight, standWidth);
    }

    // 5. Block between stands
    addCollider(0.0, standBaseHeight + blockHeight / 2, zOffset - 2 * standRowWidth, blockLength, blockHeight, 3 * standRowWidth);

    // 6. Lighting Poles
    for (int i = 0; i < poleCount; i++) {
        float x = (i == 0) ? -standLength - (standsSeparationDistance / 2) - 5.0 : standLength + (standsSeparationDistance / 2) + 5.0;
        float z = zOffset + roofWidth / 2;
        addCollider(x, poleHeight / 2, z, poleRadius * 2, poleHeight, poleRadius * 2);
    }

    // 7. Stairs
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < stairsRowCount; j++) {
            float scaleY = (j + 1) * (standBaseHeight / stairsRowCount);
            float scaleX = stairsLength;
            float scaleZ = stairsWidth;

            float x, y, z;


            if (i == 0) // Left Stand Stairs
                x = -(standsSeparationDistance / 2) - (stairsRowCount - j) * stairsLength;
            else // Right Stand Stairs
                x = standsSeparationDistance / 2 + (stairsRowCount - j) * stairsLength;


            y = scaleY / 2;


            z = zOffset + (standWidth / 2 + stairsWidth / 2);

            addCollider(x, y, z, scaleX, scaleY, scaleZ);
        }
    }
}

// Debug function to draw the collision boxes
void drawColliders() {
    if (!showColliders) return;

    glDisable(GL_LIGHTING); // Ensure lines are bright
    glLineWidth(2.0); // Make lines thicker

    // Draw Static Objects (Red)
    glColor3f(1.0, 0.0, 0.0);
    for (size_t i = 0; i < staticColliders.size(); i++) {
        AABB b = staticColliders[i];
        glPushMatrix();
        // Draw wireframe cube
        glBegin(GL_LINES);
        // Bottom
        glVertex3f(b.minX, b.minY, b.minZ); glVertex3f(b.maxX, b.minY, b.minZ);
        glVertex3f(b.maxX, b.minY, b.minZ); glVertex3f(b.maxX, b.minY, b.maxZ);
        glVertex3f(b.maxX, b.minY, b.maxZ); glVertex3f(b.minX, b.minY, b.maxZ);
        glVertex3f(b.minX, b.minY, b.maxZ); glVertex3f(b.minX, b.minY, b.minZ);
        // Top
        glVertex3f(b.minX, b.maxY, b.minZ); glVertex3f(b.maxX, b.maxY, b.minZ);
        glVertex3f(b.maxX, b.maxY, b.minZ); glVertex3f(b.maxX, b.maxY, b.maxZ);
        glVertex3f(b.maxX, b.maxY, b.maxZ); glVertex3f(b.minX, b.maxY, b.maxZ);
        glVertex3f(b.minX, b.maxY, b.maxZ); glVertex3f(b.minX, b.maxY, b.minZ);
        // Verticals
        glVertex3f(b.minX, b.minY, b.minZ); glVertex3f(b.minX, b.maxY, b.minZ);
        glVertex3f(b.maxX, b.minY, b.minZ); glVertex3f(b.maxX, b.maxY, b.minZ);
        glVertex3f(b.maxX, b.minY, b.maxZ); glVertex3f(b.maxX, b.maxY, b.maxZ);
        glVertex3f(b.minX, b.minY, b.maxZ); glVertex3f(b.minX, b.maxY, b.maxZ);
        glEnd();
        glPopMatrix();
    }

    // Draw Player Box (Green)
    glColor3f(0.0, 1.0, 0.0);
    // Calculate player box exactly as used in updateStudent
    // calculate total height
    float totalHeight = LIMB_HEIGHT + BODY_HEIGHT + HEAD_SIZE;

    // size of player box
    float pW = BODY_WIDTH + 0.1f;
    float pH = totalHeight;
    float pD = BODY_DEPTH + 0.1f;

    AABB p;
    p.minX = studentX - pW / 2;
    p.maxX = studentX + pW / 2;

    p.minY = studentY;
    p.maxY = studentY + pH;

    p.minZ = studentZ - pD / 2;
    p.maxZ = studentZ + pD / 2;

    glBegin(GL_LINES);
    // Bottom
    glVertex3f(p.minX, p.minY, p.minZ); glVertex3f(p.maxX, p.minY, p.minZ);
    glVertex3f(p.maxX, p.minY, p.minZ); glVertex3f(p.maxX, p.minY, p.maxZ);
    glVertex3f(p.maxX, p.minY, p.maxZ); glVertex3f(p.minX, p.minY, p.maxZ);
    glVertex3f(p.minX, p.minY, p.maxZ); glVertex3f(p.minX, p.minY, p.minZ);
    // Top
    glVertex3f(p.minX, p.maxY, p.minZ); glVertex3f(p.maxX, p.maxY, p.minZ);
    glVertex3f(p.maxX, p.maxY, p.minZ); glVertex3f(p.maxX, p.maxY, p.maxZ);
    glVertex3f(p.maxX, p.maxY, p.maxZ); glVertex3f(p.minX, p.maxY, p.maxZ);
    glVertex3f(p.minX, p.maxY, p.maxZ); glVertex3f(p.minX, p.maxY, p.minZ);
    // Verticals
    glVertex3f(p.minX, p.minY, p.minZ); glVertex3f(p.minX, p.maxY, p.minZ);
    glVertex3f(p.maxX, p.minY, p.minZ); glVertex3f(p.maxX, p.maxY, p.minZ);
    glVertex3f(p.maxX, p.minY, p.maxZ); glVertex3f(p.maxX, p.maxY, p.maxZ);
    glVertex3f(p.minX, p.minY, p.maxZ); glVertex3f(p.minX, p.maxY, p.maxZ);
    glEnd();

    glLineWidth(1.0);
}

// --- STANDARD FUNCTIONS ---
void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat globalAmbient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    // Light 0
    GLfloat light0_pos[] = { 10.0f, 50.0f, 10.0f, 0.0f };
    GLfloat light0_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light0_diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat light0_specular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);

    // Light 1
    GLfloat light1_pos[] = { 0.0f, 20.0f, -15.0f, 1.0f };
    GLfloat light1_diffuse[] = { 0.4f, 0.4f, 0.5f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);

    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.03f);
}

void genCuboidFaces(int vTopLeft, int vTopRight, int vBottomRight, int vBottomLeft, float nx, float ny, float nz) {
    glBegin(GL_POLYGON);
    glNormal3f(nx, ny, nz);
    glVertex3fv(unitCuboidVertices[vTopLeft]);
    glVertex3fv(unitCuboidVertices[vTopRight]);
    glVertex3fv(unitCuboidVertices[vBottomRight]);
    glVertex3fv(unitCuboidVertices[vBottomLeft]);
    glEnd();
}

void genCuboid(GLfloat length) {
    genCuboidFaces(0, 1, 2, 3, -1.0f, 0.0f, 0.0f); // left face
    genCuboidFaces(4, 5, 6, 7, 1.0f, 0.0f, 0.0f); // right face
    genCuboidFaces(0, 5, 4, 1, 0.0f, 1.0f, 0.0f); // top face
    genCuboidFaces(6, 3, 2, 7, 0.0f, -1.0f, 0.0f); // bottom face
    genCuboidFaces(5, 0, 3, 6, 0.0f, 0.0f, -1.0f); // back face
    genCuboidFaces(1, 4, 7, 2, 0.0f, 0.0f, 1.0f); // front face
}

void drawColorCube(float r, float g, float b) {
    glColor3f(r, g, b);
    genCuboid(1.0f);
}

void genStands() {
    // stand base
    for (int i = 0; i < 2; i++) {
        glPushMatrix();
        if (i == 0) //left stand
            glTranslatef(-((standLength + standsSeparationDistance) / 2), standBaseHeight / 2, 0.0);
        else //right stand
            glTranslatef((standLength + standsSeparationDistance) / 2, standBaseHeight / 2, 0.0);
        glScalef(standLength, standBaseHeight, standWidth);
        drawColorCube(0.9137, 0.8705, 0.7529);
        glPopMatrix();
    }

    // stand rows
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < standRowCount; j++) {
            glPushMatrix();
            if (i == 0) // left stand rows
                glTranslatef(-((standLength + standsSeparationDistance) / 2), standBaseHeight + standRowHeight / 2 + j, standWidth / 5 - (j + 1.0));
            else // right stand rows
                glTranslatef((standLength + standsSeparationDistance) / 2, standBaseHeight + standRowHeight / 2 + j, standWidth / 5 - (j + 1.0));
            glScalef(standLength, standRowHeight, standRowWidth);
            if (j % 2 == 0) // lighter yellow
                drawColorCube(0.9537, 0.9105, 0.7729);
            else //darker yellow
                drawColorCube(0.9137, 0.8705, 0.7529);
            glPopMatrix();
        }
    }

    // outer walls
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            glPushMatrix();
            if (i == 0) // left outer wall
                glTranslatef(-(standsSeparationDistance / 2) - standLength - wallLength / 2 + 0.01, (standBaseHeight + j * standRowHeight) / 2, standWidth / 5 - (j + 1.0));
            else // right outer wall
                glTranslatef(standsSeparationDistance / 2 + standLength + wallLength / 2 - 0.01, (standBaseHeight + j * standRowHeight) / 2, standWidth / 5 - (j + 1.0));
            glScalef(wallLength, standBaseHeight + j * standRowHeight, outerWallWidth);
            drawColorCube(0.8667, 0.8157, 0.6431);
            glPopMatrix();
        }
    }

    // walls btw stands
    for (int i = 0; i < 2; i++) {
        glPushMatrix();
        if (i == 0) // left wall
            glTranslatef(-(standsSeparationDistance / 2) + wallLength / 2, wallHeight / 2, -standRowWidth);
        else // right wall
            glTranslatef(standsSeparationDistance / 2 - wallLength / 2, wallHeight / 2, -standRowWidth);
        glScalef(wallLength, wallHeight, standWidth);
        drawColorCube(0.8667, 0.8157, 0.6431);
        glPopMatrix();
    }

    // platform btw stands
    glPushMatrix();
    glTranslatef(0.0, (standBaseHeight - standRowHeight / 2) + 0.01, 0.0);
    glScalef(platformLength, standRowHeight, platformWidth);
    drawColorCube(0.9537, 0.9105, 0.7729);
    glPopMatrix();

    // block btw stands
    glPushMatrix();
    glTranslatef(0.0, standBaseHeight + blockHeight / 2, -2 * standRowWidth);
    glScalef(blockLength, blockHeight, 3 * standRowWidth);
    drawColorCube(0.4078, 0.1490, 0.08627);
    glPopMatrix();

    // stairs
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < stairsRowCount; j++) {
            glPushMatrix();
            if (i == 0) // left stand stairs
                glTranslatef(-(standsSeparationDistance / 2) - (stairsRowCount - j) * stairsLength, ((j + 1) * (standBaseHeight / stairsRowCount)) / 2, standWidth / 2 + stairsWidth / 2);
            else // right stand stairs
                glTranslatef(standsSeparationDistance / 2 + (stairsRowCount - j) * stairsLength, ((j + 1) * (standBaseHeight / stairsRowCount)) / 2, standWidth / 2 + stairsWidth / 2);
            glScalef(stairsLength, (j + 1) * (standBaseHeight / stairsRowCount), stairsWidth);
            drawColorCube(0.9537, 0.9105, 0.7729);
            glPopMatrix();
        }
    }

    // seats
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < standRowCount; j++) {
            for (int k = 0; k < seatInARowCount; k++) {
                glPushMatrix();
                if (i == 0) // left stand seats
                    glTranslatef((-standsSeparationDistance / 2 - 1 - (seatInARowCount - k - 1) * seatLength - ((seatInARowCount - k) * seatGap)), standBaseHeight + standRowHeight + seatHeight / 2 + j, standWidth / 5 - (j + 1.0));
                else // right stand seats
                    glTranslatef((standsSeparationDistance / 2 + 1 + (seatInARowCount - k - 1) * seatLength + ((seatInARowCount - k) * seatGap)), standBaseHeight + standRowHeight + seatHeight / 2 + j, standWidth / 5 - (j + 1.0));
                glScalef(seatLength, seatHeight, seatWidth);
                if (j % 2 == 0) // blue
                    drawColorCube(0.3490, 0.7529, 0.9686);
                else // yellow
                    drawColorCube(0.9922, 0.8471, 0.0314);
                glPopMatrix();
            }
        }
    }

    // roof
    glPushMatrix();
    glTranslatef(0.0, wallHeight - 5.0, 0.0);
    glRotatef(roofAngle, 1.0, 0.0, 0.0);
    glScalef(roofLength, roofHeight, roofWidth);
    drawColorCube(0.3490, 0.2784, 0.2);
    glPopMatrix();
}

void genLightingPoles() {
    GLUquadric* quad = gluNewQuadric();

    for (int i = 0; i < poleCount; i++) {
        // poles
        glPushMatrix();
        if (i == 0)
            glTranslatef(-standLength - (standsSeparationDistance / 2) - 5.0, 0.0, roofWidth / 2);
        else
            glTranslatef(standLength + (standsSeparationDistance / 2) + 5.0, 0.0, roofWidth / 2);
        glRotatef(-90, 1, 0, 0);
        glColor3f(0.2, 0.2, 0.2);
        gluCylinder(quad, poleRadius, poleRadius, poleHeight, poleSlices, poleVSubDiv);
        glPopMatrix();

        // frames & lights
        glPushMatrix();
        if (i == 0) {
            glTranslatef(-standLength - (standsSeparationDistance / 2) - 5.0 + poleRadius, poleHeight, roofWidth / 2 + poleRadius + frameWidth);
            glRotatef(frameAngleAroundY, 0.0, 1.0, 0);
        }
        else {
            glTranslatef(standLength + (standsSeparationDistance / 2) + 5.0 - poleRadius, poleHeight, roofWidth / 2 + poleRadius + frameWidth);
            glRotatef(-frameAngleAroundY, 0.0, 1.0, 0);
        }
        glRotatef(frameAngleAroundX, 1.0, 0.0, 0);
        // frames
        glPushMatrix();
        glScalef(frameLength, frameHeight, frameWidth);
        drawColorCube(0.15, 0.15, 0.15);
        glPopMatrix();
        // lights
        for (int j = 0; j < lightRowCount; j++) {
            glPushMatrix();
            if (j == 0) //top row
                glTranslatef(0.0, (lightHeight + lightGap) / 2, frameWidth);
            else // bottom row
                glTranslatef(0.0, -(lightHeight + lightGap) / 2, frameWidth);
            glScalef(lightLength, lightHeight, lightWidth);
            drawColorCube(1.0, 1.0, 1.0);
            glPopMatrix();
        }
        glPopMatrix();
    }
    gluDeleteQuadric(quad);
}

void genTrack() {
    GLUquadric* quad = gluNewQuadric();

    // center pitch
    glPushMatrix();
    glTranslatef(0.0, 0.012, 0.0);
    glScalef(centerPitchRadiusX, 1.0f, centerPitchRadiusZ);
    glRotatef(90, 1, 0, 0);
    glColor3f(0.2627, 0.7059, 0.2745);
    gluDisk(quad, 0.0, 1.0, 40, 1);
    glPopMatrix();

    // tracks
    for (int i = 0; i < laneCount + laneLineCount + 1; i++) {
        glPushMatrix();
        glTranslatef(0.0, i * 0.001, 0.0);
        if (i % 2 == 0) // lane lines
            glScalef(outermostLaneLineRadiusX - ((i / 2 + 0.5) * laneLineWidth) - ((i / 2 - 0.5) * laneWidth), 1.0f, outermostLaneLineRadiusZ - ((i / 2 + 0.5) * laneLineWidth) - ((i / 2 - 0.5) * laneWidth));
        else // lanes
            glScalef(outermostLaneLineRadiusX - ((i / 2) * laneLineWidth) - ((i / 2) * laneWidth), 1.0f, outermostLaneLineRadiusZ - ((i / 2) * laneLineWidth) - ((i / 2) * laneWidth));
        glRotatef(90, 1, 0, 0);
        if (i % 2 == 0) // lane lines
            glColor3f(0.7098, 0.6706, 0.5686);
        else // lanes
            glColor3f(0.4118, 0.2078, 0.1686);
        gluDisk(quad, 0.0, 1.0, 40, 1);
        glPopMatrix();
    }

    gluDeleteQuadric(quad);
}


// draw humanoid at origin
void drawHumanoid() {
    // calculate swing angle based on limbSwingAngle
    float swing = sin(limbSwingAngle * 3.14159 / 180.0f) * MAX_SWING_ANGLE;

    glPushMatrix();
    glTranslatef(0.0f, LIMB_HEIGHT + BODY_HEIGHT / 2.0f - 0.5f, 0.0f);

    // body
    glPushMatrix();
    glScalef(BODY_WIDTH, BODY_HEIGHT, BODY_DEPTH);
    drawColorCube(0.2f, 0.2f, 0.8f); // blue shirt
    glPopMatrix();

    // head
    glPushMatrix();
    glTranslatef(0.0f, BODY_HEIGHT / 2.0f + HEAD_SIZE / 2.0f, 0.0f);
    glScalef(HEAD_SIZE, HEAD_SIZE, HEAD_SIZE);
    drawColorCube(1.0f, 0.49f, 0.39f); // skin color
    glPopMatrix();

    // left arm
    glPushMatrix();
    glTranslatef(-(BODY_WIDTH / 2.0f + LIMB_WIDTH / 2.0f), BODY_HEIGHT / 1.5f - LIMB_WIDTH / 2.0f, 0.0f);
    glRotatef(swing, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -LIMB_HEIGHT / 2.0f, 0.0f);
    glScalef(LIMB_WIDTH, LIMB_HEIGHT, LIMB_DEPTH);
    drawColorCube(1.0f, 0.49f, 0.39f); // skin color
    glPopMatrix();

    // right arm
    glPushMatrix();
    glTranslatef((BODY_WIDTH / 2.0f + LIMB_WIDTH / 2.0f), BODY_HEIGHT / 1.5f - LIMB_WIDTH / 2.0f, 0.0f);
    glRotatef(-swing, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -LIMB_HEIGHT / 2.0f, 0.0f);
    glScalef(LIMB_WIDTH, LIMB_HEIGHT, LIMB_DEPTH);
    drawColorCube(1.0f, 0.49f, 0.39f); // skin color
    glPopMatrix();

    // left leg
    glPushMatrix();
    glTranslatef(-(BODY_WIDTH / 4.0f), -BODY_HEIGHT / 2.0f, 0.0f);
    glRotatef(-swing, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -LIMB_HEIGHT / 2.0f, 0.0f);
    glScalef(LIMB_WIDTH, LIMB_HEIGHT, LIMB_DEPTH);
    drawColorCube(0.3f, 0.3f, 0.3f); // dark gray pants
    glPopMatrix();

    // right leg
    glPushMatrix();
    glTranslatef((BODY_WIDTH / 4.0f), -BODY_HEIGHT / 2.0f, 0.0f);
    glRotatef(swing, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -LIMB_HEIGHT / 2.0f, 0.0f);
    glScalef(LIMB_WIDTH, LIMB_HEIGHT, LIMB_DEPTH);
    drawColorCube(0.3f, 0.3f, 0.3f);
    glPopMatrix();

    glPopMatrix();
}

void genGround(GLfloat centerX, GLfloat centerY, GLfloat centerZ) {
    GLfloat gLeft = centerX - groundLength / 2.0;
    GLfloat gRight = centerX + groundLength / 2.0;
    GLfloat gFront = centerZ + groundWidth / 2.0;
    GLfloat gBack = centerZ - groundWidth / 2.0;

    //ground plane
    glPushMatrix();
    glBegin(GL_QUADS);
    glColor3f(0.2627, 0.7059, 0.2745);
    glVertex3f(gLeft, centerY, gBack);
    glVertex3f(gRight, centerY, gBack);
    glVertex3f(gRight, centerY, gFront);
    glVertex3f(gLeft, centerY, gFront);
    glEnd();
    glPopMatrix();
}

// render bitmap string at top left corner
void renderBitmapString(float x, float y, void* font, const char* string) {
    const char* c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

// draw keyboard controls UI
void drawHelpUI() {
    // get window dimensions
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glColor3f(0.0f, 0.0f, 0.0f);

    int x = 20;
    int yStart = h - 30;
    int lineHeight = 20;

    void* font = GLUT_BITMAP_HELVETICA_18;

    renderBitmapString(x, yStart, font, "--- CONTROLS ---");
    renderBitmapString(x, yStart - lineHeight * 1, GLUT_BITMAP_HELVETICA_12, "W / S : Move Forward / Backward");
    renderBitmapString(x, yStart - lineHeight * 2, GLUT_BITMAP_HELVETICA_12, "A / D : Turn Camera Left / Right (Keyboard)");
    renderBitmapString(x, yStart - lineHeight * 3, GLUT_BITMAP_HELVETICA_12, "Space : Jump");
    renderBitmapString(x, yStart - lineHeight * 4, GLUT_BITMAP_HELVETICA_12, "M     : Toggle Mouse Control");
    renderBitmapString(x, yStart - lineHeight * 5, GLUT_BITMAP_HELVETICA_12, "F     : Fullscreen");
    renderBitmapString(x, yStart - lineHeight * 6, GLUT_BITMAP_HELVETICA_12, "B     : Toggle Debug Box");
    renderBitmapString(x, yStart - lineHeight * 7, GLUT_BITMAP_HELVETICA_12, "Mouse : Look Around");
    renderBitmapString(x, yStart - lineHeight * 8, GLUT_BITMAP_HELVETICA_12, "ESC   : Exit Game");

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void draw() {
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    setupLighting();

    // Draw Environment
    glPushMatrix();
    glTranslatef(0.0, 0.0, -15.0);
    genStands();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0, 0.0, -15.0);
    genLightingPoles();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0, 0.01, 10.0);
    genTrack();
    glPopMatrix();

    glPushMatrix();
    genGround(0.0, 0.0, 0.0);
    glPopMatrix();


    // Draw Student
    glPushMatrix();
    glTranslatef(studentX, studentY + 0.5, studentZ);
    glRotatef(rotAboutY, 0.0, 1.0, 0.0);
    drawHumanoid();
    glPopMatrix();

    // Draw Debug Colliders
    drawColliders();

    drawHelpUI();

    glutSwapBuffers();
}

// mouse movement handler
void mouseMove(int x, int y) {
    if (!isMouseEnabled) return;

    float sensitivity = 0.05f;

    int centerX = glutGet(GLUT_WINDOW_WIDTH) / 2;
    int centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2;

    int dx = x - centerX;
    int dy = y - centerY;

    // Horizontal rotation
    rotAboutY -= dx * sensitivity;

    // Vertical rotation
    rotAboutX -= dy * sensitivity;

    // Clamp vertical look
    if (rotAboutX > MAX_LOOK_UP) rotAboutX = MAX_LOOK_UP;
    if (rotAboutX < MAX_LOOK_DOWN) rotAboutX = MAX_LOOK_DOWN;

    if (rotAboutY >= 360.0) rotAboutY -= 360.0;
    if (rotAboutY < 0.0) rotAboutY += 360.0;

    // Reset mouse to center of window
    if (dx != 0 || dy != 0) {
        glutWarpPointer(centerX, centerY);
    }
}

void updateStudent() {
    float rotAngle = rotAboutY * 3.1415926 / 180.0;

    // calculate forward direction vector
    float forwardX = -sin(rotAngle);
    float forwardZ = -cos(rotAngle);

    // calculate right direction vector
    float rightX = -cos(rotAngle);
    float rightZ = sin(rotAngle);

    // Calculate NEXT position based on input
    float nextX = studentX;
    float nextZ = studentZ;
    float nextY = studentY;

    // Apply Gravity
    verticalVelocity -= GRAVITY;
    nextY += verticalVelocity;

    // Ground Check
    if (nextY <= 0.0f) {
        nextY = 0.0f;
        verticalVelocity = 0.0f;
        isAirborne = false;
    }
    else {
        isAirborne = true;
    }

    isMoving = (keyW || keyS);

    // move forward/backward (W/S)
    if (keyW) {
        nextX += forwardX * speed;
        nextZ += forwardZ * speed;
    }
    if (keyS) {
        nextX -= forwardX * speed;
        nextZ -= forwardZ * speed;
    }

    // rotate left/right (A/D)
    if (keyTurnLeft) rotAboutY += 0.05f;
    if (keyTurnRight) rotAboutY -= 0.05f;

    if (rotAboutY >= 360.0) rotAboutY -= 360.0;
    if (rotAboutY < 0.0) rotAboutY += 360.0;

    // Constrain to World Boundaries first
    if (nextX < worldMinX) nextX = worldMinX;
    if (nextX > worldMaxX) nextX = worldMaxX;
    if (nextZ < worldMinZ) nextZ = worldMinZ;
    if (nextZ > worldMaxZ) nextZ = worldMaxZ;
    if (nextY < worldMinY) nextY = worldMinY;

    float totalHeight = LIMB_HEIGHT + BODY_HEIGHT + HEAD_SIZE;

    // --- COLLISION CHECK ---
    // Define the player's AABB at the NEXT position
    float pW = BODY_WIDTH + 0.1f;
    float pH = totalHeight;
    float pD = BODY_DEPTH + 0.1f;

    AABB playerBox;
    playerBox.minX = nextX - pW / 2;
    playerBox.maxX = nextX + pW / 2;
    playerBox.minY = nextY;
    playerBox.maxY = nextY + pH;
    playerBox.minZ = nextZ - pD / 2;
    playerBox.maxZ = nextZ + pD / 2;

    bool collisionDetected = false;
    for (size_t i = 0; i < staticColliders.size(); i++) {
        if (checkCollision(playerBox, staticColliders[i])) {
            collisionDetected = true;
            break;
        }
    }

    if (!collisionDetected) {
        studentX = nextX;
        studentZ = nextZ;
        studentY = nextY;

        if (isMoving && !isAirborne) {
            limbSwingAngle += ANIM_SPEED; // aniamtion speed
            if (limbSwingAngle > 360.0f) limbSwingAngle -= 360.0f;
        }
    }
    else {
        if (verticalVelocity > 0) verticalVelocity = 0;
    }

    // Smoothly return limbs to neutral position when not moving
    if (!isMoving) {
        float target = (sin(limbSwingAngle * 3.14159f / 180.0f) > 0) ? 180.0f : 0.0f;
        limbSwingAngle = limbSwingAngle * 0.8f + target * 0.2f;
    }
}

void updateCam() {
    // Convert angles to radians
    float radY = rotAboutY * 3.1415926 / 180.0;
    float radX = rotAboutX * 3.1415926 / 180.0;
    float dist = 6.0; // distance from student

    // Calculate camera position in 3D space
    // X and Z are affected by both horizontal and vertical rotation
    camX = studentX + dist * cos(radX) * sin(radY);
    camZ = studentZ + dist * cos(radX) * cos(radY);
    camY = studentY + 2.0 + dist * sin(radX); // Height changes as we look up/down

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // gluLookAt: Eye point (cam), Center point (student), Up vector
    gluLookAt(camX, camY, camZ, studentX, studentY + 1.0, studentZ, 0.0, 1.0, 0.0);
}

void update() {
    updateStudent();
    updateCam();
    glutPostRedisplay();
}

void reshape(int w, int h) {
    (h == 0) ? h : 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// keyboard input handler
void keyDown(unsigned char key, int x, int y) {
    switch (key) {
    case 'w': case 'W': // move forward
        keyW = true;
        break;
    case 's': case 'S': // move backward
        keyS = true;
        break;
    case 'a': case 'A': // turn left
        keyTurnLeft = true;
        break;
    case 'd': case 'D': // turn right
        keyTurnRight = true;
        break;
    case 'b': case 'B': // toggle debug colliders
        showColliders = !showColliders;
        break;
    case 'f': case 'F': // toggle fullscreen
        isFullScreen = !isFullScreen;
        if (isFullScreen) glutFullScreen();
        else glutReshapeWindow(800, 600);
        break;
    case 'm': case 'M': // toggle mouse control
        isMouseEnabled = !isMouseEnabled;
        if (isMouseEnabled) {
            // Enable mouse control
            glutSetCursor(GLUT_CURSOR_NONE);
            glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
        }
        else {
            // Disable mouse control
            glutSetCursor(GLUT_CURSOR_INHERIT);
        }
        break;
    case ' ': // Spacebar, jump
        if (!isAirborne) {
            verticalVelocity = JUMP_FORCE;
            isAirborne = true;
        }
        break;
    case 27: // ESC key
        exit(0);
        break;
    }
}

void keyUp(unsigned char key, int x, int y) {
    switch (key) {
    case 'w': case 'W': keyW = false; break;
    case 's': case 'S': keyS = false; break;
    case 'a': case 'A': keyTurnLeft = false; break;
    case 'd': case 'D': keyTurnRight = false; break;
    }
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("UTeM Stadium");

    initColliders();

    glutDisplayFunc(draw);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);

    glutPassiveMotionFunc(mouseMove);

    glutSetCursor(GLUT_CURSOR_NONE);

    glutIdleFunc(update);
    glutReshapeFunc(reshape);
    glEnable(GL_DEPTH_TEST);
    glutMainLoop();
    return 0;
}