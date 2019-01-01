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
#include <SDL_image.h>
#include "audioHelper.h"


/*!\brief identifiant du programme GLSL */
static GLuint _pId = 0;
/*!\brief tableau des identifiants de texture à charger */
static GLuint _tId[6] = {0};//3
/*!\brief pour les deux astres : terre et lune */
static GLuint _cube = {0};

/* Dans n.c */
static void n_init(void);
static void n_draw(void);


static void n_init(void) {
  int i;
  SDL_Surface * t;
  int vp[4], w, h;
  static char * files[] = {"images/n.jpg", "images/argent.jpg","images/gold.jpg"};
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
  glGenTextures(3, _tId);
  for(i = 0; i < 3; i++) {
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
  _cube = gl4dgGenSpheref(30, 30);
}



static void n_draw(void) {
  static GLfloat a0 = 0.0;
  static Uint32 t0 = 0, t;
  GLfloat dt = 0.0;
  dt = ((t = SDL_GetTicks()) - t0) / 50.0;
  t0 = t;

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.1f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  gl4duBindMatrix("modelViewMatrix");
  gl4duLoadIdentityf();
  gl4duTranslatef(0, 0, -14);
  glUseProgram(_pId);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _tId[0]);
  glUniform1i(glGetUniformLocation(_pId, "tex0"), 0);
  glUniform1i(glGetUniformLocation(_pId, "tex1"), 1);

  gl4duPushMatrix(); {
    gl4duRotatef(a0, 0, 1, 0);
    gl4duSendMatrices(); 
  } gl4duPopMatrix();
 
  gl4dgDraw(_cube);
  
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, 0);
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _tId[1]);

  gl4duPushMatrix(); {
    gl4duRotatef(a0 / 28, 0, 1, 0);
    gl4duTranslatef(-5.1, 0, 0);  
    gl4duScalef(0.5, 0.5, 0.5);
    gl4duSendMatrices();
  } gl4duPopMatrix();
  glUniform1i(glGetUniformLocation(_pId, "specular"), 0);
  gl4dgDraw(_cube);

  gl4duPushMatrix(); {
    gl4duRotatef(a0 / 28, 0, 1, 0);
    gl4duTranslatef(5.1, 0, 0);  
    gl4duScalef(0.5, 0.5, 0.5);
    gl4duSendMatrices();
  } gl4duPopMatrix();
  glUniform1i(glGetUniformLocation(_pId, "specular"), 0);
  gl4dgDraw(_cube);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, 0);
    
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _tId[2]);

  gl4duPushMatrix(); {
    gl4duRotatef(a0 / 28, 1, 0, 0);
    gl4duTranslatef(0, 3.5, 0);  
    gl4duScalef(0.5, 0.5, 0.5);
    gl4duSendMatrices();
  } gl4duPopMatrix();
  glUniform1i(glGetUniformLocation(_pId, "specular"), 0);
  gl4dgDraw(_cube);
   
  gl4duPushMatrix(); {
    gl4duRotatef(a0 / 28, 1, 0, 0);
    gl4duTranslatef(0, -3.5, 0);  
    gl4duScalef(0.5, 0.5, 0.5);
    gl4duSendMatrices();
  } gl4duPopMatrix();
  glUniform1i(glGetUniformLocation(_pId, "specular"), 0);
  gl4dgDraw(_cube);

  gl4duPushMatrix(); {
    gl4duRotatef(a0 / 28, -2, 2 , 1/2);
    gl4duTranslatef(0,0,-3);
    gl4duScalef(0.5,0.5, 0.5);
    gl4duSendMatrices();
  } gl4duPopMatrix();
  glUniform1i(glGetUniformLocation(_pId, "specular"), 0);
  gl4dgDraw(_cube);
  
  gl4duPushMatrix(); {
    gl4duRotatef(a0 / 28, -2, 2 , 1/2);
    gl4duTranslatef(0,0,3);
    gl4duScalef(0.5,0.5, 0.5);
    gl4duSendMatrices();
  } gl4duPopMatrix();
  glUniform1i(glGetUniformLocation(_pId, "specular"), 0);
  gl4dgDraw(_cube);

  gl4duPushMatrix(); {
    gl4duRotatef(a0 / 28, -1, -2.5 , 1/2);
    gl4duTranslatef(0,0,-4);
    gl4duScalef(0.5,0.5, 0.5);
    gl4duSendMatrices();
  } gl4duPopMatrix();
  glUniform1i(glGetUniformLocation(_pId, "specular"), 0);
  gl4dgDraw(_cube);
  
  gl4duPushMatrix(); {
    gl4duRotatef(a0 / 28, -1, -2.5 , 1/2);
    gl4duTranslatef(0,0,4);   
    gl4duScalef(0.5,0.5, 0.5);
    gl4duSendMatrices();
  } gl4duPopMatrix();
  glUniform1i(glGetUniformLocation(_pId, "specular"), 0);
  gl4dgDraw(_cube);


  a0 += 2400.0 * dt / (5.0);
  
}



void animation_n(int state) {
  switch(state) {
  case GL4DH_INIT:
    n_init();
    return;
  case GL4DH_FREE:   
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    return;
  default: /* GL4DH_DRAW */
    n_draw();
    return;
  }
}
