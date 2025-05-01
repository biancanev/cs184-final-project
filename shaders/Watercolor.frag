#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Standard Phong-style lighting (from standard.frag)
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

// Paper & noise textures
uniform sampler2D u_noise_texture;  
uniform sampler2D u_paper_texture;  

// Watercolor palette
uniform vec3 u_color1;  // e.g. vec3(0.9, 0.3, 0.2);
uniform vec3 u_color2;  // e.g. vec3(0.2, 0.5, 0.8);
uniform vec3 u_color3;  // e.g. vec3(0.1, 0.7, 0.4);
uniform vec3 u_color4;  // e.g. vec3(0.8, 0.6, 0.1);

// Artistic parameters
uniform float u_edge_intensity;    // edge darkening strength
uniform float u_paper_visibility;  // how much paper shows through
uniform float u_granulation;       // pigment separation
uniform float u_edge_noise;        // noisy edge detail
uniform float u_transparency;      // overall paint opacity
uniform float u_dilute_strength;
uniform float u_bleed_strength;
uniform float u_edge_width;
uniform float u_edge_darkness;

uniform float u_overlay_threshold;

vec3 layer1colr(float f) {
    return mix(u_color1, u_color2, f);
}

vec3 layer2colr(float f) {
    return mix(u_color3, u_color4, f);
}

// Simple 1D ramp between four colors
// vec3 ramp_col(float t) {
//     t = clamp(t, 0.0, 1.0);
//     if (t < 1.0/3.0) {
//         return mix(u_color1, u_color2, t * 3.0);
//     } else if (t < 2.0/3.0) {
//         return mix(u_color2, u_color3, (t - 1.0/3.0) * 3.0);
//     } else {
//         return mix(u_color3, u_color4, (t - 2.0/3.0) * 3.0);
//     }
// }

void main() {
    // Standard lighting 
    vec3 norm = normalize(Normal);
    vec2 uv = TexCoords;
    if (uv.x < 1e-3 && uv.y < 1e-3)
        uv = FragPos.xz * 0.1;

    float noise1 = texture(u_noise_texture, uv * 3.0).r;
    float noise2 = texture(u_noise_texture, uv * 20.0).r;

    // Base color or texture 
    vec3 colBase = layer1colr(clamp(norm.y * 0.5 + 0.5 + (noise1 - 0.5) * 0.1, 0.0, 1.0));

    // Paper + noise sampling 
    float noise3 = texture(u_noise_texture, uv * 30.0).r;
    colBase *= (1.0 - (u_granulation * 0.6) * noise3);
    float paper = texture(u_paper_texture, uv * 5.0).r;
    colBase = mix(vec3(paper), colBase, u_paper_visibility * 1.2);

    // Highlight Dilution lighting for layer 1 
    float diff = max(dot(norm, normalize(lightPos - FragPos)), 0.0);
    vec3 diffuse = diff * lightColor;
    colBase = mix(colBase, vec3(paper), u_dilute_strength * (1.0 - diff) * 1.2);
    colBase *= (0.5 * lightColor + diff * lightColor * 0.5);

    // layer 2 
    float layer2mask = step(u_overlay_threshold, texture(u_noise_texture, uv * 0).r);
    vec3 layer2colr = layer2colr(clamp(norm.y * 0.5 + 0.5 + (noise2 - 0.5) * 0.1, 0.0, 1.0));
    layer2colr *= (1.0 - (u_granulation * 0.6) * noise3);
    layer2colr = mix(vec3(paper), layer2colr, u_paper_visibility * 1.2);
    vec3 finalCol = mix(colBase, layer2colr, layer2mask);

    // Bleed 
    float bleed_rand = texture(u_noise_texture, uv * 6.0).r;
    finalCol *= mix(1.0, 1.0 - u_bleed_strength * (0.7 + 0.5 * bleed_rand), noise2);
    float edgeInt = clamp(smoothstep(u_edge_width * 0.8, 0.0, length(dFdx(norm)) + length(dFdy(norm))) * u_edge_intensity, 0.0, 1.0);
    finalCol = mix(finalCol, finalCol * (1.0 - u_edge_darkness * 0.7), edgeInt);

    // Edge darkening 
    float edgeSil = clamp(smoothstep(u_edge_width * 0.5, 0.0, abs(dot(norm, normalize(viewPos - FragPos)))) * u_edge_intensity, 0.0, 1.0);
    finalCol = mix(finalCol, finalCol * (1.0 - u_edge_darkness * 0.7), edgeSil);

    // Variable transparency 
    finalCol = mix(finalCol, vec3(1.0), u_transparency * 0.3);
    finalCol += vec3(0.03);

    FragColor = vec4(finalCol, 1.0);
}
