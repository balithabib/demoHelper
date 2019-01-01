/*!\file basic.fs
 * \brief Cellules de Voronoi
 * \author Farès Belhadj amsi@ai.uiv-paris8.fr
 * \date April 03, 2014
 */
#version 330
uniform int nbSites;
uniform sampler1D siteCoords, siteColors;
uniform vec2 dim;
out vec4 fragColor;

float dist(vec2 a, vec2 b) {
  float dx = a.x - b.x, dy = a.y - b.y;
  return dx * dx + dy * dy;
}

vec4 couleur_simple(void) {
  vec2 c = gl_FragCoord.xy / dim;
  float i, l, step = 1.0 / float(nbSites);
  float mini = 0, minl = dist(c, texture(siteCoords, mini).rg);
  for(i = step; i < 1; i += step) {
    l = dist(c, texture(siteCoords, i).rg);
    if(minl > l) {
      minl = l;
      mini = i;
    }
  }
  if(minl > 0.000005)
    return texture(siteColors, mini);
  return vec4(0, 0, 0, 1);
}

vec4 couleur_progressive(void) {
  vec2 c = gl_FragCoord.xy / dim;
  float step = 1.0 / float(nbSites - 1), i, l;
  float mini[2] = float[2](0, step); 
  float minl[2] = float[2](dist(c, texture(siteCoords, mini[0]).rg), 
			   dist(c, texture(siteCoords, mini[1]).rg));
  if(minl[0] > minl[1]) {
    l = minl[0];
    minl[0] = minl[1];
    minl[1] = l;
    mini[0] = step;
    mini[1] = 0;
  }
  for(i = step + step; i < 1; i += step) {
    l = dist(c, texture(siteCoords, i).rg);
    if(minl[0] > l) {
      minl[1] = minl[0];
      mini[1] = mini[0];
      minl[0] = l;
      mini[0] = i;
    } else if(minl[1] > l) {
      minl[1] = l;
      mini[1] = i;
    }
  }
  return ((minl[1] - minl[0]) * texture(siteColors, mini[0]) + 
	  2.0 * minl[0] * vec4(0, 0, 0, 1)) / (minl[1] + minl[0]);
}

void main(void) {
  //fragColor = couleur_simple();
  fragColor = couleur_progressive();
}
