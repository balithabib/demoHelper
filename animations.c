/*!\file animations.c
 *
 * \brief Votre espace de liberté : c'est ici que vous pouvez ajouter
 * vos fonctions de transition et d'animation avant de les faire
 * référencées dans le tableau _animations du fichier \ref window.c
 *
 * Des squelettes d'animations et de transitions sont fournis pour
 * comprendre le fonctionnement de la bibliothèque. En bonus des
 * exemples dont un fondu en GLSL.
 *
 * \author Farès BELHADJ, amsi@ai.univ-paris8.fr
 * \date May 05, 2014
 */
#include <GL4D/gl4dh.h>
#include "audioHelper.h"
#include "animations.h"
#include <assert.h>
#include <stdlib.h>
#include <GL4D/gl4dg.h>
#include <SDL.h>
#include <SDL_image.h>
#include <fftw3.h>

/*!\brief identifiant de la géométrie QUAD GL4Dummies */
static GLuint _quadId = 0;

static void loadTexture(GLuint id, const char * filename) {
  SDL_Surface * t;
  glBindTexture(GL_TEXTURE_2D, id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  if( (t = IMG_Load(filename)) != NULL ) {
#ifdef __APPLE__
    int mode = t->format->BytesPerPixel == 4 ? GL_BGRA : GL_BGR;
#else
    int mode = t->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;
#endif       
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, mode, GL_UNSIGNED_BYTE, t->pixels);
    SDL_FreeSurface(t);
  } else {
    fprintf(stderr, "can't open file %s : %s\n", filename, SDL_GetError());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  }
}

//====================================================================================================
void transition_fondu(void (* a0)(int), void (* a1)(int), Uint32 t, Uint32 et, int state) {
  /* INITIALISEZ VOS VARIABLES */
  int vp[4], i;
  GLint tId;
  static GLuint tex[2], pId;
  switch(state) {
  case GL4DH_INIT:
    /* INITIALISEZ VOTRE TRANSITION (SES VARIABLES <STATIC>s) */
    glGetIntegerv(GL_VIEWPORT, vp);
    glGenTextures(2, tex);
    for(i = 0; i < 2; i++) {
      glBindTexture(GL_TEXTURE_2D, tex[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vp[2], vp[3], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
    pId = gl4duCreateProgram("<vs>shaders/basic.vs", "<fs>shaders/mix.fs", NULL);
    return;
  case GL4DH_FREE:
    /* LIBERER LA MEMOIRE UTILISEE PAR LES <STATIC>s */
    if(tex[0]) {
      glDeleteTextures(2, tex);
      tex[0] = tex[1] = 0;
    }
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    /* METTRE A JOUR LES DEUX ANIMATIONS EN FONCTION DU SON */
    if(a0) a0(state);
    if(a1) a1(state);
    return;
  default: /* GL4DH_DRAW */
    /* RECUPERER L'ID DE LA DERNIERE TEXTURE ATTACHEE AU FRAMEBUFFER */
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &tId);
    /* JOUER LES DEUX ANIMATIONS */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,  tex[0],  0);
    if(a0) a0(state);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,  tex[1],  0);
    if(a1) a1(state);
    /* MIXER LES DEUX ANIMATIONS */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,  tId,  0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glUseProgram(pId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    if(et / (GLfloat)t > 1) {
      fprintf(stderr, "%d-%d -- %f\n", et, t, et / (GLfloat)t);
      exit(0);
    }
    glUniform1f(glGetUniformLocation(pId, "dt"), et / (GLfloat)t);
    glUniform1i(glGetUniformLocation(pId, "tex0"), 0);
    glUniform1i(glGetUniformLocation(pId, "tex1"), 1);
    gl4dgDraw(_quadId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return;
  }
}
//====================================================================================================
void fondui(void (* a0)(int), void (* a1)(int), Uint32 t, Uint32 et, int state) {
  /* INITIALISEZ VOS VARIABLES */
  int vp[4], i;
  GLint tId;
  static GLuint tex[3], pId;
  switch(state) {
  case GL4DH_INIT:
    /* INITIALISEZ VOTRE TRANSITION (SES VARIABLES <STATIC>s) */
    glGetIntegerv(GL_VIEWPORT, vp);
    glGenTextures(3, tex);
    for(i = 0; i < 3; i++) {
      glBindTexture(GL_TEXTURE_2D, tex[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vp[2], vp[3], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
    loadTexture(tex[2], "images/fondui.jpg");
    pId = gl4duCreateProgram("<vs>shaders/basic.vs", "<fs>shaders/mixi.fs", NULL);
    return;
  case GL4DH_FREE:
    /* LIBERER LA MEMOIRE UTILISEE PAR LES <STATIC>s */
    if(tex[0]) {
      glDeleteTextures(3, tex);
      tex[0] = tex[1] = 0;
    }
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    /* METTRE A JOUR LES DEUX ANIMATIONS EN FONCTION DU SON */
    if(a0) a0(state);
    if(a1) a1(state);
    return;
  default: /* GL4DH_DRAW */
    /* RECUPERER L'ID DE LA DERNIERE TEXTURE ATTACHEE AU FRAMEBUFFER */
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &tId);
    /* JOUER LES DEUX ANIMATIONS */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,  tex[0],  0);
    if(a0) a0(state);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,  tex[1],  0);
    if(a1) a1(state);
    /* MIXER LES DEUX ANIMATIONS */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,  tId,  0);
    glDisable(GL_DEPTH);
    glUseProgram(pId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex[2]);
    if(et / (GLfloat)t > 1) {
      fprintf(stderr, "%d-%d -- %f\n", et, t, et / (GLfloat)t);
      exit(0);
    }
    glUniform1f(glGetUniformLocation(pId, "dt"), et / (GLfloat)t);
    glUniform1i(glGetUniformLocation(pId, "tex0"), 0);
    glUniform1i(glGetUniformLocation(pId, "tex1"), 1);
    glUniform1i(glGetUniformLocation(pId, "tex2"), 2);
    gl4dgDraw(_quadId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return;
  }
}

void bidon00(int state) {
  /* INITIALISEZ VOS VARIABLES */
  int l, i;
  Uint16 * s;
  static GLfloat c[4] = {0, 0, 0, 0};
  switch(state) {
  case GL4DH_INIT:
    /* INITIALISEZ VOTRE ANIMATION (SES VARIABLES <STATIC>s) */
    return;
  case GL4DH_FREE:
    /* LIBERER LA MEMOIRE UTILISEE PAR LES <STATIC>s */
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    /* METTRE A JOUR VOTRE ANIMATION EN FONCTION DU SON */
    s = (Uint16 *)ahGetAudioStream();
    l = ahGetAudioStreamLength();
    if(l >= 8)
      for(i = 0; i < 4; i++)
  c[i] = s[i] / ((1 << 16) - 1.0);
    return;
  default: /* GL4DH_DRAW */
    /* JOUER L'ANIMATION */
    glClearColor(c[0], c[1], c[2], c[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    return;
  }
}

void bidon01(int state) {
  /* INITIALISEZ VOS VARIABLES */
  static GLuint tId = 0, pId = 0;
  switch(state) {
  case GL4DH_INIT:
    /* INITIALISEZ VOTRE ANIMATION (SES VARIABLES <STATIC>s) */
    glGenTextures(1, &tId);
    loadTexture(tId, "images/01.jpg");
    pId = gl4duCreateProgram("<vs>shaders/basic.vs", "<fs>shaders/basic.fs", NULL);
    return;
  case GL4DH_FREE:
    /* LIBERER LA MEMOIRE UTILISEE PAR LES <STATIC>s */
    if(tId) {
      glDeleteTextures(1, &tId);
      tId = 0;
    }
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    /* METTRE A JOUR VOTRE ANIMATION EN FONCTION DU SON */
    return;
  default: /* GL4DH_DRAW */
    /* JOUER L'ANIMATION */
    glDisable(GL_DEPTH);
    glUseProgram(pId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tId);
    glUniform1i(glGetUniformLocation(pId, "inv"), 1);
    glUniform1i(glGetUniformLocation(pId, "myTexture"), 0);
    gl4dgDraw(_quadId);
    return;
  }
}

void bidon02(int state) {
  /* INITIALISEZ VOS VARIABLES */
  static GLuint tId = 0, pId = 0;
  switch(state) {
  case GL4DH_INIT:
    /* INITIALISEZ VOTRE ANIMATION (SES VARIABLES <STATIC>s) */
    glGenTextures(1, &tId);
    loadTexture(tId, "images/02.jpg");
    pId = gl4duCreateProgram("<vs>shaders/basic.vs", "<fs>shaders/basic.fs", NULL);
    return;
  case GL4DH_FREE:
    /* LIBERER LA MEMOIRE UTILISEE PAR LES <STATIC>s */
    if(tId) {
      glDeleteTextures(1, &tId);
      tId = 0;
    }
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    /* METTRE A JOUR VOTRE ANIMATION EN FONCTION DU SON */
    return;
  default: /* GL4DH_DRAW */
    /* JOUER L'ANIMATION */
    glDisable(GL_DEPTH);
    glUseProgram(pId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tId);
    glUniform1i(glGetUniformLocation(pId, "inv"), 1);
    glUniform1i(glGetUniformLocation(pId, "myTexture"), 0);
    gl4dgDraw(_quadId);
    return;
  }
}

void bidon03(int state) {
  /* INITIALISEZ VOS VARIABLES */
  static GLuint tId = 0, pId = 0;
  switch(state) {
  case GL4DH_INIT:
    /* INITIALISEZ VOTRE ANIMATION (SES VARIABLES <STATIC>s) */
    glGenTextures(1, &tId);
    loadTexture(tId, "images/03.jpg");
    pId = gl4duCreateProgram("<vs>shaders/basic.vs", "<fs>shaders/basic.fs", NULL);
    return;
  case GL4DH_FREE:
    /* LIBERER LA MEMOIRE UTILISEE PAR LES <STATIC>s */
    if(tId) {
      glDeleteTextures(1, &tId);
      tId = 0;
    }
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    /* METTRE A JOUR VOTRE ANIMATION EN FONCTION DU SON */
    return;
  default: /* GL4DH_DRAW */
    /* JOUER L'ANIMATION */
    glDisable(GL_DEPTH);
    glUseProgram(pId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tId);
    glUniform1i(glGetUniformLocation(pId, "inv"), 1);
    glUniform1i(glGetUniformLocation(pId, "myTexture"), 0);
    gl4dgDraw(_quadId);
    return;
  }
}

//====================================================================================================
void animation_flash(int state) {
  /* INITIALISEZ VOS VARIABLES */
  int l, i;
  Uint16 * s;
  static GLfloat c[4] = {0, 0, 0, 0};
  switch(state) {
  case GL4DH_INIT:
    /* INITIALISEZ VOTRE ANIMATION (SES VARIABLES <STATIC>s) */
    return;
  case GL4DH_FREE:
    /* LIBERER LA MEMOIRE UTILISEE PAR LES <STATIC>s */
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    /* METTRE A JOUR VOTRE ANIMATION EN FONCTION DU SON */
    s = (Uint16 *)ahGetAudioStream();
    l = ahGetAudioStreamLength();
    if(l >= 8)
      for(i = 0; i < 4; i++)
	c[i] = s[i] / ((1 << 16) - 1.0);
    return;
  default: /* GL4DH_DRAW */
    /* JOUER L'ANIMATION */
    glClearColor(c[0], c[1], c[2], c[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    return;
  }
}
//====================================================================================================
void animation_vide(int state) {
  switch(state) {
  case GL4DH_DRAW:
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return;
  default:
    return;
  }
}

//====================================================================================================
static void copyFromSurface(SDL_Surface * s, int x0, int y0) {
  Uint32 * p, coul, ocoul;
  Uint8 rmix, gmix, bmix;
  double f1, f2;
  int x, y, maxx, maxy;
  assert(s->format->BytesPerPixel == 4); /* pour simplifier, on ne gère que le format RGBA */
  p = s->pixels;
  maxx = gl4dpGetWidth();
  maxy =  gl4dpGetHeight();
  for(y = y0; y < maxy; y++) {
    for(x = x0; x < maxx; x++) {
      ocoul = gl4dpGetPixel(x, y);
      coul = p[(s->h - 1 - (y - y0)) * s->w + x - x0]; /* axe y à l'envers */
      f1 = ALPHA(coul) / 255.0; f2 = 1.0 - f1;
      rmix = f1 *   RED(coul) + f2 *   RED(ocoul);
      gmix = f1 * GREEN(coul) + f2 * GREEN(ocoul);
      bmix = f1 *  BLUE(coul) + f2 *  BLUE(ocoul);
      gl4dpSetColor(RGB(rmix, gmix, bmix));
      gl4dpPutPixel(x, y);
    }
  }
}
//====================================================================================================
void animation_1(int state) {
  /* INITIALISEZ VOS VARIABLES */
  int l, i;
  Sint16 * s;
  static Sint16 _hauteurs[1024];
  static GLuint s3;

  switch(state) {
  case GL4DH_INIT:
    /* INITIALISEZ VOTRE ANIMATION (SES VARIABLES <STATIC>s) */
    glViewport(0, 0, 600,1024);
    s3 = gl4dpInitScreen();
    return;
  case GL4DH_FREE:
    /* LIBERER LA MEMOIRE UTILISEE PAR LES <STATIC>s */
    gl4dpSetScreen(s3);
    gl4dpDeleteScreen();
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    /* METTRE A JOUR VOTRE ANIMATION EN FONCTION DU SON */
    s = (Sint16 *)ahGetAudioStream();
    l = ahGetAudioStreamLength();
    if(l >= 2 * 1024)
      for(i = 0; i < 1024; i++){
        _hauteurs[i] = (600/ 2) + (600 / 2 ) * s[i] / ((1 << 15) - 1.0);
      }
    return;
  default: /* GL4DH_DRAW */
    gl4dpSetScreen(s3);
    gl4dpClearScreen();

    for(i = 0; i < 1024; i+=2) {
      int x0, y0, y1;
      x0 = (i * (gl4dpGetHeight() - 1)) / (gl4dpGetWidth() - 1);
      y0 = _hauteurs[i];
      y1 =gl4dpGetHeight() - y0;
      gl4dpSetColor(RGB(0, 255, 0));
      gl4dpLine(x0, y0, x0, y1);
    }
    gl4dpUpdateScreen(NULL);
    return;
  }
}
//====================================================================================================                        
void animation_2(int state) {
  /* INITIALISEZ VOS VARIABLES */
  int l,i,j;
  Sint16 * s;
  static Sint16 _hauteurs[1024];
  static GLuint s1;
  /*!\brief données entrées/sorties pour la lib fftw */
  static fftw_complex * _in4fftw = NULL, * _out4fftw = NULL;
  /*!\brief donnée à précalculée utile à la lib fftw */
  static fftw_plan _plan4fftw = NULL;

  switch(state) {
  case GL4DH_INIT:
    _in4fftw   = fftw_malloc(1024 *  sizeof *_in4fftw);
    memset(_in4fftw, 0, 1024 *  sizeof *_in4fftw);
    assert(_in4fftw);
    _out4fftw  = fftw_malloc(1024 * sizeof *_out4fftw);
    assert(_out4fftw);
    _plan4fftw = fftw_plan_dft_1d(1024, _in4fftw, _out4fftw, FFTW_FORWARD, FFTW_ESTIMATE);
    assert(_plan4fftw);
    glViewport(0, 0, gl4dpGetHeight(), gl4dpGetWidth());
    /* INITIALISEZ VOTRE ANIMATION (SES VARIABLES <STATIC>s) */
    s1 = gl4dpInitScreen();
    return;
  case GL4DH_FREE:
    /* LIBERER LA MEMOIRE UTILISEE PAR LES <STATIC>s */
    gl4dpSetScreen(s1);
    gl4dpDeleteScreen();
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    /* METTRE A JOUR VOTRE ANIMATION EN FONCTION DU SON */
    s = (Sint16 *)ahGetAudioStream();
    l = ahGetAudioStreamLength();
    if(_plan4fftw) {
      l = MIN(l >> 1, 1024);
      Sint16 *d = (Sint16 *)s;
      for(i = 0; i < l; i++)
        _in4fftw[i][0] = d[i] / ((1 << 15) - 1.0);
      fftw_execute(_plan4fftw);
      for(i = 0; i < l >> 2; i++) {
        _hauteurs[4 * i] = (int)(sqrt(_out4fftw[i][0] * _out4fftw[i][0] + _out4fftw[i][1] * _out4fftw[i][1]) * exp(2.0 * i / (double)(l / 4.0)));
        for(j = 1; j < 4; j++)
          _hauteurs[4 * i + j] = MIN(_hauteurs[4 * i], 255);
      }
    }
    return;
  default: /* GL4DH_DRAW */
    gl4dpSetScreen(s1);
    gl4dpClearScreen();
    gl4dpSetColor(RGB(0,255, 0));
    for(i = 0; i < 1024; ++i) {
      int x0, y0;
      //gl4dpSetColor(RGB(0,255, 0));
      x0 = (i * (gl4dpGetHeight() - 1)) / (1024 - 1);
      y0 = 4 * _hauteurs[i]; 
      for (int j = 0;j <= 10 ; ++j){
        gl4dpPutPixel(x0, j);
      }
      for (int j = 0;j <= y0 ; ++j){
        gl4dpPutPixel(x0, j);
      }

      y0 = gl4dpGetHeight() - 4 * _hauteurs[i]; 
      for (int j = gl4dpGetHeight();j >= gl4dpGetHeight()-10 ; j--){
        gl4dpPutPixel(x0, j);
      }
      for (int j = y0;j <= gl4dpGetHeight() ; ++j){
        gl4dpPutPixel(x0, j);
      }
      //for (int j = (gl4dpGetHeight()/2) - 2 ;j <= (gl4dpGetHeight()/2)+2 ; ++j){
      //j = abs(_hauteurs[i]-(gl4dpGetHeight()/2));
      //gl4dpSetColor(RGB(rand()%255,rand()%255, rand()%255));
      //gl4dpPutPixel(i, j);
      
    }



    gl4dpUpdateScreen(NULL);  
    return;
  }
}
//====================================================================================================

void animation_3(int state) {
  /* INITIALISEZ VOS VARIABLES */
  int l, i;
  Sint16 * s;
  static Sint16 _hauteurs[1024];
  static GLuint s3;

  switch(state) {
  case GL4DH_INIT:
    /* INITIALISEZ VOTRE ANIMATION (SES VARIABLES <STATIC>s) */
    glViewport(0, 0, gl4dpGetHeight(), gl4dpGetWidth());
    s3 = gl4dpInitScreen();
    return;
  case GL4DH_FREE:
    /* LIBERER LA MEMOIRE UTILISEE PAR LES <STATIC>s */
    gl4dpSetScreen(s3);
    gl4dpDeleteScreen();
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    /* METTRE A JOUR VOTRE ANIMATION EN FONCTION DU SON */
    s = (Sint16 *)ahGetAudioStream();
    l = ahGetAudioStreamLength();
    if(l >= 2 * 1024)
      for(i = 0; i < 1024; i++){
        _hauteurs[i] = (600/ 2) + (600 / 2 ) * s[i] / ((1 << 15) - 1.0);
      }
    return;
  default: /* GL4DH_DRAW */
    gl4dpSetScreen(s3);
    gl4dpClearScreen(); 
    gl4dpSetColor(RGB(0,255, 0));
    for(i = 0; i < 1024 - 1; i=i+100) {
      int r = 3 * abs(_hauteurs[i] - _hauteurs[i+1]);     
      gl4dpCircle(i,(600 -1)/2,r);      
      for (int j = 0; j <= 20; j++){
        gl4dpPutPixel(i, _hauteurs[i] + j);
      }
    }
    gl4dpUpdateScreen(NULL);
    return;
  }
}
//====================================================================================================
void animation_4(int state) {
  /* INITIALISEZ VOS VARIABLES */
  int l, i;
  Sint16 * s;
  static Sint16 _hauteurs[1024];
  static GLuint s2;

  switch(state) {
  case GL4DH_INIT:
    /* INITIALISEZ VOTRE ANIMATION (SES VARIABLES <STATIC>s) */
    glViewport(0, 0, gl4dpGetHeight(), gl4dpGetWidth());
    s2 = gl4dpInitScreen();
    return;
  case GL4DH_FREE:
    /* LIBERER LA MEMOIRE UTILISEE PAR LES <STATIC>s */
    gl4dpSetScreen(s2);
    gl4dpDeleteScreen();
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    /* METTRE A JOUR VOTRE ANIMATION EN FONCTION DU SON */
    s = (Sint16 *)ahGetAudioStream();
    l = ahGetAudioStreamLength();
    if(l >= 2 * 1024)
      for(i = 0; i < 1024; i++){
        _hauteurs[i] = (600/ 2) + (600 / 2 ) * s[i] / ((1 << 15) - 1.0);
      }
    return;
  default: /* GL4DH_DRAW */
    gl4dpSetScreen(s2);
    gl4dpClearScreen(); 
    for(i = 0; i < 1024 - 1; i=i+100) {
      int r = 3 * abs(_hauteurs[i] - _hauteurs[i+1]);
      gl4dpSetColor(rand());
      for (int k = 0; k <=(1024 - 1); k = k + 102){
        gl4dpFilledCircle(k,(600 -1)/2,r);
      }
    }
    gl4dpUpdateScreen(NULL);
    return;
  }
}
//====================================================================================================
void animation_5(int state) {
  /* INITIALISEZ VOS VARIABLES */
  int l, i;
  Sint16 * s;
  GLint rect[4], tr, mr;
  static int r = 0, or = 0;
  static GLuint screen_id;
 static GLfloat rf = 0.0;
  switch(state) {
  case GL4DH_INIT:
    /* INITIALISEZ VOTRE ANIMATION (SES VARIABLES <STATIC>s) */
    screen_id = gl4dpInitScreen();
    gl4dpUpdateScreen(NULL);
    return;
  case GL4DH_FREE:
    /* LIBERER LA MEMOIRE UTILISEE PAR LES <STATIC>s */
    gl4dpSetScreen(screen_id);
    gl4dpDeleteScreen();
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    /* METTRE A JOUR VOTRE ANIMATION EN FONCTION DU SON */
    s = (Sint16 *)ahGetAudioStream();
    l = ahGetAudioStreamLength();
    for(i = 0, tr = 0; i < l >> 1; i++)
      tr += abs(s[i]);
    tr /= l >> 1;
    r = 300 + (tr >> 6);
    return;
  default: /* GL4DH_DRAW */
    /* JOUER L'ANIMATION */
    tr = r;
    mr = MAX(tr, or) + 1;
    gl4dpSetScreen(screen_id);
    rect[0] = MAX((gl4dpGetWidth()  >> 1) - mr, 0);
    rect[1] = MAX((gl4dpGetHeight() >> 1) - mr, 0);
    rect[2] = MIN(mr << 1,  gl4dpGetWidth());
    rect[3] = MIN(mr << 1, gl4dpGetHeight());

   Uint8 r = rand()&255, g = rand()&255, b = rand()&255;
    gl4dpSetColor(RGB(r, g, b));
   gl4dpRect(rect);
  
    rf += 0.05;
   gl4dpUpdateScreen(rect);
  
    return;

  }
}
//====================================================================================================
void animation_6(int state) {
  //INITIALISEZ VOS VARIABLES 
  static SDL_Surface * sprite = NULL;
  static GLuint screen_id = 0;
  switch(state) {
  case GL4DH_INIT: {
    // INITIALISEZ VOTRE ANIMATION (SES VARIABLES <STATIC>s) 
    SDL_Surface * s = IMG_Load("images/g.jpg");
    sprite = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_FreeSurface(s);
    screen_id = gl4dpInitScreen();
    gl4dpClearScreenWith(0xFFFFFFFF);
    return;
  }
  case GL4DH_FREE:
    // LIBERER LA MEMOIRE UTILISEE PAR LES <STATIC>s 
    gl4dpSetScreen(screen_id);
    gl4dpDeleteScreen();
    if(sprite) {
      SDL_FreeSurface(sprite);
      sprite = NULL;
    }
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    //METTRE A JOUR VOTRE ANIMATION EN FONCTION DU SON 
    return;
  default: 
    gl4dpSetScreen(screen_id);
    copyFromSurface(sprite, 0,0);
    gl4dpUpdateScreen(NULL);
    return;
  }
}  
//====================================================================================================
void animation_7(int state) {
  /* INITIALISEZ VOS VARIABLES */
  int l, i;
  Sint16 * s;
  GLint rect[4], tr, mr;
  static int r = 0, or = 0;
  static GLuint screen_id;
  switch(state) {
  case GL4DH_INIT:
    /* INITIALISEZ VOTRE ANIMATION (SES VARIABLES <STATIC>s) */
    screen_id = gl4dpInitScreen();
    gl4dpUpdateScreen(NULL);
    return;
  case GL4DH_FREE:
    /* LIBERER LA MEMOIRE UTILISEE PAR LES <STATIC>s */
    gl4dpSetScreen(screen_id);
    gl4dpDeleteScreen();
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    /* METTRE A JOUR VOTRE ANIMATION EN FONCTION DU SON */
    s = (Sint16 *)ahGetAudioStream();
    l = ahGetAudioStreamLength();
    for(i = 0, tr = 0; i < l >> 1; i++)
      tr += abs(s[i]);
    tr /= l >> 1;
    r = 100 + (tr >> 6);
    return;
  default: /* GL4DH_DRAW */
    /* JOUER L'ANIMATION */
    tr = r;
    mr = MAX(tr, or) + 1;
    gl4dpSetScreen(screen_id);
    rect[0] = MAX((gl4dpGetWidth()  >> 1) - mr, 0);
    rect[1] = MAX((gl4dpGetHeight() >> 1) - mr, 0);
    rect[2] = MIN(mr << 1,  gl4dpGetWidth() - rect[0]);
    rect[3] = MIN(mr << 1, gl4dpGetHeight() - rect[1]);
    gl4dpSetColor(RGB(0, 0, 0));
    gl4dpRect(rect);
    gl4dpSetColor(RGB(255, 255, 255));
    gl4dpFilledCircle(gl4dpGetWidth() >> 1, gl4dpGetHeight() >> 1, tr);
    gl4dpUpdateScreen(rect);
    or = tr;
    return;
  }
}
//====================================================================================================
void animationsInit(void) {
  if(!_quadId)
    _quadId = gl4dgGenQuadf();
}
