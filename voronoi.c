/*!\file window.c
 *
 * \brief Utilisation de la SDL2, SDL2_Mixer et d'OpenGL 3+
 * \author Farès BELHADJ, amsi@ai.univ-paris8.fr
 * \date February 03 2014
 */

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <GL4D/gl4du.h>
#include <SDL_mixer.h>
#include "audioHelper.h"

/* Dans voronoi.c */
static void init_voronoi(void);
static void draw_voronoi(void);
static void quit_voronoi(void);
static void audio_voronoi(void);


/*!\brief dimensions de la fenêtre */
static GLfloat _dim[] = {800, 600};
/*!\brief nombre de cellules de voronoi */
static int _nbSites = 40;
/*!\brief coordonnées des cellules de voronoi */
static GLfloat *_coords = NULL;
/*!\brief couleurs des cellules de voronoi */
static GLubyte *_colors = NULL;
/*!\brief pointeur vers la (future) fenêtre SDL */
static SDL_Window * _win = NULL;
/*!\brief pointeur vers le (futur) contexte OpenGL */
static SDL_GLContext _oglContext = NULL;
/*!\brief identifiant du (futur) vertex array object */
static GLuint _vao = 0;
/*!\brief identifiant du (futur) buffer de data */
static GLuint _buffer = 0;
/*!\brief identifiant du (futur) GLSL program */
static GLuint _pId = 0;
/*!\brief identifiant des textures utilisées pour stocker les
 * coordonnées et les couleurs des sites */
static GLuint _t1Id = 0, _t2Id = 0;
/*!\brief pointeur vers la musique chargée par SDL_Mixer */
static Mix_Music * _mmusic = NULL;
/*!\brief flag permettant de savoir si les cellules ont été mises à jour au sein du processus parallèle utilisant le buffer de son */
static int _updatedCoords = 0;
/*!\brief Cette fonction est appelée quand l'audio est joué et met
 * dans \a stream les données audio de longueur \a len. Elle met à
 * jour les y des cellules de voronoi en fonction du son.
 */
static void audio_voronoi(void) {
  int i, id, l;
  GLfloat d;
  Sint16 * s;
  s = (Sint16 *)ahGetAudioStream();
  l = ahGetAudioStreamLength();
  if(!_coords) return;
  for(i = 0; i < _nbSites; i++) {
    if((id = _coords[3 * i + 0] * _dim[0]) < l) {
      d = s[id] / (256.0 * 256.0 / 2);
      _coords[3 * i + 1] = _coords[3 * i + 2] +  powf(d, 2.);
    }
  }
  _updatedCoords = 1;
}

static void init_voronoi(void) {
  int i;
  _pId = gl4duCreateProgram("<vs>shaders/voronoi.vs", "<fs>shaders/voronoi.fs", NULL);
  GLfloat data[] = {
    /* 4 coordonnées de sommets */
    -1.f, -1.f, 1.f, -1.f,
    -1.f,  1.f, 1.f,  1.f  
  };
  glClearColor(0.0f, 0.4f, 0.0f, 0.0f);
  //resizeGL(win);
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);

  glGenBuffers(1, &_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, _buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof data, data, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  /* initialisation et envoi des coordonnées et des couleurs des
     cellules de voronoi */

  glGenTextures(1, &_t1Id);
  glBindTexture(GL_TEXTURE_1D, _t1Id);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);

  _coords = malloc(3 * _nbSites * sizeof *_coords);
  assert(_coords);
  _colors = malloc(3 * _nbSites * sizeof *_colors);
  assert(_colors);
  for(i = 0; i < _nbSites; i++) {
    _coords[3 * i + 0] = rand() / (RAND_MAX + 1.0);
    _coords[3 * i + 1] = _coords[3 * i + 2] = 0.2 + (rand() / (RAND_MAX + 1.0)) / 1.5;
    _colors[3 * i + 0] = (int)(256.0 * rand() / (RAND_MAX + 1.0));
    _colors[3 * i + 1] = (int)(256.0 * rand() / (RAND_MAX + 1.0));
    _colors[3 * i + 2] = (int)(256.0 * rand() / (RAND_MAX + 1.0));
  }
  glBindTexture(GL_TEXTURE_1D, _t1Id);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, _nbSites, 0, GL_RGB, GL_FLOAT, _coords);

  glGenTextures(1, &_t2Id);
  glBindTexture(GL_TEXTURE_1D, _t2Id);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, _nbSites, 0, GL_RGB, GL_UNSIGNED_BYTE, _colors);

  glBindTexture(GL_TEXTURE_1D, 0);
}

/*!\brief Cette fonction paramétrela vue (viewPort) OpenGL en fonction
 * des dimensions de la fenêtre SDL pointée par \a win.
 *
 * \param win le pointeur vers la fenêtre SDL pour laquelle nous avons
 * attaché le contexte OpenGL.
 */
/*void resizeGL(SDL_Window * win) {
  int w, h;
  SDL_GetWindowSize(win, &w, &h);
  glViewport(0, 0, w, h);
}*/

/*!\brief Cette fonction dessine dans le contexte OpenGL actif.
 */
static void draw_voronoi(void) {
  if(_updatedCoords) { /* mise à jours des coordonnées des cellules si
			  mises à jour  */
  glBindTexture(GL_TEXTURE_1D, _t1Id);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, _nbSites, 0, GL_RGB, GL_FLOAT, _coords);
    _updatedCoords = 0;
  }
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(_pId);
  glBindVertexArray(_vao);
  glEnableVertexAttribArray(0);
  glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_1D, _t1Id);
  glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_1D, _t2Id);
  glUniform1i(glGetUniformLocation(_pId, "nbSites"), _nbSites);
  glUniform1i(glGetUniformLocation(_pId, "siteCoords"), 0);
  glUniform1i(glGetUniformLocation(_pId, "siteColors"), 1);
  glUniform2fv(glGetUniformLocation(_pId, "dim"), 1, _dim);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray(0);
  glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_1D, 0);
  glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_1D, 0);
  glUseProgram(0);
}
/*!\brief Cette fonction est appelée au moment de sortir du programme
 *  (atexit), elle libère les données audio, la fenêtre SDL \ref _win
 *  et le contexte OpenGL \ref _oglContext.
 */
static void quit_voronoi(void) {
  if(_mmusic) {
    while(Mix_PlayingMusic() && Mix_FadeOutMusic(2000)) {
      draw_voronoi();
      SDL_GL_SwapWindow(_win);
      SDL_Delay(1);
    }
    Mix_FreeMusic(_mmusic);
    _mmusic = NULL;
  }
  Mix_CloseAudio();
  Mix_Quit();
  if(_vao)
    glDeleteVertexArrays(1, &_vao);
  if(_buffer)
    glDeleteBuffers(1, &_buffer);
  if(_buffer)
    glDeleteTextures(1, &_t1Id);
  if(_buffer)
    glDeleteTextures(1, &_t2Id);
  if(_oglContext)
    SDL_GL_DeleteContext(_oglContext);
  if(_win)
    SDL_DestroyWindow(_win);
  if(_coords) {
    free(_coords);
    _coords = NULL;
  }
  if(_colors) {
    free(_colors);
    _colors = NULL;
  }
  gl4duClean(GL4DU_ALL);
}
void animation_voronoi(int state) {
  /* INITIALISEZ VOS VARIABLES */
  switch(state) {
  case GL4DH_INIT:
    /* INITIALISEZ VOTRE ANIMATION (SES VARIABLES <STATIC>s) */
    init_voronoi();
    return;
  case GL4DH_FREE:
    /* LIBERER LA MEMOIRE UTILISEE PAR LES <STATIC>s */
    quit_voronoi();
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    /* METTRE A JOUR VOTRE ANIMATION EN FONCTION DU SON */
    audio_voronoi();
    return;
  default: /* GL4DH_DRAW */
    draw_voronoi();
    return;
  }
}
