/*!\file window.c
 *
 * \brief Utilisation de GL4Dummies pour réaliser une démo.
 *
 * Ici est géré l'ouverture de la fenêtre ainsi que l'ordonnancement
 * des animations. Apriori les seuls éléments à modifier ici lors de
 * votre intégration sont le tableau static \ref _animations et le nom
 * du fichier audio à lire.
 *
 * \author Farès BELHADJ, amsi@ai.univ-paris8.fr
 * \date May 05, 2014 - February 26, 2016
 */
#include <stdlib.h>
#include <GL4D/gl4du.h>
#include <GL4D/gl4dh.h>
#include <GL4D/gl4duw_SDL2.h>
#include "animations.h"
#include "audioHelper.h"

/* Prototypes des fonctions statiques contenues dans ce fichier C. */
static void init(void);
static void quit(void);
static void resize(int w, int h);
static void keydown(int keycode);

/*!\brief tableau contenant les animations sous la forme de timeline,
 * ce tableau se termine toujours par l'élémént {0, NULL, NULL,
 * NULL} */
static GL4DHanime _animations[] = {

  { 1000, animation_vide, animation_flash, transition_fondu },
  { 1000, bidon01, NULL, NULL },
  { 1000, bidon01, bidon02,fondui },
  { 1000, bidon02, NULL, NULL },
  { 1000, bidon02, bidon03,fondui },
  { 1000, bidon03, NULL, NULL },
  { 1000, bidon03, animation_effet, fondui },
  { 7000, animation_effet, NULL, NULL },

  { 1000, animation_effet, animation_7, fondui },
  { 5000, animation_7, NULL, NULL },

  { 1000, animation_7, animation_flash, transition_fondu },
  { 1000, animation_flash, animation_1, transition_fondu },
  { 7000, animation_1, NULL, NULL },

  { 1000, animation_1, animation_flash, transition_fondu },
  { 1000, animation_flash, animation_3, transition_fondu },
  { 7000, animation_3, NULL, NULL },

  { 1000, animation_3, animation_flash, transition_fondu },
  { 1000, animation_flash, animation_2, transition_fondu },
  { 7000, animation_2, NULL, NULL },
  
  { 1000, animation_2, animation_flash, transition_fondu },
  { 1000, animation_flash, animation_4, transition_fondu },
  { 7000, animation_4, NULL, NULL },
  
  { 1000,  animation_4, animation_flash, transition_fondu },
  { 1000, animation_flash, animation_voronoi, transition_fondu },
  { 10000, animation_voronoi, NULL, NULL },

  { 1000,  animation_voronoi, animation_flash, transition_fondu },
  { 1000, animation_flash, animation_geom, transition_fondu },
  { 7000, animation_geom, NULL, NULL },

  { 1000,  animation_geom, animation_flash, transition_fondu },
  { 1000,  animation_flash, animation_earth, transition_fondu },
  { 6000, animation_earth, NULL, NULL },

  { 1000,  animation_earth, animation_flash, transition_fondu },
  { 1000, animation_flash, animation_n, transition_fondu},
  { 7000,animation_n, NULL, NULL },

  { 1000, animation_n, animation_flash, transition_fondu },
  { 1000, animation_flash, credits, transition_fondu},
  { 10000,credits, NULL, NULL },

  { 1000, credits, animation_flash, transition_fondu },
  { 1000, animation_flash, animation_6, transition_fondu},
  { 5000,animation_6, NULL, NULL },

  { 5000, animation_6, animation_vide, transition_fondu },

  {    0, NULL, NULL, NULL }
};

/*!\brief dimensions initiales de la fenêtre */
static GLfloat _dim[] = {1024,  600};

/*!\brief fonction principale : initialise la fenêtre, OpenGL, audio
 * et lance la boucle principale (infinie).
 */
int main(int argc, char ** argv) {
  if(!gl4duwCreateWindow(argc, argv, "GL4Dummies DemoHelper", 
			 10,10, 
			 _dim[0], _dim[1],SDL_WINDOW_SHOWN))//|SDL_WINDOW_RESIZABLE
    return 1;
  init();
  atexit(quit);
  gl4duwResizeFunc(resize);
  gl4duwKeyDownFunc(keydown);
  gl4duwDisplayFunc(gl4dhDraw);

  ahInitAudio("audio/Isisip - Lotus Flower.mp3");
  gl4duwMainLoop();
  return 0;
}

/*!\brief Cette fonction initialise les paramètres et éléments OpenGL
 * ainsi que les divers données et fonctionnalités liées à la gestion
 * des animations.
 */
static void init(void) {
  glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
  gl4dhInit(_animations, _dim[0], _dim[1], animationsInit);
  resize(_dim[0], _dim[1]);
}

/*!\brief paramétre la vue (viewPort) OpenGL en fonction des
 * dimensions de la fenêtre.
 * \param w largeur de la fenêtre.
 * \param w hauteur de la fenêtre.
 */
static void resize(int w, int h) {
  _dim[0] = w; _dim[1] = h;
  glViewport(0, 0, _dim[0], _dim[1]);
}

/*!\brief permet de gérer les évènements clavier-down.
 * \param keycode code de la touche pressée.
 */
static void keydown(int keycode) {
  switch(keycode) {
  case SDLK_ESCAPE:
  case 'q':
    exit(0);
  default: break;
  }
}

/*!\brief appelée à la sortie du programme (atexit).
 */
static void quit(void) {
    ahClean();
  gl4duClean(GL4DU_ALL);
}
