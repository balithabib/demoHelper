#version 330

layout (location = 0) in vec2 vsiPosition;
 
void main(void) {
   gl_Position = vec4(vsiPosition, 0.0, 1.0);
}
