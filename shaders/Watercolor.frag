#version 330
uniform vec3 u_cam_pos;
uniform vec3 u_light_pos;
uniform vec3 u_light_intensity;
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

in vec4 v_position;
in vec4 v_normal;
in vec2 v_uv;
out vec4 out_color;

void main() {
    // Get position and normal data
    vec3 fragPos = v_position.xyz / v_position.w;
    vec3 normal = normalize(v_normal.xyz);
    vec3 viewDir = normalize(u_cam_pos - fragPos);
    
    // Multi-layered noise for complex watercolor effects
    vec2 noiseCoord1 = v_uv * 2.5;
    vec2 noiseCoord2 = v_uv * 8.0;
    vec2 noiseCoord3 = v_uv * 20.0;
    float noise1 = texture(u_noise_texture, noiseCoord1).r;
    float noise2 = texture(u_noise_texture, noiseCoord2).r;
    float noise3 = texture(u_noise_texture, noiseCoord3).r;
    
    // Paper texture (either from actual texture or generated)
    float paperTexture = texture(u_paper_texture, v_uv * 3.0).r;
    paperTexture = mix(0.8, 1.0, paperTexture);
    
    // CRITICAL: Edge detection for watercolor edge darkening
    float edgeFactor = 1.0 - abs(dot(normal, viewDir));
    edgeFactor = pow(edgeFactor, 1.7) * u_edge_intensity;
    
    // Create noisy edges (essential for watercolor look)
    edgeFactor *= (1.0 + (noise3 - 0.5) * u_edge_noise);
    
    // Create watercolor color blending with visible paper texture
    float distortion = noise1 * 0.3;
    vec2 blendUV = v_uv + vec2(noise2 - 0.5, noise1 - 0.5) * 0.12;
    
    // Main watercolor hue selection with true pigment separation
    float hueSelect = blendUV.x + blendUV.y * 0.7 + noise1 * 0.3;
    vec3 color;
    
    // Create color regions with smooth, irregular transitions
    if (hueSelect < 0.33) {
        color = mix(u_color1, u_color2, smoothstep(0.0, 0.5, hueSelect / 0.33 + noise2 * 0.2));
    } else if (hueSelect < 0.66) {
        color = mix(u_color2, u_color3, smoothstep(0.0, 0.5, (hueSelect - 0.33) / 0.33 + noise1 * 0.2));
    } else {
        color = mix(u_color3, u_color4, smoothstep(0.0, 0.5, (hueSelect - 0.66) / 0.33 + noise3 * 0.2));
    }
    
    // CRITICAL: Pigment granulation - different colors separate differently
    // This creates the characteristic watercolor granulation effect
    float redGranulation = noise3 * u_granulation * 0.6;
    float greenGranulation = noise2 * u_granulation * 0.5;
    float blueGranulation = noise1 * u_granulation * 0.7;
    
    color.r = color.r * (1.0 - redGranulation * (1.0 - paperTexture));
    color.g = color.g * (1.0 - greenGranulation * (1.0 - paperTexture));
    color.b = color.b * (1.0 - blueGranulation * (1.0 - paperTexture));
    
    // Add paper visibility - key watercolor characteristic
    color = mix(color, vec3(paperTexture), u_paper_visibility * paperTexture);
    
    // Add simple lighting
    vec3 lightDir = normalize(u_light_pos - fragPos);
    float diffuseFactor = max(dot(normal, lightDir), 0.0);
    float lightIntensity = 0.6 + 0.4 * diffuseFactor;
    
    // Dark edges - the most characteristic watercolor feature
    color = mix(color, color * 0.3, edgeFactor);
    
    // Slightly darken concave areas (pigment pooling)
    float ao = pow((1.0 - diffuseFactor), 2.0) * 0.4;
    color = color * (1.0 - ao);
    
    // CRITICAL: Transparency variation - watercolors are never uniform in opacity
    float opacity = u_transparency;
    opacity = opacity * (1.0 - noise2 * 0.2);  // Varied transparency
    opacity = mix(opacity, opacity * 1.5, edgeFactor); // More opaque at edges
    
    // Final color with paper and light influence
    vec3 finalColor = color * lightIntensity;
    
    // Output with variable opacity
    out_color = vec4(finalColor, opacity);
}