#include <GL/glew.h>
#include <GL/glut.h>
#include <FreeImage.h>
#include <stdio.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;

GLuint texture_bulan_ID;
GLuint texture_bumi_ID;
GLuint texture_matahari_ID;

// Variabel untuk rotasi dan orbit
float rotasiBulan = 0.0;
float orbitBulan = 0.0;
float rotasiBumi = 0.0;
float rotasiMatahari = 0.0;
float orbitUFO = 0.0;    // Variabel baru untuk orbit UFO
float rotasiUFO = 0.0;   // Variabel baru untuk rotasi UFO sendiri

// ================= ROCKET ANIMATION =================
int rocketPhase = 0;   // 0=Bumi->Bulan, 1=Bulan->Matahari, 2=Matahari->Bumi
float rocketT = 0.0f; // progress animasi
float rocketSpeed = 0.002f;

// Posisi roket
float rocketX = 0.0f;
float rocketY = 0.0f;
float rocketZ = 0.0f;


// Jarak objek
float jarakBumi = 40.0;
float jarakMatahari = 150.0;
float jarakOrbitUFO = 5.0; // Jarak orbit UFO dari Bulan

// Posisi cahaya
GLfloat light_position[] = { 80.0, 0.0, 0.0, 1.0 };
GLfloat light_ambient[] = { 0.15, 0.15, 0.15, 1.0 };
GLfloat light_diffuse[] = { 1.0, 1.0, 0.9, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };

// ================= SISTEM KAMERA GANDA =================
enum CameraMode {
    CAMERA_FIXED,    // Kamera fokus ke Bulan (default)
    CAMERA_FREE      // Kamera bebas bergerak
};

CameraMode currentCameraMode = CAMERA_FIXED;

// Variabel untuk kamera fixed (fokus ke Bulan)
float kameraDistance = 8.0;
float kameraAngleX = 45.0;
float kameraAngleY = 45.0;

// Variabel untuk kamera bebas
float freeCamPosX = 0.0;
float freeCamPosY = 5.0;
float freeCamPosZ = 15.0;
float freeCamYaw = 0.0;    // Rotasi horizontal
float freeCamPitch = 0.0;  // Rotasi vertikal
float freeCamSpeed = 0.5f; // Kecepatan gerak kamera

// Variabel kontrol keyboard kamera bebas - DIPERBAIKI
bool keyW = false, keyA = false, keyS = false, keyD = false;
bool keyQ = false, keyE = false;

// Variabel mouse
int mouseLastX = 0;
int mouseLastY = 0;
bool mouseLeftDown = false;
bool mouseRightDown = false;

// Struktur untuk bintang
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

// ================= FUNGSI KAMERA BEBAS YANG DIPERBAIKI =================
void updateFreeCamera() {
    // Hitung arah pandang berdasarkan yaw dan pitch
    float yawRad = freeCamYaw * M_PI / 180.0f;
    float pitchRad = freeCamPitch * M_PI / 180.0f;
    
    // Vektor arah depan (berdasarkan pandangan kamera)
    float forwardX = sin(yawRad) * cos(pitchRad);
    float forwardY = sin(pitchRad);
    float forwardZ = cos(yawRad) * cos(pitchRad);
    
    // Normalize vektor depan
    float length = sqrt(forwardX*forwardX + forwardY*forwardY + forwardZ*forwardZ);
    if (length > 0.0001f) {
        forwardX /= length;
        forwardY /= length;
        forwardZ /= length;
    }
    
    // Vektor kanan (cross product dengan up vector)
    float rightX = cos(yawRad);
    float rightY = 0;
    float rightZ = -sin(yawRad);
    
    // Normalize vektor kanan
    length = sqrt(rightX*rightX + rightY*rightY + rightZ*rightZ);
    if (length > 0.0001f) {
        rightX /= length;
        rightY /= length;
        rightZ /= length;
    }
    
    // Update posisi berdasarkan input keyboard
    // Kontrol WASD: Gerakan relatif terhadap arah pandang
    if (keyW) { // MAJU (depan)
        freeCamPosX += forwardX * freeCamSpeed;
        freeCamPosY += forwardY * freeCamSpeed;
        freeCamPosZ += forwardZ * freeCamSpeed;
    }
    if (keyS) { // MUNDUR (belakang)
        freeCamPosX -= forwardX * freeCamSpeed;
        freeCamPosY -= forwardY * freeCamSpeed;
        freeCamPosZ -= forwardZ * freeCamSpeed;
    }
    if (keyA) { // KIRI (relatif)
        freeCamPosX -= rightX * freeCamSpeed;
        freeCamPosZ -= rightZ * freeCamSpeed;
    }
    if (keyD) { // KANAN (relatif)
        freeCamPosX += rightX * freeCamSpeed;
        freeCamPosZ += rightZ * freeCamSpeed;
    }
    
    // Kontrol gerakan vertikal Q/E
    if (keyQ) { // NAIK (atas)
        freeCamPosY += freeCamSpeed;
    }
    if (keyE) { // TURUN (bawah)
        freeCamPosY -= freeCamSpeed;
    }
}

void initStars() {
    srand(time(NULL));
    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].x = (rand() % 400 - 200) * 1.0f;
        stars[i].y = (rand() % 400 - 200) * 1.0f;
        stars[i].z = (rand() % 400 - 200) * 1.0f;
        
        float colorChoice = rand() % 100 / 100.0f;
        if (colorChoice < 0.7) {
            stars[i].r = 0.9 + (rand() % 20) / 100.0f;
            stars[i].g = 0.8 + (rand() % 30) / 100.0f;
            stars[i].b = 0.7 + (rand() % 30) / 100.0f;
        } else if (colorChoice < 0.9) {
            stars[i].r = 0.6 + (rand() % 20) / 100.0f;
            stars[i].g = 0.7 + (rand() % 20) / 100.0f;
            stars[i].b = 0.9 + (rand() % 20) / 100.0f;
        } else {
            stars[i].r = 0.9 + (rand() % 20) / 100.0f;
            stars[i].g = 0.5 + (rand() % 20) / 100.0f;
            stars[i].b = 0.4 + (rand() % 20) / 100.0f;
        }
        
        stars[i].brightness = 0.5 + (rand() % 50) / 100.0f;
        stars[i].pulseSpeed = 0.5 + (rand() % 100) / 100.0f;
        stars[i].pulsePhase = rand() % 100 / 100.0f * 2 * M_PI;
        stars[i].size = 1.0 + (rand() % 10) / 10.0f;
    }
}

void drawStars() {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    glPushMatrix();
    
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    
    static float time = 0.0f;
    time += 0.01f;
    
    for (int i = 0; i < NUM_STARS; i++) {
        float pulse = 0.7f + 0.3f * sin(time * stars[i].pulseSpeed + stars[i].pulsePhase);
        float currentBrightness = stars[i].brightness * pulse;
        
        glColor4f(stars[i].r * currentBrightness,
                  stars[i].g * currentBrightness,
                  stars[i].b * currentBrightness,
                  1.0f);
        
        glVertex3f(stars[i].x, stars[i].y, stars[i].z);
        
        if (stars[i].size > 1.5) {
            glColor4f(stars[i].r * currentBrightness * 0.3,
                      stars[i].g * currentBrightness * 0.3,
                      stars[i].b * currentBrightness * 0.3,
                      0.5f);
            
            for (int j = 0; j < 2; j++) {
                float offsetX = (rand() % 100 - 50) / 100.0f * 0.1f;
                float offsetY = (rand() % 100 - 50) / 100.0f * 0.1f;
                float offsetZ = (rand() % 100 - 50) / 100.0f * 0.1f;
                
                glVertex3f(stars[i].x + offsetX,
                          stars[i].y + offsetY,
                          stars[i].z + offsetZ);
            }
        }
    }
    
    glEnd();
    
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    
    for (int i = 0; i < NUM_STARS; i++) {
        if (stars[i].brightness > 0.8) {
            float pulse = 0.8f + 0.2f * sin(time * stars[i].pulseSpeed * 0.5f + stars[i].pulsePhase);
            float currentBrightness = stars[i].brightness * pulse;
            
            glColor4f(stars[i].r * currentBrightness,
                      stars[i].g * currentBrightness,
                      stars[i].b * currentBrightness,
                      1.0f);
            
            glVertex3f(stars[i].x, stars[i].y, stars[i].z);
        }
    }
    
    glEnd();
    
    glPopMatrix();
    
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

void createObject(float radius) {      
    GLUquadric* object = gluNewQuadric();
    gluQuadricTexture(object, GL_TRUE);
    gluQuadricNormals(object, GLU_SMOOTH);
    gluSphere(object, radius, 150, 150);
}

void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.001);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.000001);
    
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_LIGHT1);

}

void drawMatahari() {
    glPushMatrix();
    glTranslatef(jarakMatahari, 0.0, 0.0);
    glRotatef(rotasiMatahari, 0.0, 1.0, 0.0);
    
    light_position[0] = jarakMatahari;
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
    GLfloat sun_emission[] = { 1.0, 1.0, 0.8, 1.0 };
    glMaterialfv(GL_FRONT, GL_EMISSION, sun_emission);
    glColor3f(1.0, 0.9, 0.3);
    
    glBindTexture(GL_TEXTURE_2D, texture_matahari_ID);
    createObject(5.0); 
    
    GLfloat no_emission[] = { 0.0, 0.0, 0.0, 1.0 };
    glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);
    
    glPopMatrix();
}

void drawBumi() {
    glPushMatrix();
    // Posisi Bumi di langit
    glTranslatef(jarakBumi, 15.0, -30.0);
    glRotatef(rotasiBumi, 0.0, 1.0, 0.0);
    glRotatef(-23.5, 0.0, 0.0, 1.0);
    
    GLfloat earth_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
    GLfloat earth_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
    GLfloat earth_specular[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat earth_shininess[] = { 5.0 };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, earth_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, earth_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, earth_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, earth_shininess);
    
    glBindTexture(GL_TEXTURE_2D, texture_bumi_ID);
    createObject(6.0); 
    
    glPopMatrix();
}

void drawBulan() {
    glPushMatrix();
    glRotatef(rotasiBulan, 0.0, 1.0, 0.0);
    
    GLfloat moon_ambient[] = { 0.05, 0.05, 0.05, 1.0 };
    GLfloat moon_diffuse[] = { 0.7, 0.7, 0.7, 1.0 };
    GLfloat moon_specular[] = { 0.1, 0.1, 0.1, 1.0 };
    GLfloat moon_shininess[] = { 5.0 };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, moon_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, moon_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, moon_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, moon_shininess);
    
    glBindTexture(GL_TEXTURE_2D, texture_bulan_ID);
    createObject(3.0);
    
    glPopMatrix();
}

// ================= FUNGSI UNTUK MENGGAMBAR UFO =================
void drawUFO() {
    glPushMatrix();

    // ================= ORBIT UFO =================
    float orbitRadius = jarakOrbitUFO;
    float rad = orbitUFO * M_PI / 180.0f;

    float ufoX = orbitRadius * cos(rad);
    float ufoZ = orbitRadius * sin(rad);

    glTranslatef(ufoX, 0.8f, ufoZ);
    glRotatef(rotasiUFO, 0.0f, 1.0f, 0.0f);

    glDisable(GL_TEXTURE_2D);

    // ================= MATERIAL METAL UFO =================
    GLfloat ufo_ambient[]  = {0.15f, 0.15f, 0.18f, 1.0f};
    GLfloat ufo_diffuse[]  = {0.45f, 0.45f, 0.55f, 1.0f};
    GLfloat ufo_specular[] = {0.9f,  0.9f,  1.0f,  1.0f};
    GLfloat ufo_shine[]    = {80.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT,  ufo_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  ufo_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, ufo_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, ufo_shine);

    // ================= BADAN UFO (PIRING) =================
    glPushMatrix();
    glScalef(1.6f, 0.35f, 1.6f);
    glutSolidSphere(1.0, 40, 40);
    glPopMatrix();

    // ================= KUBAH ATAS (KACA) =================
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat dome_emission[] = {0.2f, 0.6f, 1.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, dome_emission);

    glColor4f(0.2f, 0.6f, 1.0f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, 0.55f, 0.0f);
    glScalef(0.9f, 0.6f, 0.9f);
    glutSolidSphere(0.6, 30, 30);
    glPopMatrix();

    GLfloat no_emission[] = {0,0,0,1};
    glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);
    glDisable(GL_BLEND);

    // ================= RING BAWAH =================
    glColor3f(0.3f, 0.3f, 0.4f);
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    glutSolidTorus(0.08, 1.2, 20, 40);
    glPopMatrix();

    // ================= CAHAYA UFO (TRACTOR BEAM) =================
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glBegin(GL_TRIANGLE_FAN);
        glColor4f(0.4f, 0.8f, 1.0f, 0.5f);
        glVertex3f(0.0f, -0.2f, 0.0f);

        glColor4f(0.4f, 0.8f, 1.0f, 0.0f);
        for (int i = 0; i <= 360; i += 15) {
            float a = i * M_PI / 180.0f;
            glVertex3f(cos(a) * 2.0f, -6.0f, sin(a) * 2.0f);
        }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    glPopMatrix();
}

void drawRocket() {
    glPushMatrix();

    glTranslatef(rocketX, rocketY, rocketZ);

    // ================= ORIENTASI ROKET =================
    // Dari vertikal ? horizontal
    glRotatef(90, 0, 0, 1);

    // ================= MATERIAL UTAMA =================
    GLfloat body_ambient[]  = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat body_diffuse[]  = {0.85f, 0.85f, 0.85f, 1.0f};
    GLfloat body_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat body_shine[]    = {80.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT, body_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, body_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, body_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, body_shine);

    GLUquadric* quad = gluNewQuadric();

    // ================= BADAN ROKET =================
    glPushMatrix();
    gluCylinder(quad, 0.5, 0.5, 3.5, 30, 30);
    glPopMatrix();

    // ================= KEPALA ROKET =================
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 3.5f);
    gluCylinder(quad, 0.5, 0.0, 1.0, 30, 30);
    glPopMatrix();

    // ================= KACA (COCKPIT) =================
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat glass_emission[] = {0.2f, 0.6f, 1.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, glass_emission);

    glColor4f(0.3f, 0.7f, 1.0f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 2.5f);
    glutSolidSphere(0.4, 20, 20);
    glPopMatrix();

    GLfloat no_emission[] = {0,0,0,1};
    glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);
    glDisable(GL_BLEND);

    // ================= PINTU ROKET =================
    glColor3f(0.2f, 0.2f, 0.25f);
    glBegin(GL_QUADS);
        glVertex3f( 0.51f, -0.3f, 1.0f);
        glVertex3f( 0.51f,  0.3f, 1.0f);
        glVertex3f( 0.51f,  0.3f, 2.0f);
        glVertex3f( 0.51f, -0.3f, 2.0f);
    glEnd();

    // ================= SAYAP KIRI =================
    glBegin(GL_TRIANGLES);
        glVertex3f(-0.5f, 0.0f, 1.0f);
        glVertex3f(-1.5f, 0.0f, 0.5f);
        glVertex3f(-0.5f, 0.0f, 2.0f);
    glEnd();

    // ================= SAYAP KANAN =================
    glBegin(GL_TRIANGLES);
        glVertex3f(0.5f, 0.0f, 1.0f);
        glVertex3f(1.5f, 0.0f, 0.5f);
        glVertex3f(0.5f, 0.0f, 2.0f);
    glEnd();

    // ================= API ROKET =================
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 0.6f, 0.1f, 0.8f);
        glVertex3f(0.0f, 0.0f, -0.2f);

        glColor4f(1.0f, 0.2f, 0.0f, 0.0f);
        for (int i = 0; i <= 360; i += 20) {
            float a = i * M_PI / 180.0f;
            glVertex3f(cos(a) * 1.2f, sin(a) * 1.2f, -3.0f);
        }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    gluDeleteQuadric(quad);
    glPopMatrix();
}

void updateRocket() {
    rocketT += rocketSpeed;
    if (rocketT > 1.0f) {
        rocketT = 0.0f;
        rocketPhase = (rocketPhase + 1) % 3;
    }

    // Posisi target
    float sx, sy, sz;
    float ex, ey, ez;

    // Bumi
    float earthX = jarakBumi;
    float earthY = 15.0f;
    float earthZ = -30.0f;

    // Bulan
    float moonX = 0.0f;
    float moonY = 0.0f;
    float moonZ = 0.0f;

    // Matahari
    float sunX = jarakMatahari;
    float sunY = 0.0f;
    float sunZ = 0.0f;

    if (rocketPhase == 0) { // bumi -> bulan
        sx = earthX; sy = earthY; sz = earthZ;
        ex = moonX;  ey = moonY;  ez = moonZ;
    } else if (rocketPhase == 1) { // bulan -> matahari
        sx = moonX; sy = moonY; sz = moonZ;
        ex = sunX;  ey = sunY;  ez = sunZ;
    } else { // matahari -> bumi
        sx = sunX;  sy = sunY;  sz = sunZ;
        ex = earthX; ey = earthY; ez = earthZ;
    }

    // Interpolasi linear
    rocketX = sx + (ex - sx) * rocketT;
    rocketY = sy + (ey - sy) * rocketT;
    rocketZ = sz + (ez - sz) * rocketT;
}


// ================= FUNGSI UPDATE KAMERA =================
void updateCamera() {
    if (currentCameraMode == CAMERA_FIXED) {
        // Kamera fixed mengelilingi Bulan
        float camX = kameraDistance * cos(kameraAngleY * M_PI / 180.0) * sin(kameraAngleX * M_PI / 180.0);
        float camY = kameraDistance * sin(kameraAngleY * M_PI / 180.0);
        float camZ = kameraDistance * cos(kameraAngleY * M_PI / 180.0) * cos(kameraAngleX * M_PI / 180.0);
        
        gluLookAt(camX, camY, camZ,
                  0.0, 0.0, 0.0,
                  0.0, 1.0, 0.0);
    } 
    else { // CAMERA_FREE
        // Hitung target pandang untuk kamera bebas
        float yawRad = freeCamYaw * M_PI / 180.0f;
        float pitchRad = freeCamPitch * M_PI / 180.0f;
        
        float lookAtX = freeCamPosX + sin(yawRad) * cos(pitchRad);
        float lookAtY = freeCamPosY + sin(pitchRad);
        float lookAtZ = freeCamPosZ + cos(yawRad) * cos(pitchRad);
        
        gluLookAt(freeCamPosX, freeCamPosY, freeCamPosZ,
                  lookAtX, lookAtY, lookAtZ,
                  0.0, 1.0, 0.0);
    }
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        // Kontrol kamera fixed dan free
        case 'w': case 'W':
            if (currentCameraMode == CAMERA_FIXED) {
                kameraDistance -= 0.5;
                if (kameraDistance < 3.0) kameraDistance = 3.0;
            } else {
                keyW = true; // Untuk kamera bebas (MAJU)
            }
            break;
        case 's': case 'S':
            if (currentCameraMode == CAMERA_FIXED) {
                kameraDistance += 0.5;
                if (kameraDistance > 20.0) kameraDistance = 20.0;
            } else {
                keyS = true; // Untuk kamera bebas (MUNDUR)
            }
            break;
        case 'a': case 'A':
            if (currentCameraMode == CAMERA_FIXED) {
                kameraAngleX -= 5.0; // KIRI
            } else {
                keyA = true; 
            }
            break;
        case 'd': case 'D':
            if (currentCameraMode == CAMERA_FIXED) {
                kameraAngleX += 5.0; // KANAN
            } else {
                keyD = true;
            }
            break;
            
        // Kontrol vertikal Q/E
        case 'q': case 'Q':
            keyQ = true;
            break;
        case 'e': case 'E':
            keyE = true;
            break;
            
        // Preset kamera fixed
        case '1': 
            currentCameraMode = CAMERA_FIXED;
            kameraAngleX = 0.0;
            kameraAngleY = 0.0;
            break;
        case '2': 
            currentCameraMode = CAMERA_FIXED;
            kameraAngleX = 90.0;
            kameraAngleY = 0.0;
            break;
        case '3': 
            currentCameraMode = CAMERA_FIXED;
            kameraAngleX = 0.0;
            kameraAngleY = 85.0;
            break;
            
        // Ganti mode kamera
        case 'c': case 'C':
            currentCameraMode = (currentCameraMode == CAMERA_FIXED) ? CAMERA_FREE : CAMERA_FIXED;
            printf("Camera mode switched to: %s\n", 
                   (currentCameraMode == CAMERA_FIXED) ? "FIXED (orbit Moon)" : "FREE (fly)");
            break;
            
        // Reset semua
        case 'r': case 'R':
            currentCameraMode = CAMERA_FIXED;
            kameraDistance = 8.0;
            kameraAngleX = 45.0;
            kameraAngleY = 45.0;
            rotasiBulan = 0.0;
            rotasiBumi = 0.0;
            rotasiMatahari = 0.0;
            orbitUFO = 0.0;    // Reset orbit UFO
            rotasiUFO = 0.0;   // Reset rotasi UFO

            // Reset kamera bebas
            freeCamPosX = 0.0;
            freeCamPosY = 5.0;
            freeCamPosZ = 15.0;
            freeCamYaw = 0.0;
            freeCamPitch = 0.0;
            
            // Reset semua key states
            keyW = keyS = keyA = keyD = false;
            keyQ = keyE = false;
            break;
            
        // Kontrol kecepatan UFO
        case 'u': case 'U':
            jarakOrbitUFO += 0.5; // Perbesar orbit UFO
            if (jarakOrbitUFO > 10.0) jarakOrbitUFO = 10.0;
            break;
        case 'i': case 'I':
            jarakOrbitUFO -= 0.5; // Perkecil orbit UFO
            if (jarakOrbitUFO < 2.0) jarakOrbitUFO = 2.0;
            break;
            
        // Keluar
        case 27: // ESC
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void keyboardUp(unsigned char key, int x, int y) {
    switch(key) {
        case 'w': case 'W':
            keyW = false;
            break;
        case 's': case 'S':
            keyS = false;
            break;
        case 'a': case 'A':
            keyA = false;
            break;
        case 'd': case 'D':
            keyD = false;
            break;
        case 'q': case 'Q':
            keyQ = false;
            break;
        case 'e': case 'E':
            keyE = false;
            break;
    }
}

// ================= FUNGSI MOUSE =================
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseLeftDown = true;
            mouseLastX = x;
            mouseLastY = y;
        } else if (state == GLUT_UP) {
            mouseLeftDown = false;
        }
    } else if (button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseRightDown = true;
            mouseLastY = y;
        } else if (state == GLUT_UP) {
            mouseRightDown = false;
        }
    } else if (state == GLUT_DOWN && button == 3) {
        if (currentCameraMode == CAMERA_FIXED) {
            kameraDistance -= 1.0;
            if (kameraDistance < 3.0) kameraDistance = 3.0;
        } else {
            freeCamSpeed += 0.1f;
            if (freeCamSpeed > 3.0f) freeCamSpeed = 3.0f;
        }
        glutPostRedisplay();
    } else if (state == GLUT_DOWN && button == 4) {
        if (currentCameraMode == CAMERA_FIXED) {
            kameraDistance += 1.0;
            if (kameraDistance > 20.0) kameraDistance = 20.0;
        } else {
            freeCamSpeed -= 0.1f;
            if (freeCamSpeed < 0.1f) freeCamSpeed = 0.1f;
        }
        glutPostRedisplay();
    }
}

void mouseMotion(int x, int y) {
    if (mouseLeftDown) {
        if (currentCameraMode == CAMERA_FIXED) {
            // Kontrol kamera fixed
            float deltaX = x - mouseLastX;
            float deltaY = y - mouseLastY;
            
            kameraAngleX += deltaX * 0.5;
            kameraAngleY += deltaY * 0.5;
            
            if (kameraAngleY > 85.0) kameraAngleY = 85.0;
            if (kameraAngleY < -85.0) kameraAngleY = -85.0;
        } 
        else {
            // Kontrol kamera bebas (menggerakkan pandangan)
            float deltaX = x - mouseLastX;
            float deltaY = y - mouseLastY;
            
            freeCamYaw += deltaX * 0.3f;
            freeCamPitch -= deltaY * 0.3f;
            
            // Batasi pitch agar tidak over-rotate
            if (freeCamPitch > 89.0f) freeCamPitch = 89.0f;
            if (freeCamPitch < -89.0f) freeCamPitch = -89.0f;
        }
        
        mouseLastX = x;
        mouseLastY = y;
        glutPostRedisplay();
    }
}

// ================= FUNGSI UTAMA =================
GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    void* imgData;
    int imgWidth, imgHeight;

    FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(path);
    if (format == FIF_UNKNOWN) {
        printf("Unknown file type for texture %s\n", path);
        return 0;
    }

    FIBITMAP* bitmap = FreeImage_Load(format, path, 0);
    if (!bitmap) {
        printf("Failed to load %s\n", path);
        return 0;
    }

    FIBITMAP* bitmap2 = FreeImage_ConvertTo24Bits(bitmap);
    FreeImage_Unload(bitmap);

    imgData = FreeImage_GetBits(bitmap2);
    imgWidth = FreeImage_GetWidth(bitmap2);
    imgHeight = FreeImage_GetHeight(bitmap2);

    if (imgData) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        int formatColor = (FI_RGBA_RED == 0) ? GL_RGB : GL_BGR;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgWidth, imgHeight, 0,
            formatColor, GL_UNSIGNED_BYTE, imgData);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        FreeImage_Unload(bitmap2);
        return textureID;
    }
    
    FreeImage_Unload(bitmap2);
    return 0;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // ================= UPDATE ANIMASI =================
    rotasiBulan += 0.05f;
    rotasiBumi += 0.05f;
    rotasiMatahari += 0.01f;
    orbitUFO += 1.0f;
    rotasiUFO += 2.0f;

    // Update roket
    updateRocket();

    // ================= UPDATE KAMERA =================
    if (currentCameraMode == CAMERA_FREE) {
        updateFreeCamera();
    }
    updateCamera();

    // ================= GAMBAR SEMUA OBJEK =================
    drawStars();
    drawMatahari();
    drawBumi();
    drawBulan();
    drawUFO();
    drawRocket();   // ?? SEKARANG ROKET PASTI TERGAMBAR

    // ================= FINAL FRAME =================
    glutSwapBuffers();
    glutPostRedisplay();
}

void initProjection() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_SMOOTH);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_NORMALIZE);
    setupLighting();
    
    initStars();
    
    texture_bulan_ID = loadTexture("textures/bulan.png");
    texture_bumi_ID = loadTexture("textures/bumi.png");
    texture_matahari_ID = loadTexture("textures/sun.png");
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 800.0 / 600.0, 0.1, 1000.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("TB-kelompok 9 - UFO Mengorbit Bulan");
    
    glewInit();
    
    initProjection();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    
    glutMainLoop();
    return 0;
}
