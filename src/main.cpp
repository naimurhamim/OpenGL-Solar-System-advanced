#include <GL/glut.h>
#include <stdio.h>
#include <GL/gl.h>
#include <math.h>
#include <time.h>
#include <string.h>

// ==================== GLOBAL STATE ====================
float angle      = 0.0f;
float selfRotate = 0.0f;
float zoom       = 25.0f;
float targetZoom = 25.0f;
float speed      = 0.001f;
float camX       = 0.0f;
float camY       = 0.0f;
int   followPlanet   = 0;
int   selectedPlanet = 0;

// Star twinkle
float starBrightness = 1.0f;
bool  increaseStar   = false;

// Toggles
bool showOrbits = true;
bool showLabels = true;
bool nightMode  = false;
bool showLighting = true;

// Sun pulse
float sunPulse     = 0.0f;
bool  sunPulseGrow = true;

// Comet
float cometAngle = 0.0f;

// FPS
int   frameCount = 0;
float fps        = 0.0f;
int   lastTime   = 0;

// Planet positions (global so camera + info panel + mouse share them)
float budhX,  budhY;
float sukroX, sukroY;
float earthX, earthY;
float marsX,  marsY;
float jupX,   jupY;
float satX,   satY;
float uraX,   uraY;
float nepX,   nepY;

// ==================== HELPERS ====================

void applyProjection()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-zoom + camX,  zoom + camX,
            -zoom + camY,  zoom + camY,
            -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void init()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    applyProjection();
}

// Filled ellipse
void circle(GLfloat rx, GLfloat ry, GLfloat cx, GLfloat cy)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for(int i = 0; i <= 100; i++)
    {
        float a = 2.0f * 3.1416f * i / 100;
        glVertex2f(rx * cosf(a) + cx, ry * sinf(a) + cy);
    }
    glEnd();
}

// Orbit ring
void orbit(float radius)
{
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < 200; i++)
    {
        float a = 2.0f * 3.1416f * i / 200;
        glVertex2f(radius * cosf(a), radius * sinf(a));
    }
    glEnd();
}

// World-space text
void drawText(float x, float y, const char *text)
{
    glRasterPos2f(x, y);
    for(int i = 0; text[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, text[i]);
}

// Distance between two points
float dist2(float ax, float ay, float bx, float by)
{
    float dx = ax - bx, dy = ay - by;
    return sqrtf(dx*dx + dy*dy);
}

void drawShadowOverlay(float px, float py, float rx, float ry, float shadowStrength)
{
    if(!showLighting) return;

    float dx = px, dy = py;
    float dist = sqrtf(dx*dx + dy*dy);
    if(dist < 0.001f) return;

    float lx = dx / dist;
    float ly = dy / dist;

    // Gradient shadow — lit side থেকে dark side এ fade
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.0f, 0.0f, 0.0f, shadowStrength * 0.5f);
    glVertex2f(px + lx * rx * 0.3f, py + ly * ry * 0.3f);
    for(int i = 0; i <= 100; i++)
    {
        float a   = 2.0f * 3.1416f * i / 100;
        float vx  = cosf(a);
        float vy  = sinf(a);
        float dot = vx * lx + vy * ly;
        float alpha = (dot > 0.0f) ? dot * dot * shadowStrength : 0.0f;
        glColor4f(0.0f, 0.0f, 0.0f, alpha);
        glVertex2f(rx * vx + px, ry * vy + py);
    }
    glEnd();
}

// ==================== CAMERA ====================

void updateCamera()
{
    if      (followPlanet == 0)  { camX = 0;      camY = 0;      }
    else if (followPlanet == 1)  { camX = budhX;  camY = budhY;  }
    else if (followPlanet == 2)  { camX = sukroX; camY = sukroY; }
    else if (followPlanet == 3)  { camX = earthX; camY = earthY; }
    else if (followPlanet == 4)  { camX = marsX;  camY = marsY;  }
    else if (followPlanet == 5)  { camX = jupX;   camY = jupY;   }
    else if (followPlanet == 6)  { camX = satX;   camY = satY;   }
    else if (followPlanet == 7)  { camX = uraX;   camY = uraY;   }
    else if (followPlanet == 8)  { camX = nepX;   camY = nepY;   }
    // followPlanet == -1 : free cam, camX/camY via arrow keys
}

// ==================== INFO PANEL ====================

void drawInfoPanel()
{
    float px = -zoom + camX + 1.0f;
    float py =  zoom + camY - 2.0f;
    float lh =  zoom * 0.09f;

    const char *names[] = {"SUN","Mercury","Venus","Earth","Mars","Jupiter","Saturn","Uranus","Neptune"};
    const char *moons[] = {"-","0","0","1","2","3","1","1","1"};
    const char *radii[] = {"-","5","7","9","11","14","17","20","23"};
    const char *types[] = {"Center Star","Rocky","Rocky","Rocky","Rocky","Gas Giant","Gas Giant","Ice Giant","Ice Giant"};

    char buf[64];

    glColor3f(0.0f, 0.8f, 1.0f);
    drawText(px, py, "[ SOLAR SYSTEM INFO ]");

    glColor3f(1,1,1);
    sprintf(buf, "Body  : %s", names[selectedPlanet]);  drawText(px, py - lh,   buf);
    sprintf(buf, "Type  : %s", types[selectedPlanet]);  drawText(px, py - lh*2, buf);
    sprintf(buf, "Moons : %s", moons[selectedPlanet]);  drawText(px, py - lh*3, buf);
    sprintf(buf, "Orbit : %s", radii[selectedPlanet]);  drawText(px, py - lh*4, buf);

    // FPS
    glColor3f(0.4f, 1.0f, 0.4f);
    sprintf(buf, "FPS   : %.1f", fps);
    drawText(px, py - lh*5.5f, buf);

    // Speed
    glColor3f(1.0f, 0.8f, 0.2f);
    sprintf(buf, "Speed : %.4f", speed);
    drawText(px, py - lh*6.5f, buf);

    // Controls hint � bottom left
    float hy = -zoom + camY + 0.4f;
    glColor3f(0.45f, 0.45f, 0.45f);
    drawText(px, hy + lh*5, "1-8: Follow  0: Sun  F: Free cam");
    drawText(px, hy + lh*4, "+/-: Zoom    [/]: Speed");
    drawText(px, hy + lh*3, "O: Orbits  L: Labels  N: Night");
    drawText(px, hy + lh*2, "Arrow: Pan (Free cam)");
    drawText(px, hy + lh,   "P: Pause  R: Resume");
    drawText(px, hy,        "Click: Select & follow planet");
}

// ==================== MINI-MAP ====================

void drawMiniMap()
{
    float mx   =  zoom + camX - 7.5f;
    float my   = -zoom + camY + 7.5f;
    float ms   = 6.5f;
    float sc   = ms / 25.0f;

    // Background
    glColor4f(0.0f, 0.0f, 0.15f, 0.75f);
    glBegin(GL_QUADS);
    glVertex2f(mx-ms, my-ms); glVertex2f(mx+ms, my-ms);
    glVertex2f(mx+ms, my+ms); glVertex2f(mx-ms, my+ms);
    glEnd();

    // Border
    glColor3f(0.3f, 0.3f, 0.6f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(mx-ms, my-ms); glVertex2f(mx+ms, my-ms);
    glVertex2f(mx+ms, my+ms); glVertex2f(mx-ms, my+ms);
    glEnd();

    // Title
    glColor3f(0.5f, 0.5f, 0.7f);
    drawText(mx - ms + 0.3f, my + ms - 0.9f, "MINIMAP");

    // Orbits on minimap
    glColor3f(0.2f, 0.2f, 0.3f);
    float oRadii[] = {5,7,9,11,14,17,20,23};
    for(int i = 0; i < 8; i++)
    {
        glBegin(GL_LINE_LOOP);
        for(int j = 0; j < 60; j++)
        {
            float a = 2*3.1416f*j/60;
            glVertex2f(mx + oRadii[i]*sc*cosf(a), my + oRadii[i]*sc*sinf(a));
        }
        glEnd();
    }

    // Sun
    glColor3f(1.0f, 1.0f, 0.0f);
    circle(0.35f, 0.35f, mx, my);

    // Planets
    float wx[] = {0, budhX, sukroX, earthX, marsX, jupX, satX, uraX, nepX};
    float wy[] = {0, budhY, sukroY, earthY, marsY, jupY, satY, uraY, nepY};
    float col[][3] = {
        {1,1,0},{0.8f,0.5f,0.2f},{1,0.9f,0.3f},
        {0,0.5f,1},{1,0.2f,0.2f},{1,0.6f,0.2f},
        {0.9f,0.8f,0.5f},{0.5f,1,1},{0.2f,0.2f,1}
    };
    for(int i = 1; i <= 8; i++)
    {
        glColor3f(col[i][0], col[i][1], col[i][2]);
        circle(0.28f, 0.28f, mx + wx[i]*sc, my + wy[i]*sc);
    }

    // Viewport rectangle
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex2f(mx + (-zoom+camX)*sc, my + (-zoom+camY)*sc);
    glVertex2f(mx + ( zoom+camX)*sc, my + (-zoom+camY)*sc);
    glVertex2f(mx + ( zoom+camX)*sc, my + ( zoom+camY)*sc);
    glVertex2f(mx + (-zoom+camX)*sc, my + ( zoom+camY)*sc);
    glEnd();
}

// ==================== DRAW ====================

void Draw()
{
    // --- Planet positions ---
    budhX  = 5  * cosf(angle * 1.8f + 1);
    budhY  = 5  * sinf(angle * 1.8f + 1);
    sukroX = 7  * cosf(angle * 1.5f + 2);
    sukroY = 7  * sinf(angle * 1.5f + 2);
    earthX = 9  * cosf(angle + 0.5f);
    earthY = 9  * sinf(angle + 0.5f);
    marsX  = 11 * cosf(angle * 0.8f + 3);
    marsY  = 11 * sinf(angle * 0.8f + 3);
    jupX   = 14 * cosf(angle * 0.5f + 1.5f);
    jupY   = 14 * sinf(angle * 0.5f + 1.5f);
    satX   = 17 * cosf(angle * 0.4f + 2.5f);
    satY   = 17 * sinf(angle * 0.4f + 2.5f);
    uraX   = 20 * cosf(angle * 0.3f + 4);
    uraY   = 20 * sinf(angle * 0.3f + 4);
    nepX   = 23 * cosf(angle * 0.2f + 5);
    nepY   = 23 * sinf(angle * 0.2f + 5);

    // --- Camera ---
    updateCamera();
    applyProjection();

    // --- Background ---
    if(nightMode) glClearColor(0.0f, 0.0f, 0.04f, 1.0f);
    else          glClearColor(0.0f, 0.0f, 0.0f,  1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // ==================== STARS ====================
    srand(42);
    glPointSize(2);
    glBegin(GL_POINTS);
    for(int i = 0; i < 400; i++)
    {
        float b  = starBrightness * (0.5f + (rand() % 10) / 20.0f);
        float sx = (float)(rand() % 120) - 60.0f;
        float sy = (float)(rand() % 120) - 60.0f;
        glColor3f(b, b, b);
        glVertex2f(sx, sy);
    }
    glEnd();

    // ==================== ORBITS ====================
    if(showOrbits)
    {
        glColor3f(0.22f, 0.22f, 0.32f);
        orbit(5); orbit(7); orbit(9); orbit(11);
        orbit(14); orbit(17); orbit(20); orbit(23);
    }

    // ==================== ASTEROID BELT ====================
    srand(99);
    glPointSize(1);
    glBegin(GL_POINTS);
    for(int i = 0; i < 350; i++)
    {
        float r   = 12.0f + (rand() % 100) / 50.0f;
        float a   = 2.0f * 3.1416f * (rand() % 1000) / 1000.0f;
        float brt = 0.25f + (rand() % 40) / 100.0f;
        glColor3f(brt, brt * 0.85f, brt * 0.65f);
        glVertex2f(r * cosf(a + angle * 0.08f), r * sinf(a + angle * 0.08f));
    }
    glEnd();

    // ==================== SUN ====================
    // Glow layers
    glColor4f(1.0f, 0.5f, 0.0f, 0.05f);
    circle(6.0f + sunPulse, 6.0f + sunPulse, 0, 0);
    glColor4f(1.0f, 0.7f, 0.0f, 0.08f);
    circle(5.0f + sunPulse, 5.0f + sunPulse, 0, 0);
    glColor4f(1.0f, 0.9f, 0.0f, 0.13f);
    circle(4.0f + sunPulse * 0.6f, 4.0f + sunPulse * 0.6f, 0, 0);
    glColor4f(1.0f, 1.0f, 0.3f, 0.20f);
    circle(3.5f + sunPulse * 0.3f, 3.5f + sunPulse * 0.3f, 0, 0);
    // Sun body
    glColor3f(1.0f, 0.95f, 0.0f);
    circle(3.0f + sunPulse * 0.25f, 3.0f + sunPulse * 0.25f, 0, 0);

    if(showLabels) { glColor3f(1,1,1); drawText(-0.8f, -4.3f, "SUN"); }

    // ==================== MERCURY ====================
    glColor3f(0.8f, 0.5f, 0.2f);
    circle(0.3f, 0.3f, budhX, budhY);
    drawShadowOverlay(budhX, budhY, 0.3f, 0.3f, 0.85f);
    if(showLabels) { glColor3f(1,1,1); drawText(budhX+0.4f, budhY+0.4f, "Mercury"); }

    // ==================== VENUS ====================
    glColor3f(1.0f, 0.9f, 0.3f);
    circle(0.5f, 0.5f, sukroX, sukroY);
    drawShadowOverlay(sukroX, sukroY, 0.5f, 0.5f, 0.80f);
    if(showLabels) { glColor3f(1,1,1); drawText(sukroX+0.6f, sukroY+0.6f, "Venus"); }

    // ==================== EARTH ====================
    glPushMatrix();
    glTranslatef(earthX, earthY, 0);
    glRotatef(selfRotate, 0, 0, 1);
    glColor3f(0.0f, 0.45f, 1.0f);
    circle(0.6f, 0.6f, 0, 0);
    // Green continent hint
    glColor3f(0.1f, 0.55f, 0.1f);
    circle(0.25f, 0.18f,  0.18f,  0.08f);
    circle(0.15f, 0.12f, -0.2f, -0.1f);
    glPopMatrix();
    drawShadowOverlay(earthX, earthY, 0.6f, 0.6f, 0.80f);
    if(showLabels) { glColor3f(1,1,1); drawText(earthX+0.7f, earthY+0.7f, "Earth"); }

    // Earth Moon orbit
    glColor3f(0.28f, 0.28f, 0.28f);
    glPushMatrix();
    glTranslatef(earthX, earthY, 0);
    orbit(1.2f);
    glPopMatrix();

    float moonX = earthX + 1.2f * cosf(angle * 4);
    float moonY = earthY + 1.2f * sinf(angle * 4);
    glColor3f(0.8f, 0.8f, 0.8f);
    circle(0.15f, 0.15f, moonX, moonY);
    if(showLabels) { glColor3f(0.75f,0.75f,0.75f); drawText(moonX+0.2f, moonY+0.2f, "Moon"); }

    // ==================== MARS ====================
    glColor3f(1.0f, 0.22f, 0.22f);
    circle(0.5f, 0.5f, marsX, marsY);
    drawShadowOverlay(marsX, marsY, 0.5f, 0.5f, 0.85f);
    if(showLabels) { glColor3f(1,1,1); drawText(marsX+0.6f, marsY+0.6f, "Mars"); }

    float mm1x = marsX + 0.9f * cosf(angle * 5);
    float mm1y = marsY + 0.9f * sinf(angle * 5);
    float mm2x = marsX + 1.3f * cosf(angle * 3);
    float mm2y = marsY + 1.3f * sinf(angle * 3);
    glColor3f(0.7f, 0.7f, 0.7f);
    circle(0.10f, 0.10f, mm1x, mm1y);
    circle(0.12f, 0.12f, mm2x, mm2y);

    // ==================== JUPITER ====================
    // Base
    glPushMatrix();
    glTranslatef(jupX, jupY, 0);
    glRotatef(selfRotate * 0.7f, 0, 0, 1);
    glColor3f(1.0f, 0.6f, 0.2f);
    circle(1.0f, 1.0f, 0, 0);
    // Horizontal bands
    float bc[][3] = {{0.85f,0.45f,0.1f},{1.0f,0.72f,0.35f},{0.78f,0.38f,0.08f},{0.95f,0.62f,0.22f}};
    float by[]    = {0.52f, 0.18f, -0.18f, -0.52f};
    for(int b = 0; b < 4; b++)
    {
        glColor3f(bc[b][0], bc[b][1], bc[b][2]);
        glBegin(GL_QUADS);
        glVertex2f(-1.0f, by[b]-0.13f);
        glVertex2f( 1.0f, by[b]-0.13f);
        glVertex2f( 1.0f, by[b]+0.13f);
        glVertex2f(-1.0f, by[b]+0.13f);
        glEnd();
    }
    glPopMatrix();
    drawShadowOverlay(jupX, jupY, 1.0f, 1.0f, 0.75f);
    if(showLabels) { glColor3f(1,1,1); drawText(jupX+1.1f, jupY+1.1f, "Jupiter"); }

    for(int i = 0; i < 3; i++)
    {
        float jmx = jupX + (1.55f + i*0.4f) * cosf(angle*(2+i) + i);
        float jmy = jupY + (1.55f + i*0.4f) * sinf(angle*(2+i) + i);
        glColor3f(0.9f, 0.9f, 0.9f);
        circle(0.12f, 0.12f, jmx, jmy);
    }

    // ==================== SATURN ====================
    glColor3f(0.9f, 0.8f, 0.5f);
    circle(0.9f, 0.9f, satX, satY);
    if(showLabels) { glColor3f(1,1,1); drawText(satX+1.0f, satY+1.0f, "Saturn"); }

    // Inner ring (B ring)
    glColor4f(0.85f, 0.75f, 0.45f, 0.75f);
    glBegin(GL_QUAD_STRIP);
    for(int i = 0; i <= 200; i++) {
        float a = 2*3.1416f*i/200;
        glVertex2f(1.05f*cosf(a)+satX, 0.40f*sinf(a)+satY);
        glVertex2f(1.30f*cosf(a)+satX, 0.50f*sinf(a)+satY);
    }
    glEnd();

    // Cassini Division (dark gap)
    glColor4f(0.0f, 0.0f, 0.0f, 0.85f);
    glBegin(GL_QUAD_STRIP);
    for(int i = 0; i <= 200; i++) {
        float a = 2*3.1416f*i/200;
        glVertex2f(1.30f*cosf(a)+satX, 0.50f*sinf(a)+satY);
        glVertex2f(1.40f*cosf(a)+satX, 0.54f*sinf(a)+satY);
    }
    glEnd();

    // Outer ring (A ring)
    glColor4f(0.78f, 0.65f, 0.35f, 0.60f);
    glBegin(GL_QUAD_STRIP);
    for(int i = 0; i <= 200; i++) {
        float a = 2*3.1416f*i/200;
        glVertex2f(1.40f*cosf(a)+satX, 0.54f*sinf(a)+satY);
        glVertex2f(1.72f*cosf(a)+satX, 0.66f*sinf(a)+satY);
    }
    glEnd();

    glColor3f(0.9f, 0.8f, 0.5f);
    circle(0.9f, 0.9f, satX, satY);
    drawShadowOverlay(satX, satY, 0.9f, 0.9f, 0.75f);

    // Saturn moon
    float smx = satX + 1.95f * cosf(angle * 2);
    float smy = satY + 1.95f * sinf(angle * 2);
    glColor3f(0.8f, 0.8f, 0.8f);
    circle(0.15f, 0.15f, smx, smy);

    // ==================== URANUS ====================
    glColor3f(0.5f, 1.0f, 1.0f);
    circle(0.7f, 0.7f, uraX, uraY);
    drawShadowOverlay(uraX, uraY, 0.7f, 0.7f, 0.78f);
    // Tilted ring
    glColor4f(0.5f, 0.9f, 0.9f, 0.55f);
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < 100; i++)
    {
        float a = 2*3.1416f*i/100;
        glVertex2f(0.3f*cosf(a)+uraX, 1.25f*sinf(a)+uraY);
    }
    glEnd();
    if(showLabels) { glColor3f(1,1,1); drawText(uraX+0.8f, uraY+0.8f, "Uranus"); }

    float umx = uraX + 1.25f * cosf(angle * 2.5f);
    float umy = uraY + 1.25f * sinf(angle * 2.5f);
    glColor3f(0.9f, 0.9f, 0.9f);
    circle(0.12f, 0.12f, umx, umy);

    // ==================== NEPTUNE ====================
    glColor3f(0.15f, 0.15f, 1.0f);
    circle(0.7f, 0.7f, nepX, nepY);
    drawShadowOverlay(nepX, nepY, 0.7f, 0.7f, 0.80f);
    if(showLabels) { glColor3f(1,1,1); drawText(nepX+0.8f, nepY+0.8f, "Neptune"); }

    float nmx = nepX + 1.4f * cosf(angle * 1.8f);
    float nmy = nepY + 1.4f * sinf(angle * 1.8f);
    glColor3f(0.9f, 0.9f, 0.9f);
    circle(0.13f, 0.13f, nmx, nmy);

    // ==================== COMET ====================
    float cometR = 19.0f;
    float cometCX = cometR * cosf(cometAngle);
    float cometCY = cometR * 0.38f * sinf(cometAngle);

    // Tail
    for(int t = 1; t <= 14; t++)
    {
        float ta  = cometAngle - t * 0.055f;
        float tx  = cometR * cosf(ta);
        float ty  = cometR * 0.38f * sinf(ta);
        float alp = 1.0f - (float)t / 15.0f;
        glColor4f(0.75f, 0.88f, 1.0f, alp * 0.65f);
        circle((0.09f * alp + 0.02f), (0.09f * alp + 0.02f), tx, ty);
    }
    // Head
    glColor3f(0.9f, 0.96f, 1.0f);
    circle(0.2f, 0.2f, cometCX, cometCY);
    if(showLabels) { glColor3f(0.6f,0.85f,1.0f); drawText(cometCX+0.3f, cometCY+0.3f, "Comet"); }

    // ==================== INFO PANEL ====================
    drawInfoPanel();

    // ==================== MINI-MAP ====================
    //drawMiniMap();  //To see the mini map, remove the comment

    glutSwapBuffers();

    // FPS update
    frameCount++;
    int now = glutGet(GLUT_ELAPSED_TIME);
    if(now - lastTime >= 500)
    {
        fps        = frameCount * 1000.0f / (now - lastTime);
        frameCount = 0;
        lastTime   = now;
    }
}

// ==================== MOUSE ====================

void mouse(int button, int state, int x, int y)
{
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        int   w  = glutGet(GLUT_WINDOW_WIDTH);
        int   h  = glutGet(GLUT_WINDOW_HEIGHT);
        float wx = ((float)x / w) * (2*zoom) + (-zoom + camX);
        float wy = ((float)(h - y) / h) * (2*zoom) + (-zoom + camY);

        float pxA[]   = {0, budhX, sukroX, earthX, marsX, jupX, satX, uraX, nepX};
        float pyA[]   = {0, budhY, sukroY, earthY, marsY, jupY, satY, uraY, nepY};
        float radii[] = {3.0f, 0.3f, 0.5f, 0.6f, 0.5f, 1.0f, 0.9f, 0.7f, 0.7f};
        float zooms[] = {12,    5,    5,    5,    5,    7,    7,    6,    6};

        int   best  = -1;
        float bestD = 9999;
        for(int i = 0; i <= 8; i++)
        {
            float d = dist2(wx, wy, pxA[i], pyA[i]);
            if(d < bestD && d < radii[i] * 3.5f)
            {
                bestD = d;
                best  = i;
            }
        }

        if(best >= 0)
        {
            selectedPlanet = best;
            followPlanet   = best;
            targetZoom     = zooms[best];
        }
    }
}

// ==================== KEYBOARD ====================

void keyboard(unsigned char key, int x, int y)
{
    if(key == '+') { targetZoom -= 2.0f; if(targetZoom < 1.5f)  targetZoom = 1.5f;  }
    if(key == '-') { targetZoom += 2.0f; if(targetZoom > 50.0f) targetZoom = 50.0f; }

    if(key == ']') { speed += 0.0005f; if(speed > 0.012f) speed = 0.012f; }
    if(key == '[') { speed -= 0.0005f; if(speed < 0.0f)   speed = 0.0f;   }

    if(key == 'p' || key == 'P') speed = 0.0f;
    if(key == 'r' || key == 'R') speed = 0.001f;
    if(key == 'o' || key == 'O') showOrbits = !showOrbits;
    if(key == 'l' || key == 'L') showLabels = !showLabels;
    if(key == 'n' || key == 'N') nightMode  = !nightMode;
    if(key == 'f' || key == 'F') { followPlanet = -1; targetZoom = 25; }
    if(key == 'g' || key == 'G') showLighting = !showLighting;

    if(key == '0') { followPlanet = 0; selectedPlanet = 0; targetZoom = 12; }
    if(key == '1') { followPlanet = 1; selectedPlanet = 1; targetZoom = 5;  }
    if(key == '2') { followPlanet = 2; selectedPlanet = 2; targetZoom = 5;  }
    if(key == '3') { followPlanet = 3; selectedPlanet = 3; targetZoom = 5;  }
    if(key == '4') { followPlanet = 4; selectedPlanet = 4; targetZoom = 5;  }
    if(key == '5') { followPlanet = 5; selectedPlanet = 5; targetZoom = 7;  }
    if(key == '6') { followPlanet = 6; selectedPlanet = 6; targetZoom = 7;  }
    if(key == '7') { followPlanet = 7; selectedPlanet = 7; targetZoom = 6;  }
    if(key == '8') { followPlanet = 8; selectedPlanet = 8; targetZoom = 6;  }

    glutPostRedisplay();
}

void specialKeys(int key, int x, int y)
{
    if(followPlanet != -1) return;

    float step = zoom * 0.08f;
    if(key == GLUT_KEY_LEFT)  camX -= step;
    if(key == GLUT_KEY_RIGHT) camX += step;
    if(key == GLUT_KEY_UP)    camY += step;
    if(key == GLUT_KEY_DOWN)  camY -= step;

    glutPostRedisplay();
}

// ==================== UPDATE ====================

void update()
{
    angle      += speed;
    selfRotate += 0.5f;
    cometAngle += speed * 0.55f;

    // Smooth zoom
    zoom += (targetZoom - zoom) * 0.05f;

    // Sun pulse
    if(sunPulseGrow) { sunPulse += 0.008f; if(sunPulse >= 0.45f) sunPulseGrow = false; }
    else             { sunPulse -= 0.008f; if(sunPulse <= 0.0f)  sunPulseGrow = true;  }

    // Star twinkle
    if(increaseStar) { starBrightness += 0.004f; if(starBrightness >= 1.0f) increaseStar = false; }
    else             { starBrightness -= 0.004f; if(starBrightness <= 0.3f) increaseStar = true;  }

    if(angle > 2 * 3.1416f) angle = 0.0f;

    glutPostRedisplay();
}

// ==================== MAIN ====================

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Solar System by NAIMUR RASHID");

    init();

    glutDisplayFunc(Draw);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutIdleFunc(update);

    lastTime = glutGet(GLUT_ELAPSED_TIME);

    glutMainLoop();
    return 0;
}
