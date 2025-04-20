#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// uniform vec3 lightPos;
// uniform vec3 viewPos;
// uniform vec3 lightColor;
uniform vec3 objectColor;
uniform sampler2D u_noise_texture;      // Noise texture for paper grain
uniform sampler2D u_paper_texture;      // Optional actual paper texture

// Multiple colors for watercolor palette
uniform vec3 u_color1 = vec3(0.9, 0.3, 0.2);  // Warm red
uniform vec3 u_color2 = vec3(0.2, 0.5, 0.8);  // Mid blue
uniform vec3 u_color3 = vec3(0.1, 0.7, 0.4);  // Seafoam green
uniform vec3 u_color4 = vec3(0.8, 0.6, 0.1);  // Golden yellow

// True watercolor effect parameters
uniform float u_edge_intensity = 2.5;      // Edge darkening intensity
uniform float u_paper_visibility = 0.3;    // How much paper shows through
uniform float u_granulation = 0.7;         // Pigment separation amount
uniform float u_edge_noise = 0.8;          // Noisy edge detail
uniform float u_transparency = 0.85;       // Overall paint transparency

vec3 ramp_col(float t) {
    t = clamp(t, 0.0, 1.0);
    if (t < 1.0/3.0) {
        return mix(u_color1, u_color2, t * 3.);
    } else if (t < 2.0/3.0) {
        return mix(u_color2, u_color3, (t - 1.0/3.0) * 3.);
    } else {
        return mix(u_color3, u_color4, (t - 2.0/3.0) * 3.);
    }
}

void main() {
    // Get position and normal data
    vec3 N = normalize(Normal);
    vec2 uv = TexCoords;
    if (uv.x < 1e-3 && uv.y < 1e-3) {
        uv = FragPos.xz * 0.1;
    }

    vec3 fragPos = FragPos;
    vec3 viewDir = normalize(- FragPos);
    
    // Multi-layered noise for complex watercolor effects
    float noise1 = texture(u_noise_texture, uv * 2.5).r;
    float noise2 = texture(u_noise_texture, uv * 8.0).r;
    float noise3 = texture(u_noise_texture, uv * 20.0).r;
    
    // Paper texture (either from actual texture or generated)
    float paper = mix(0.8, 1.0, texture(u_paper_texture, uv * 3.0).r);
    
    // CRITICAL: Edge detection for watercolor edge darkening
    float edge = 1.0 - abs(dot(N, normalize(-FragPos)));
    edge = pow(edge, 1.7) * u_edge_intensity;
    
    // Create noisy edges (essential for watercolor look)
    edge *= (1.0 + (noise3 - 0.5) * u_edge_noise);
    
    // Create watercolor color blending with visible paper texture    
    float hue = clamp(N.y * .5 + .5 + (noise1 - .5) * .1, 0.0, 1.0);
    vec3 color = ramp_col(hue);
    
    // Create color regions with smooth, irregular transitions
    if (hue < 0.33) {
        color = mix(u_color1, u_color2, smoothstep(0.0, 0.5, hue / 0.33 + noise2 * 0.2));
    } else if (hue < 0.66) {
        color = mix(u_color2, u_color3, smoothstep(0.0, 0.5, (hue - 0.33) / 0.33 + noise1 * 0.2));
    } else {
        color = mix(u_color3, u_color4, smoothstep(0.0, 0.5, (hue - 0.66) / 0.33 + noise3 * 0.2));
    }
    
    float redGranulation = noise3 * u_granulation * 0.6;
    float greenGranulation = noise2 * u_granulation * 0.5;
    float blueGranulation = noise1 * u_granulation * 0.7;
    
    color.r *= (1.0 - redGranulation * (1.0 - paper));
    color.g *= (1.0 - greenGranulation * (1.0 - paper));
    color.b *= (1.0 - blueGranulation * (1.0 - paper));
    // color.r *= 1.0 - noise3 * u_granulation * (1.0 - paper);
    // color.g *= 1.0 - noise2 * u_granulation * (1.0 - paper);
    // color.b *= 1.0 - noise1 * u_granulation * (1.0 - paper);
    
    // Add paper visibility - key watercolor characteristic
    color = mix(vec3(paper), color, u_paper_visibility * paper);
    
    // Add simple lighting
    // vec3 lightDir = normalize(lightPos - fragPos);
    // float diffuseFactor = max(dot(normal, lightDir), 0.0);
    // float lightIntensity = 0.6 + 0.4 * diffuseFactor;
    
    // Dark edges - the most characteristic watercolor feature
    // color = mix(color, color * 0.3, edge);
    color *= mix(.8, 1., texture(u_paper_texture, uv * 20.).r * .3);
    
    // Slightly darken concave areas (pigment pooling)
    // float ao = pow((1.0 - diffuseFactor), 2.0) * 0.4;
    // color = color * (1.0 - ao);
    
    // CRITICAL: Transparency variation - watercolors are never uniform in opacity
    float opacity = u_transparency;
    opacity = opacity * (1.0 - noise2 * 0.2);  // Varied transparency
    opacity = mix(opacity, opacity * 1.5, edge); // More opaque at edges
    opacity = clamp(opacity, 0.0, 1.0);
    
    // Final color with paper and light influence
    vec3 finalColor = color;
    if (edge > 0.3) finalColor = mix(finalColor, vec3(0.0), edge * 0.5);
    
    // Output with variable opacity
    FragColor = vec4(finalColor, opacity);
}