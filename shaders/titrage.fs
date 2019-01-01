#version 330
uniform float dt;
uniform sampler2D tex0;
uniform sampler2D tex1;
in  vec2 vsoTexCoord;
out vec4 fragColor;

void main(void) {
  float t = 8 * dt - 4.0;
  float m = exp(-0.5 * t * t);
  vec4 c0 = texture(tex0, vsoTexCoord);
  vec4 c1 = texture(tex1, vsoTexCoord);
  c1 = length(c1.rgb) > 0.0 ? c1 : c0;
  fragColor = mix(c0, c1, m);
}
