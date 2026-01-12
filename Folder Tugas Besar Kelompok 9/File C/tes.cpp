#include <GL/glew.h>
#include <GL/glut.h>
#include <FreeImage.h>
#include <stdio.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;


// Texture IDs
GLuint texture_bulan_ID;
GLuint texture_bumi_ID;
GLuint texture_matahari_ID;

// Animation & Rotation Variables
float rotasiBulan     = 0.0f;
float rotasiBumi      = 0.0f;
float rotasiMatahari  = 0.0f;
float orbitBulan      = 0.0f;
float orbitBumi       = 0.0f;
float orbitUFO        = 0.0f;
float rotasiUFO       = 0.0f;
float nozzle          = 1.0f;

// Scaling Variables
float scalePohon      = 1.5f;
float scaleRocket     = 0.8f;

// Lighting State
bool isSunLightOn = true;

// Object Distances & Positions
float jarakBumi       = 0.0f;
float jarakMatahari   = -60.0f;
float jarakOrbitUFO   = 5.0f;

// Lighting Configuration
GLfloat light_position[] = { 80.0, 0.0, 0.0, 1.0 };
GLfloat light_ambient[]  = { 0.15, 0.15, 0.15, 1.0 };
GLfloat light_diffuse[]  = { 1.0, 1.0, 0.9, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };

// Camera Variables (Free Cam)
float freeCamPosX   = 0.0f;
float freeCamPosY   = 5.0f;
float freeCamPosZ   = 15.0f;
float freeCamYaw    = 0.0f;
float freeCamPitch  = 0.0f;
float freeCamSpeed  = 0.5f;

// Input States
bool keyW = false, keyA = false, keyS = false, keyD = false;
bool keyQ = false, keyE = false;
bool mouseLeftDown  = false;
int mouseLastX      = 0;
int mouseLastY      = 0;

// Star System
struct Star {
    float x, y, z;
    float r, g, b;
    float brightness;
    float pulseSpeed;
    float pulsePhase;
    float size;
};
const int NUM_STARS = 2000;
Star stars[NUM_STARS];

// ================= INITIALIZATION & HELPER FUNCTIONS =================

void initStars() {
    srand(time(NULL));
    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].x = (rand() % 400 - 200) * 1.0f;
        stars[i].y = (rand() % 400 - 200) * 1.0f;
        stars[i].z = (rand() % 400 - 200) * 1.0f;
        stars[i].brightness = 0.5 + (rand() % 50) / 100.0f;
        stars[i].pulseSpeed = 0.5 + (rand() % 100) / 100.0f;
        stars[i].pulsePhase = rand() % 100 / 100.0f * 2 * M_PI;
        stars[i].size = 1.0 + (rand() % 10) / 10.0f;
        stars[i].r = 0.8f; stars[i].g = 0.8f; stars[i].b = 1.0f;
    }
}

void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(path);
    if (format == FIF_UNKNOWN) return 0;
    
    FIBITMAP* bitmap = FreeImage_Load(format, path, 0);
    if (!bitmap) return 0;
    
    FIBITMAP* bitmap2 = FreeImage_ConvertTo24Bits(bitmap);
    FreeImage_Unload(bitmap);
    
    int w = FreeImage_GetWidth(bitmap2);
    int h = FreeImage_GetHeight(bitmap2);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    int formatColor = (FI_RGBA_RED == 0) ? GL_RGB : GL_BGR;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, formatColor, GL_UNSIGNED_BYTE, FreeImage_GetBits(bitmap2));
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    FreeImage_Unload(bitmap2);
    return textureID;
}

void createObject(float radius) {
    GLUquadric* object = gluNewQuadric();
    gluQuadricTexture(object, GL_TRUE);
    gluQuadricNormals(object, GLU_SMOOTH);
    gluSphere(object, radius, 100, 100);
    gluDeleteQuadric(object);
}

// ================= DRAWING FUNCTIONS =================

void drawStars() {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glPointSize(2.0f);

    glBegin(GL_POINTS);
    static float time = 0.0f;
    time += 0.01f;
    for (int i = 0; i < NUM_STARS; i++) {
        float pulse = 0.7f + 0.3f * sin(time * stars[i].pulseSpeed + stars[i].pulsePhase);
        glColor4f(stars[i].r, stars[i].g, stars[i].b, stars[i].brightness * pulse);
        glVertex3f(stars[i].x, stars[i].y, stars[i].z);
    }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

void drawRocket() {
    glPushMatrix();
    glDisable(GL_TEXTURE_2D);
    glScalef(scaleRocket, scaleRocket, scaleRocket);

    GLUquadric* quad = gluNewQuadric();

    // 1. Nozzle
    glColor3f(0.3f, 0.3f, 0.3f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluCylinder(quad, 0.5, 0.4, 0.6, 30, 10);
    gluDisk(quad, 0.0, 0.5, 30, 1);
    glPopMatrix();

    // 2. Body
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.6f, 0.0f);
    glRotatef(-90, 1, 0, 0);
    gluCylinder(quad, 0.6, 0.5, 2.2, 30, 30);
    gluDisk(quad, 0.0, 0.6, 30, 1);
    glPopMatrix();

    // 3. Fins
    glColor3f(0.9f, 0.1f, 0.1f);
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glRotatef(90 * i, 0, 1, 0);
        glTranslatef(0.6f, 0.8f, 0.0f);
        glBegin(GL_QUADS);
        glVertex3f(0.0f, 0.4f, 0.0f);
        glVertex3f(-0.1f, -0.4f, 0.0f);
        glVertex3f(0.7f, -1.4f, 0.0f);
        glVertex3f(0.7f, -0.1f, 0.0f);
        glVertex3f(0.7f, -0.1f, 0.0f);
        glVertex3f(0.7f, -1.4f, 0.0f);
        glVertex3f(-0.1f, -0.4f, 0.0f);
        glVertex3f(0.0f, 0.4f, 0.0f);
        glEnd();
        glPopMatrix();
    }

    // 4. Nose
    glColor3f(0.9f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, 2.8f, 0.0f);
    glRotatef(-90, 1, 0, 0);
    glutSolidCone(0.52, 1.2, 30, 30);
    glPopMatrix();

    // 5. Thruster
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0f, -0.2f, 0.0f);
    glRotatef(90, 1, 0, 0);
    float scaleApi = 1.0f + 0.2f * sin(nozzle);
    glColor3f(1.0f, 0.5f, 0.0f);
    glutSolidCone(0.3 * scaleApi, 1.0 * scaleApi, 20, 20);
    glColor3f(1.0f, 1.0f, 0.0f);
    glutSolidCone(0.15 * scaleApi, 0.6 * scaleApi, 20, 20);
    glPopMatrix();
    glEnable(GL_LIGHTING);

    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
    gluDeleteQuadric(quad);
    glPopMatrix();
}

void drawTree() {
    glPushMatrix();
    glDisable(GL_TEXTURE_2D);
    glScalef(scalePohon, scalePohon, scalePohon);

    // batang
    glColor3f(0.55f, 0.27f, 0.07f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, 0.4, 0.4, 1.0, 20, 5);
    gluDisk(quad, 0.0, 0.4, 20, 1);
    gluDeleteQuadric(quad);
    glPopMatrix();

    // Daun
    glColor3f(0.42f, 0.69f, 0.18f);
    float coneHeights[] = { 1.5f, 1.4f, 1.2f };
    float coneBase[] = { 1.8f, 1.4f, 0.9f };
    float yPos[] = { 0.8f, 1.8f, 2.7f };

    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glTranslatef(0.0f, yPos[i], 0.0f);
        glRotatef(-90, 1, 0, 0);
        glutSolidCone(coneBase[i], coneHeights[i], 20, 10);
        glPopMatrix();
    }

    // Dekorasi
    glDisable(GL_LIGHTING);
    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.0f, 0.0f);
    float spiralHeight = 3.5f;
    float spiralTurns = 4.0f;
    for (float t = 0; t <= spiralTurns * 2 * M_PI; t += 0.1f) {
        float h = (t / (spiralTurns * 2 * M_PI)) * spiralHeight;
        float currentRadius = 1.8f * (1.0f - (h / 3.8f));
        glVertex3f(currentRadius * cos(t), h + 1.0f, currentRadius * sin(t));
    }
    glEnd();
    glLineWidth(1.0f);

    // Ornamen
    struct Ball { float x, y, z, r, g, b; };
    Ball ornaments[] = {
        {0.9f, 1.2f, 0.8f, 1.0f, 0.0f, 0.0f},
        {-1.0f, 1.4f, 0.5f, 0.0f, 0.5f, 1.0f},
        {0.0f, 1.3f, -1.2f, 1.0f, 1.0f, 0.0f},
        {0.6f, 2.2f, 0.0f, 0.0f, 0.5f, 1.0f},
        {-0.5f, 2.3f, 0.6f, 1.0f, 0.0f, 0.0f},
        {0.0f, 3.0f, 0.5f, 1.0f, 1.0f, 0.0f},
        {0.3f, 2.9f, -0.3f, 1.0f, 0.0f, 0.0f}
    };

    for (int i = 0; i < 7; i++) {
        glPushMatrix();
        glColor3f(ornaments[i].r, ornaments[i].g, ornaments[i].b);
        glTranslatef(ornaments[i].x, ornaments[i].y, ornaments[i].z);
        glutSolidSphere(0.15, 10, 10);
        glPopMatrix();
    }

    // Hiasan Bintang
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0f, 3.9f, 0.0f);
    glScalef(0.4f, 0.4f, 0.1f);
    glColor3f(1.0f, 0.85f, 0.0f);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 5; i++) {
        float angle = i * 2.0f * M_PI / 5.0f;
        float nextAngle = (i + 1) * 2.0f * M_PI / 5.0f;
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(sin(angle), cos(angle), 0.0f);
        glVertex3f(0.4f * sin(angle + 0.628f), 0.4f * cos(angle + 0.628f), 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.4f * sin(angle + 0.628f), 0.4f * cos(angle + 0.628f), 0.0f);
        glVertex3f(sin(nextAngle), cos(nextAngle), 0.0f);
    }
    glEnd();
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawUFO() {
    glPushMatrix();

    // Hierarchical Transformation: Sun -> Earth -> Moon -> UFO
    glTranslatef(jarakMatahari, 0.0, -20.0);
    glRotatef(orbitBumi, 0.0, 1.0, 0.0);
    glTranslatef(60.0, 0.0, 0.0);
    glRotatef(orbitBulan, 0.0, 1.0, 0.0);
    glTranslatef(13.0, 0.0, 0.0);

    float orbitRadius = jarakOrbitUFO;
    float rad = orbitUFO * M_PI / 180.0f;
    glTranslatef(orbitRadius * cos(rad), 0.8f, orbitRadius * sin(rad));
    glRotatef(rotasiUFO, 0.0f, 1.0f, 0.0f);

    glDisable(GL_TEXTURE_2D);
    GLfloat no_emission[] = { 0, 0, 0, 1 };
    glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);

    // Body
    GLfloat ufo_ambient[] = { 0.3f, 0.0f, 0.3f, 1.0f };
    GLfloat ufo_diffuse[] = { 0.6f, 0.1f, 0.8f, 1.0f };
    GLfloat ufo_specular[] = { 0.9f, 0.9f, 1.0f, 1.0f };
    GLfloat ufo_shine[] = { 80.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, ufo_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, ufo_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, ufo_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, ufo_shine);

    glPushMatrix();
    glScalef(1.6f, 0.35f, 1.6f);
    glutSolidSphere(1.0, 50, 50);
    glPopMatrix();

    // Lights
    GLfloat light_emission[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_off[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, light_emission);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, light_off);

    int numLights = 16;
    float ringRadius = 1.55f;
    for (int i = 0; i < numLights; i++) {
        float angle = (float)i / numLights * 2.0f * M_PI;
        glPushMatrix();
        glTranslatef(cos(angle) * ringRadius, 0.0f, sin(angle) * ringRadius);
        glutSolidSphere(0.12, 20, 20);
        glPopMatrix();
    }
    glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);

    // Dome


    // Antenna & Beam
    glMaterialfv(GL_FRONT, GL_AMBIENT, ufo_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, ufo_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, ufo_shine);

    glPushMatrix();
    glTranslatef(0.0f, 0.95f, 0.0f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    GLUquadric* antennaObj = gluNewQuadric();
    gluCylinder(antennaObj, 0.02, 0.02, 0.4, 16, 1);
    gluDeleteQuadric(antennaObj);
    glPopMatrix();
    glTranslatef(0.0f, 0.4f, 0.0f);
    GLfloat tip_color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, tip_color);
    glutSolidSphere(0.05, 16, 16);
    glPopMatrix();

    glMaterialfv(GL_FRONT, GL_AMBIENT, ufo_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, ufo_diffuse);
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    glutSolidTorus(0.08, 1.2, 20, 40);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.4f, 0.8f, 1.0f, 0.5f); glVertex3f(0.0f, -0.2f, 0.0f);
    glColor4f(0.4f, 0.8f, 1.0f, 0.0f);
    for (int i = 0; i <= 360; i += 20) {
        float a = i * M_PI / 180.0f;
        glVertex3f(cos(a) * 2.0f, -6.0f, sin(a) * 2.0f);
    }
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    glPopMatrix();
}

// ================= PLANETARY DRAWING FUNCTIONS =================

void drawMatahari() {
    glPushMatrix();
    glTranslatef(jarakMatahari, 0.0, -20.0);
    glRotatef(rotasiMatahari, 0.0, 1.0, 0.0);

    if (isSunLightOn) {
        light_position[0] = jarakMatahari;
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        GLfloat sun_emission[] = { 1.0, 1.0, 0.8, 1.0 };
        glMaterialfv(GL_FRONT, GL_EMISSION, sun_emission);
    } else {
        GLfloat no_emission[] = { 0.0, 0.0, 0.0, 1.0 };
        glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);
    }

    glBindTexture(GL_TEXTURE_2D, texture_matahari_ID);
    createObject(25.0);

    GLfloat no_emission[] = { 0.0, 0.0, 0.0, 1.0 };
    glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);
    glPopMatrix();
}

void drawBumi() {
    glPushMatrix();
    // Hierarchical: Sun -> Earth
    glTranslatef(jarakMatahari, 0.0, -20.0);
    glRotatef(orbitBumi, 0.0, 1.0, 0.0);
    glTranslatef(60.0, 0.0, 0.0);

    glRotatef(rotasiBumi, 0.0, 1.0, 0.0);

    glColor3f(1.0f, 1.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, texture_bumi_ID);
    createObject(6.0);

    // Tree on Earth
    glPushMatrix();
    glTranslatef(0.0f, 6.0f, 0.0f);
    drawTree();
    glPopMatrix();

    glPopMatrix();
}

void drawBulan() {
    glPushMatrix();
    // Hierarchical: Sun -> Earth -> Moon
    glTranslatef(jarakMatahari, 0.0, -20.0);
    glRotatef(orbitBumi, 0.0, 1.0, 0.0);
    glTranslatef(60.0, 0.0, 0.0);

    glRotatef(orbitBulan, 0.0, 1.0, 0.0);
    glTranslatef(13.0, 0.0, 0.0);

    glRotatef(rotasiBulan, 0.0, 1.0, 0.0);

    glColor3f(1.0f, 1.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, texture_bulan_ID);
    createObject(3.0);

    // Rocket on Moon
    glPushMatrix();
    glTranslatef(0.0f, 3.8f, 0.0f);
    drawRocket();
    glPopMatrix();

    glPopMatrix();
}

// ==========================================
//           INPUT & UPDATE LOGIC
// ==========================================

void updateCamera() {
    float yawRad = freeCamYaw * M_PI / 180.0f;
    float pitchRad = freeCamPitch * M_PI / 180.0f;

    float fx = sin(yawRad) * cos(pitchRad);
    float fy = sin(pitchRad);
    float fz = cos(yawRad) * cos(pitchRad);

    float rx = cos(yawRad);
    float rz = -sin(yawRad);

    if (keyW) { freeCamPosX += fx * freeCamSpeed; freeCamPosY += fy * freeCamSpeed; freeCamPosZ += fz * freeCamSpeed; }
    if (keyS) { freeCamPosX -= fx * freeCamSpeed; freeCamPosY -= fy * freeCamSpeed; freeCamPosZ -= fz * freeCamSpeed; }
    if (keyA) { freeCamPosX += rx * freeCamSpeed; freeCamPosZ += rz * freeCamSpeed; }
    if (keyD) { freeCamPosX -= rx * freeCamSpeed; freeCamPosZ -= rz * freeCamSpeed; }
    if (keyQ) { freeCamPosY += freeCamSpeed; }
    if (keyE) { freeCamPosY -= freeCamSpeed; }

    gluLookAt(freeCamPosX, freeCamPosY, freeCamPosZ,
              freeCamPosX + fx, freeCamPosY + fy, freeCamPosZ + fz,
              0.0, 1.0, 0.0);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 'l' || key == 'L') {
        isSunLightOn = !isSunLightOn;
        if (isSunLightOn) glEnable(GL_LIGHT0); else glDisable(GL_LIGHT0);
    }

    if (key == 'b' || key == 'B') scalePohon += 0.1f;
    if (key == 'k' || key == 'K') { scalePohon -= 0.1f; if (scalePohon < 0.1f) scalePohon = 0.1f; }

    if (key == 'n' || key == 'N') scaleRocket += 0.1f;
    if (key == 'm' || key == 'M') { scaleRocket -= 0.1f; if (scaleRocket < 0.1f) scaleRocket = 0.1f; }

    if (key == 'w' || key == 'W') keyW = true;
    if (key == 's' || key == 'S') keyS = true;
    if (key == 'a' || key == 'A') keyA = true;
    if (key == 'd' || key == 'D') keyD = true;
    if (key == 'q' || key == 'Q') keyQ = true;
    if (key == 'e' || key == 'E') keyE = true;
    if (key == 27) exit(0);
}

void keyboardUp(unsigned char key, int x, int y) {
    if (key == 'w' || key == 'W') keyW = false;
    if (key == 's' || key == 'S') keyS = false;
    if (key == 'a' || key == 'A') keyA = false;
    if (key == 'd' || key == 'D') keyD = false;
    if (key == 'q' || key == 'Q') keyQ = false;
    if (key == 'e' || key == 'E') keyE = false;
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseLeftDown = true;
            mouseLastX = x;
            mouseLastY = y;
        } else if (state == GLUT_UP) {
            mouseLeftDown = false;
        }
    }
}

void mouseMotion(int x, int y) {
    if (mouseLeftDown) {
        float dx = x - mouseLastX;
        float dy = y - mouseLastY;
        freeCamYaw += dx * 0.3f;
        freeCamPitch -= dy * 0.3f;
        if (freeCamPitch > 89.0f) freeCamPitch = 89.0f;
        if (freeCamPitch < -89.0f) freeCamPitch = -89.0f;
        mouseLastX = x;
        mouseLastY = y;
        glutPostRedisplay();
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    rotasiBulan    += 0.05f;
    rotasiBumi     += 0.05f;
    rotasiMatahari += 0.01f;
    orbitUFO       += 1.0f;
    rotasiUFO      += 2.0f;
    nozzle         += 0.5f;
    orbitBulan     += 0.5f;
    orbitBumi      += 0.3f;

    updateCamera();
    drawStars();
    drawMatahari();
    drawBumi();
    drawBulan();
    drawUFO();

    glutSwapBuffers();
    glutPostRedisplay();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    float ratio = (float)w / (float)h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    gluPerspective(60.0, ratio, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("TUGAS BESAR KELOMPOK 9");
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_NORMALIZE);

    setupLighting();
    initStars();

    texture_bulan_ID    = loadTexture("textures/bulan.png");
    texture_bumi_ID     = loadTexture("textures/bumi.png");
    texture_matahari_ID = loadTexture("textures/sun.png");

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);

    glutMainLoop();
    return 0;
}
