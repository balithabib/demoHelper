  /*!\file earth.c
 *
 * \brief Terre en GL / GL4Dummies + Texture + Lumière positionnelle +
 * Phong + Spéculaire
 * \author Farès BELHADJ, amsi@ai.univ-paris8.fr
 * \date April 15 2016 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <GL4D/gl4du.h>
#include <GL4D/gl4df.h>
#include <GL4D/gl4duw_SDL2.h>
#include <SDL_image.h>
#include "audioHelper.h"
#include <fftw3.h>
/* Dans earth.c */
static void earth_init(void);  
static void earth_draw(void);
static void earth_quit(void);

/*!\brief identifiant du programme GLSL */
static GLuint _pId = 0;
/*!\brief position de la lumière relativement à la sphère éclairée */
static GLfloat _lumPos0[4] = {1500.0, 20.0, 30.0, 1.0};
/*!\brief tableau des identifiants de texture à charger */
static GLuint _tId[35] = {0};
//static GLfloat _posP[17] = {2.0, 0.0, 0.0, 2.0, -2.0, 0.0, 0.0, -2.0, 
//                            1.46, 1.46, -1.46, 1.46, -1.46, -1.46, 1.46, -1.46,};
static GLfloat _posP[30] = {2.0, 0.0, 1.8, 0.83, 1.33, 1.5, 0.63, 1.86, -0.2, 1.96, -0.96, 1.7, -1.56, 1.16, -1.93, 0.45, 
                            -1.96, -0.43, -1.63, -1.2, -1, -1.73, -0.2, -1.96, 0.63, -1.90, 1.33, -1.43, 1.80, -0.8};
/*!\brief pour les deux astres : terre et lune */
static GLuint _sphere = {0};


static void earth_init(void) {
  int i;
  SDL_Surface * t;
  int vp[4], w, h;
  static char * files[] = {"images/land_ocean_ice_2048.png", "images/land_ocean_ice_2048_glossmap.png", "images/Ballon.jpg",
          "images/Europe/Allemagne.jpg", "images/Europe/France.jpg", "images/Europe/Royaume-Uni.jpg", "images/Europe/Belgique.jpg", 
          "images/Europe/Croatie.jpg", "images/Europe/Danemark.jpg", "images/Europe/Islande.jpg", "images/Europe/Pologne.jpg", 
          "images/Europe/Russie.jpg", "images/Europe/Serbie.jpg", "images/Europe/Suede.jpg", "images/Europe/Suisse .jpg",
          "images/Drapeaux/Arabie saoudite.jpg", "images/Drapeaux/Australie.jpg", "images/Drapeaux/Coree du Sud.jpg", "images/Drapeaux/Iran.jpg", "images/Drapeaux/Japon.jpg", 
          "images/Drapeaux/Costa Rica.jpg", "images/Drapeaux/Mexique.jpg", "images/Drapeaux/Panama.jpg", 
          "images/Drapeaux/Egypte.jpg", "images/Drapeaux/Maroc.jpg", "images/Drapeaux/Nigeria.jpg", "images/Drapeaux/Senegal.jpg", "images/Drapeaux/Tunisie.jpg",
          "images/Drapeaux/Argentine.jpg", "images/Drapeaux/Bresil.jpg", "images/Drapeaux/Colombie.jpg","images/Drapeaux/Uruguay.jpg", "images/Drapeaux/Perou.jpg",  "images/Europe/Espagne.jpg", "images/Europe/Portugal.jpg"};

  _pId  = gl4duCreateProgram("<vs>shaders/earth.vs", "<fs>shaders/earth.fs", NULL);
  glGetIntegerv(GL_VIEWPORT, vp);
  w = vp[2] - vp[0];
  h = vp[3] - vp[1];
  gl4duGenMatrix(GL_FLOAT, "modelViewMatrix");
  gl4duGenMatrix(GL_FLOAT, "projectionMatrix");
  gl4duBindMatrix("projectionMatrix");
  gl4duLoadIdentityf();
  gl4duFrustumf(-0.5, 0.5, -0.5 * h / w, 0.5 * h / w, 1.0, 1000.0);
  gl4duBindMatrix("modelViewMatrix");
  glGenTextures(35, _tId);
  for(i = 0; i < 35; i++) {
    glBindTexture(GL_TEXTURE_2D, _tId[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    if( (t = IMG_Load(files[i])) != NULL ) {
#ifdef __APPLE__
      int mode = t->format->BytesPerPixel == 4 ? GL_BGRA : GL_BGR;
#else
      int mode = t->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;
#endif       
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, mode, GL_UNSIGNED_BYTE, t->pixels);
      SDL_FreeSurface(t);
    } else {
      fprintf(stderr, "can't open file %s : %s\n", files[i], SDL_GetError());
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
  }
  _sphere = gl4dgGenSpheref(30, 30);
}

static void earth_draw(void) {
  int i,j;
  static GLfloat a0 = 0.0;
  static Uint32 t0 = 0, t;

  GLfloat dt = 0.0, lumPos[4], *mat;
  dt = ((t = SDL_GetTicks()) - t0) / 1000.0;
  t0 = t;

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.1f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  gl4duBindMatrix("modelViewMatrix");
  gl4duLoadIdentityf();
  gl4duTranslatef(0, 0, -14);
  mat = gl4duGetMatrixData();
  MMAT4XVEC4(lumPos, mat, _lumPos0);

  glUseProgram(_pId);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _tId[0]);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, _tId[1]);
  glUniform1i(glGetUniformLocation(_pId, "tex0"), 0);
  glUniform1i(glGetUniformLocation(_pId, "tex1"), 1);
  //glUniform4fv(glGetUniformLocation(_pId, "lumPos"), 1, lumPos);
  glUniform1i(glGetUniformLocation(_pId, "specular"), 1);
  /* envoi de toutes les matrices stockées par GL4D */
  gl4duPushMatrix(); {
    gl4duRotatef(a0, 0, 1, 0);
    gl4duSendMatrices();
  } gl4duPopMatrix();
  gl4dgDraw(_sphere);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, 0);

  glActiveTexture(GL_TEXTURE0);
  for(i = 2, j = 0; i <= 16; i++, j+=2){
    glBindTexture(GL_TEXTURE_2D, _tId[i]);
    gl4duPushMatrix(); {
      gl4duRotatef(a0 - 220 / 28, 1, 0, 0);
      gl4duTranslatef(_posP[j], _posP[j+1], 0);  
      gl4duScalef(0.2, 0.2, 0.2);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    glUniform1i(glGetUniformLocation(_pId, "specular"), 0);
    gl4dgDraw(_sphere);
  }
  for(i = 17, j = 0; i < 32; i++, j+=2){
    glBindTexture(GL_TEXTURE_2D, _tId[i]);
    gl4duPushMatrix(); {
      gl4duRotatef(a0 - 220 / 28, 0, 0, 1);
      gl4duScalef(1.3, 1.3, 1.3);
      gl4duTranslatef(_posP[j], _posP[j+1], 1);  
      gl4duScalef(0.2, 0.2, 0.2);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    glUniform1i(glGetUniformLocation(_pId, "specular"), 0);
    gl4dgDraw(_sphere);
  }
    glBindTexture(GL_TEXTURE_2D, _tId[33]);
    gl4duPushMatrix(); {
      gl4duRotatef(a0 - 220 / 28, 1, 0, 0);
      gl4duScalef(1.3, 1.3, 1.3);
      gl4duTranslatef(4, 0, 0);  
      gl4duScalef(0.2, 0.2, 0.2);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    glUniform1i(glGetUniformLocation(_pId, "specular"), 0);
    gl4dgDraw(_sphere);

    glBindTexture(GL_TEXTURE_2D, _tId[34]);
    gl4duPushMatrix(); {
      gl4duRotatef(a0 - 220 / 28, 1, 0, 0);
      gl4duScalef(1.3, 1.3, 1.3);
      gl4duTranslatef(-4, 0, 0);  
      gl4duScalef(0.2, 0.2, 0.2);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    glUniform1i(glGetUniformLocation(_pId, "specular"), 0);
    gl4dgDraw(_sphere);

// Les ballons
    glBindTexture(GL_TEXTURE_2D, _tId[2]);
    gl4duPushMatrix(); {
      gl4duTranslatef(6, 3, 0);  
      gl4duRotatef(a0 - 220 / 28, 1, 0, 0);
      gl4duScalef(0.3, 0.3, 0.3);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    glUniform1i(glGetUniformLocation(_pId, "specular"), 0);
    gl4dgDraw(_sphere);

    gl4duPushMatrix(); {
      gl4duTranslatef(-6, 3, 0);  
      gl4duRotatef(a0 - 220 / 28, 1, 0, 0);
      gl4duScalef(0.3, 0.3, 0.3);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    glUniform1i(glGetUniformLocation(_pId, "specular"), 0);
    gl4dgDraw(_sphere);

    gl4duPushMatrix(); {
      gl4duTranslatef(-6, -3, 0);  
      gl4duRotatef(a0 - 220 / 28, 1, 0, 0);
      gl4duScalef(0.3, 0.3, 0.3);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    glUniform1i(glGetUniformLocation(_pId, "specular"), 0);
    gl4dgDraw(_sphere);

    gl4duPushMatrix(); {
      gl4duTranslatef(6, -3, 0);  
      gl4duRotatef(a0 - 220 / 28, 1, 0, 0);
      gl4duScalef(0.3, 0.3, 0.3);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    glUniform1i(glGetUniformLocation(_pId, "specular"), 0);
    gl4dgDraw(_sphere);

  a0 += 360 * dt / (6.0);
  gl4dfBlur(0, 0, 5, 1, 0, GL_FALSE);
  gl4dfSobelSetMixMode(GL4DF_SOBEL_MIX_MULT);
  gl4dfSobel(0, 0, GL_FALSE);
}

/*!\brief appelée au moment de sortir du programme (atexit), libère les éléments utilisés */
static void earth_quit(void) {
  for(int i = 0; i < 35 ; i++){
    if(_tId[i]) {
      _tId[i] = 0;
    }
  }
  glDeleteTextures(sizeof _tId / sizeof *_tId, _tId);
  gl4duClean(GL4DU_ALL);
}

void animation_earth(int state) {
  switch(state) {
  case GL4DH_INIT:
    earth_init();
    return;
  case GL4DH_FREE:
    earth_quit();
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    return;
  default: /* GL4DH_DRAW */
    earth_draw();
    return;
  }
}
