/*!\file animations.h
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
#ifndef _ANIMATIONS_H

#define _ANIMATIONS_H

#ifdef __cplusplus
extern "C" {
#endif

  extern void transition_fondu(void (* a0)(int), void (* a1)(int), Uint32 t, Uint32 et, int state);
  extern void fondui(void (* a0)(int), void (* a1)(int), Uint32 t, Uint32 et, int state);
  extern void animation_flash(int state);
  extern void animation_vide(int state);
  extern void animation_earth(int state);
  extern void animation_n(int state);
  extern void animation_voronoi(int state);
  extern void animation_effet(int state);
  extern void animation_geom(int state);

  extern void animation_1(int state);
  extern void animation_2(int state);
  extern void animation_3(int state);
  extern void animation_4(int state);
  extern void animation_5(int state);
  extern void animation_6(int state);
  extern void animation_7(int state);

  extern void bidon00(int state);
  extern void bidon01(int state);
  extern void bidon02(int state);
  extern void bidon03(int state);
  
  extern void credits(int state);
  extern void animationsInit(void);
  

  

#ifdef __cplusplus
}
#endif

#endif
