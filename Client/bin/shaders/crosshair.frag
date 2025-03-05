/// TODO: can I make this with the bullet spread indicator that I want? Depends on player position and mouse position (in game world, not in shader coords)
// yes, I can, but the shader would then have to be as large as the window size of the game and center of the shader would be the player pos, makes sense or nah?

#ifdef GL_ES
precision lowp float;
#endif

#define PI 3.14159265359
#define TWO_PI 6.28318530718

uniform vec2 u_resolution;
uniform vec2 u_mouse;

// float box(in vec2 _st, in vec2 _size) {
//     _size = vec2(0.5) - _size * 0.5;
//     vec2 uv = smoothstep(_size,
//                         _size + vec2(0.001),
//                         _st);
//     uv *= smoothstep(_size,
//                     _size + vec2(0.001),
//                     vec2(1.0)-_st);
//     return uv. * uv.y;
// }

// float cross(in vec2 _st, float _size) {
//     return box(_st, vec2(_size, _size / 4.)) + box(_st, vec2(_size / 4., _size));
// }

void main() {
    vec2 st = gl_FragCoord.xy / u_resolution.xy;
    vec3 color; /// TODO: make determined by player, customizable, is that possible?

    // (st.x < 0.6 && st.x > 0.4)
    float a = step(0.4, st.x) * step(0.4, 1.0 - st.x);
    // (st.y < 0.6 && st.x > 0.4)
    float b = step(0.4, st.y) * step(0.4, 1.0 - st.y);
    // a || b
    float pct = a + b - a * b;

    color = vec3(pct);

    gl_FragColor = vec4(color, 1.0);
}