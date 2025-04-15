#version 330
uniform vec3 u_cam_pos;
uniform vec3 u_light_pos;
uniform vec3 u_light_intensity;
uniform sampler2D u_noise_texture;    // Noise texture

// Enhanced grayscale shader parameters
uniform vec3 u_light_color = vec3(1.0, 1.0, 1.0);      
uniform vec3 u_dark_color = vec3(0.15, 0.15, 0.15);       
uniform vec3 u_mid_color = vec3(0.65, 0.65, 0.65);        
uniform float u_stroke_density = 150.0;                
uniform float u_line_thickness = 0.2;                 
uniform float u_outline_thickness = 0.4;               
uniform float u_tone_strength = 0.7;

in vec4 v_position;
in vec4 v_normal;
in vec2 v_uv;
out vec4 out_color;

// Generate thin line pattern using smoothstep for anti-aliasing
float lineMask(float coord, float thickness) {
    float lineCenter = fract(coord);
    // Create thin lines with slight anti-aliasing
    return smoothstep(0.0, thickness * 0.5, lineCenter) * 
           smoothstep(thickness * 1.5, thickness, lineCenter);
}

void main() {
    // Calculate basic lighting parameters
    vec3 fragPos = v_position.xyz / v_position.w;
    vec3 normal = normalize(v_normal.xyz);
    vec3 lightDir = normalize(u_light_pos - fragPos);
    vec3 viewDir = normalize(u_cam_pos - fragPos);
    
    // Calculate diffuse factor
    float diffuseFactor = max(dot(normal, lightDir), 0.0);
    
    // Calculate outline effect
    float outlineFactor = dot(viewDir, normal);
    outlineFactor = smoothstep(0.0, u_outline_thickness, outlineFactor);
    
    // Calculate screen-space coordinates for stable hatching
    vec2 screenPos = gl_FragCoord.xy / 1000.0;
    
    // Sample noise for variation
    float noise1 = texture(u_noise_texture, screenPos * 5.0).r;
    float noise2 = texture(u_noise_texture, screenPos * 15.0).r;
    
    // Calculate diagonal line patterns
    float dLine1 = 1.0 - lineMask((screenPos.x + screenPos.y) * u_stroke_density, u_line_thickness);
    float dLine2 = 1.0 - lineMask((screenPos.x - screenPos.y) * u_stroke_density, u_line_thickness);
    float dLine3 = 1.0 - lineMask((screenPos.x * 1.2 + screenPos.y * 0.8) * u_stroke_density, u_line_thickness);
    float dLine4 = 1.0 - lineMask((screenPos.x * 0.8 - screenPos.y * 1.2) * u_stroke_density, u_line_thickness);
    
    // Start with white (no hatches)
    float strokeFactor = 1.0;
    
    // Apply hatching based on light intensity with focus on diagonal lines
    if (diffuseFactor < 0.9) {
        strokeFactor = mix(strokeFactor, dLine1, smoothstep(0.9, 0.7, diffuseFactor));
    }
    if (diffuseFactor < 0.65) {
        strokeFactor = mix(strokeFactor, min(dLine1, dLine2), smoothstep(0.65, 0.45, diffuseFactor));
    }
    if (diffuseFactor < 0.4) {
        strokeFactor = mix(strokeFactor, min(min(dLine1, dLine2), dLine3), 
                         smoothstep(0.4, 0.25, diffuseFactor));
    }
    if (diffuseFactor < 0.2) {
        strokeFactor = mix(strokeFactor, min(min(min(dLine1, dLine2), dLine3), dLine4), 
                         smoothstep(0.2, 0.05, diffuseFactor));
    }
    
    // Add subtle variation with noise
    strokeFactor = mix(strokeFactor, strokeFactor * (0.95 + 0.05 * noise1), 0.2);
    
    // Always apply outline
    if (outlineFactor < 0.3) {
        strokeFactor = 0.0;
    }
    
    // Calculate a grayscale tone based on diffuse factor
    vec3 tonedColor;
    if (diffuseFactor > 0.7) {
        // Brightest areas: white to light gray
        tonedColor = mix(u_mid_color * 1.5, u_light_color, smoothstep(0.7, 1.0, diffuseFactor));
    } else if (diffuseFactor > 0.4) {
        // Mid-tone areas: dark gray to light gray
        tonedColor = mix(u_mid_color * 0.7, u_mid_color * 1.5, smoothstep(0.4, 0.7, diffuseFactor));
    } else {
        // Dark areas: black to dark gray
        tonedColor = mix(u_dark_color, u_mid_color * 0.7, smoothstep(0.0, 0.4, diffuseFactor));
    }
    
    // Combine hatching with grayscale toning
    // More hatching in darker areas, more tone in mid-to-bright areas
    vec3 hatchedColor = mix(u_dark_color, u_light_color, strokeFactor);
    vec3 finalColor = mix(hatchedColor, tonedColor, u_tone_strength * diffuseFactor);
    
    // Make sure outlines remain solid black
    if (outlineFactor < 0.3) {
        finalColor = u_dark_color;
    }
    
    // Output final color
    out_color = vec4(finalColor, 1.0);
}