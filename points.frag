#version 460 
in float vIntensity;
out vec4 FragColor;

void main() {
   
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord) * 2.0;

    if (dist > 1.0) discard; // circular clipping

    float glow = exp(-dist * dist * 3.0) * vIntensity ;

    vec3 deepBlue = vec3(0.0, 0.05, 0.5);
    vec3 lightPurple = vec3(0.6, 0.3, 0.9);
    vec3 white = vec3(1.0, 1.0, 1.0);

    vec3 color;
    if (vIntensity < 0.5) {
        color = mix(deepBlue, lightPurple, vIntensity * 2.0);
    } else {
        color = mix(lightPurple, white, (vIntensity - 0.5) * 2.0);
    }

    FragColor = vec4(color * glow, glow);
}