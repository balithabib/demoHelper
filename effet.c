/*!\file window.c
 * \brief effet 2D en shader
 * \author Farès BELHADJ, amsi@ai.univ-paris8.fr
 * \date May 14 2018 */
#include <stdio.h>
#include <GL4D/gl4du.h>
#include <GL4D/gl4df.h>
#include <GL4D/gl4duw_SDL2.h>
#include <SDL_image.h>
#include "audioHelper.h"

/*!\brief identifiant du programme GLSL */
static GLuint _pId = 0;
/*!\brief identifiants ... */
static GLuint _quad = 0, _tex = 0;

/* Dans effet.c */
static void init_effet(void);
static void draw_effet(void);
static void quit_effet(void);
static void loadTexture(GLuint id, const char * filename);

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

/*!\brief initialise les paramètres OpenGL et les données */
static void init_effet(void) {
  _pId  = gl4duCreateProgram("<vs>shaders/effet.vs", "<fs>shaders/effet.fs", NULL);
  _quad = gl4dgGenQuadf();
  /* génération d'un identifiant de texture */
  glGenTextures(1, &_tex);
  /* chargement et transfert d'image dans une texture OpenGL */
  loadTexture(_tex, "images/c.jpg");
  glEnable(GL_TEXTURE_2D);
}

/*!\brief dessine dans le contexte OpenGL actif. */
static void draw_effet(void) {
  double temps = gl4dGetElapsedTime();
  glUseProgram(_pId);
  glActiveTexture(GL_TEXTURE0);
  glUniform1i(glGetUniformLocation(_pId, "tex"), 0);
  glUniform1f(glGetUniformLocation(_pId, "temps"), (GLfloat)temps);
  glBindTexture(GL_TEXTURE_2D, _tex);
  gl4dgDraw(_quad);
}
/*!\brief appelée au moment de sortir du programme (atexit), libère les éléments utilisés */
static void quit_effet(void) {
  /* suppression d'identifiant de texture */
  glDeleteTextures(1, &_tex);
  //gl4duClean(GL4DU_ALL);
}

void animation_effet(int state) {
  /* INITIALISEZ VOS VARIABLES */
  switch(state) {
  case GL4DH_INIT:
    /* INITIALISEZ VOTRE ANIMATION (SES VARIABLES <STATIC>s) */
    init_effet();
    return;
  case GL4DH_FREE:
    /* LIBERER LA MEMOIRE UTILISEE PAR LES <STATIC>s */
    quit_effet();
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    /* METTRE A JOUR VOTRE ANIMATION EN FONCTION DU SON */
    return;
  default: /* GL4DH_DRAW */
    draw_effet();
    return;
  }
}
