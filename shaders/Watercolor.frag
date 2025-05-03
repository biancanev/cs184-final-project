#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Standard Phong-style lighting (from standard.frag)
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 ambientStrength;

uniform bool hasTexture;

// Paper & noise textures
uniform sampler2D u_noise_texture;  
uniform sampler2D u_paper_texture;
uniform sampler2D texture_diffuse1;

// Watercolor palette
uniform vec3 objectColor;
uniform vec3 u_color1;
uniform vec3 u_color2;
uniform vec3 u_color3;
uniform vec3 u_color4;

uniform float u_edge_intensity;    // edge darkening strength
uniform float u_paper_visibility;  // how much paper shows through
uniform float u_granulation;       // pigment separation
uniform float u_edge_noise;        // noisy edge detail
uniform float u_transparency;      // overall paint opacity

uniform float u_overlay_threshold;

vec3 layer1colr(float f) {
    return mix(u_color1, u_color2, f);
}

vec3 layer2colr(float f) {
    return mix(u_color3, u_color4, f);
}

vec3 ramp_col(float t) {
    t = clamp(t, 0.0, 1.0);
    if (t < 1/3.0) return mix(u_color1, u_color2, t*3.0);
    else if (t < 2/3.0) return mix(u_color2, u_color3, (t - 1/3.0)*3.0);
    else return mix(u_color3, u_color4, (t - 2/3.0)*3.0);
}

float noisetex(float t, vec2 uv) {
    return texture(u_noise_texture, uv * t).r;
}

void main() {
    // Standard lighting 
    vec3 norm = normalize(Normal);
    vec3 vdir = normalize(viewPos - FragPos);
    vec2 uv = TexCoords;
    if (uv.x < 1e-3 && uv.y < 1e-3)
        uv = FragPos.xz * 0.1;

    // Base color or texture 
    // vec3 colBase = layer1colr(clamp(norm.y * 0.5 + 0.5 + (noise1 - 0.5) * 0.1, 0.0, 1.0));
    vec3 colBase = hasTexture ? texture(texture_diffuse1, uv).rgb : objectColor;
    float paper = texture(u_paper_texture, uv * 3.0).r;
    float noise1 = noisetex(2.5, uv);
    float noise2 = noisetex(8.0, uv);
    float noise3 = noisetex(20.0, uv);
    float hue = clamp(norm.y * 0.5 + 0.5 + 0.1 * (noise1 - 0.5), 0.0, 1.0);
    vec3 col = ramp_col(hue);
    if (hue < 1/3.0)
        col = mix(u_color1, u_color2, smoothstep(0.0, 0.5, hue * 1/(1/3.0) + noise2 * 0.2));
    else if (hue < 2/3.0)
        col = mix(u_color2, u_color3, smoothstep(0.0, 0.5, (hue - 1/3.0) * 1/(1/3.0) + noise1 * 0.2));
    else
        col = mix(u_color3, u_color4, smoothstep(0.0, 0.5, (hue - 2/3.0) * 1/(1/3.0) + noise3 * 0.2));
    
    float sepgran = u_granulation * (1.0 - paper);
    col.r *= 1.0 - noise3 * sepgran;
    col.g *= 1.0 - noise2 * sepgran;
    col.b *= 1.0 - noise1 * sepgran;

    float edge = (pow(1.0 - dot(norm, vdir), 1.7) * u_edge_intensity) * (1.0 + u_edge_noise * (noise3 - 0.5));
    col = mix(vec3(1.0), col, u_paper_visibility);
    col = mix(col, col * 0.3, clamp(edge, 0.0, 1.0));

    FragColor = vec4(colBase * col, 1.0);

}
