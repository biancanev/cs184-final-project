#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform sampler2D u_noise_texture;    // Noise texture
uniform float time;                   // Add time uniform for dynamic variation

// Enhanced grayscale shader parameters
uniform vec3 u_light_color = vec3(1.0, 1.0, 1.0);      
uniform vec3 u_dark_color = vec3(0.0, 0.0, 0.0);       // Pure black for outlines
uniform vec3 u_mid_color = vec3(0.65, 0.65, 0.65);        
uniform float u_stroke_density = 150.0;                
uniform float u_line_thickness = 0.2;                 
uniform float u_outline_thickness = 0.7;               
uniform float u_tone_strength = 0.7;
uniform float u_detail_enhancement = 4.0;    
uniform float u_edge_contrast = 2.0;            
uniform float u_stroke_randomness = 0.5;

uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;

// Generate random value based on position
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Generate thin line pattern with randomness
float lineMask(float coord, float thickness, vec2 pos) {
    // Add randomness to line position
    float noise = texture(u_noise_texture, pos * 25.0).r;
    float noiseOffset = (noise - 0.5) * u_stroke_randomness * 0.2;
    
    // Add randomness to line thickness
    float thicknessVar = thickness * (1.0 + (noise - 0.5) * u_stroke_randomness * 0.6);
    
    // Add slight waviness to the lines
    float waviness = sin(pos.y * 50.0) * u_stroke_randomness * 0.4;
    
    float lineCenter = fract(coord + noiseOffset + waviness);
    
    // Create uneven line edges for hand-drawn feel
    float edgeNoise1 = texture(u_noise_texture, pos * 80.0).r * 0.03 * u_stroke_randomness;
    float edgeNoise2 = texture(u_noise_texture, pos * 90.0).r * 0.03 * u_stroke_randomness;
    
    return smoothstep(0.0, thicknessVar * 0.5 + edgeNoise1, lineCenter) * 
           smoothstep(thicknessVar * 1.5 + edgeNoise2, thicknessVar, lineCenter);
}

void main() {
    // Calculate basic lighting parameters
    vec3 fragPos = FragPos;
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 ambient = ambientStrength * lightColor;
    
    // Calculate diffuse factor
    float diffuseFactor = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    float totalLightingFactor = (ambient.r + diffuseFactor + specular.r);

    diffuseFactor = min(totalLightingFactor, 1.0);
    
    // Enhanced outline effect - stronger outline based on view angle
    float outlineFactor = dot(viewDir, normal);
    outlineFactor = smoothstep(0.0, u_outline_thickness, outlineFactor);
    
    // Make outline bolder by widening the outline threshold
    float outlineThreshold = 0.5;
    
    // Calculate screen-space coordinates for stable hatching
    vec2 screenPos = gl_FragCoord.xy / 1000.0;
    
    // Add noise for nice random sketch look
    float noise1 = texture(u_noise_texture, screenPos * 5.0).r;
    float noise2 = texture(u_noise_texture, screenPos * 15.0).r;
    float noise3 = texture(u_noise_texture, screenPos * 30.0).r;
    
    // Detail detection - using curvature approximation
    float curvature = 1.0 - abs(dot(normal, viewDir));
    float detailFactor = pow(curvature, u_detail_enhancement) * u_edge_contrast;
    
    // Calculate diagonal line patterns with randomness
    // Each line pattern gets different randomization for natural look
    float dLine1 = 1.0 - lineMask((screenPos.x + screenPos.y) * u_stroke_density, 
                                u_line_thickness, screenPos * 1.1);
    float dLine2 = 1.0 - lineMask((screenPos.x - screenPos.y) * u_stroke_density, 
                                u_line_thickness * 0.9, screenPos * 2.3);
    float dLine3 = 1.0 - lineMask((screenPos.x * 1.2 + screenPos.y * 0.8) * u_stroke_density, 
                                u_line_thickness * 1.1, screenPos * 3.7);
    float dLine4 = 1.0 - lineMask((screenPos.x * 0.8 - screenPos.y * 1.2) * u_stroke_density, 
                                u_line_thickness * 0.85, screenPos * 5.2);
    
    // Randomize stroke angles slightly for imperfect look
    vec2 distortedPos = screenPos + vec2(noise3 - 0.5, noise1 - 0.5) * u_stroke_randomness * 0.03;
    float dLine5 = 1.0 - lineMask((distortedPos.x * 0.7 + distortedPos.y * 1.3) * u_stroke_density, 
                                u_line_thickness, screenPos * 6.1);
    
    // Start with white (no hatches)
    float strokeFactor = 1.0;
    
    // Apply hatching based on light intensity with focus on diagonal lines
    // Add randomized thresholds for more organic transitions
    float thresh1 = 0.9 + (noise1 - 0.5) * 0.1 * u_stroke_randomness;
    float thresh2 = 0.65 + (noise2 - 0.5) * 0.15 * u_stroke_randomness;
    float thresh3 = 0.4 + (noise3 - 0.5) * 0.12 * u_stroke_randomness;
    float thresh4 = 0.2 + (noise1 - 0.5) * 0.1 * u_stroke_randomness;
    
    if (diffuseFactor < thresh1) {
        // Randomize the mix amount for natural transition
        float mixAmount = smoothstep(thresh1, thresh1 - 0.2 - noise2 * 0.1, diffuseFactor);
        strokeFactor = mix(strokeFactor, dLine1, mixAmount);
    }
    if (diffuseFactor < thresh2) {
        float mixAmount = smoothstep(thresh2, thresh2 - 0.2 - noise1 * 0.1, diffuseFactor);
        // Occasionally skip a hatching layer for variety
        if (noise3 > 0.1) {
            strokeFactor = mix(strokeFactor, min(dLine1, dLine2), mixAmount);
        }
    }
    if (diffuseFactor < thresh3) {
        float mixAmount = smoothstep(thresh3, thresh3 - 0.15 - noise3 * 0.1, diffuseFactor);
        // Sometimes add line5 instead of line3 for variety
        if (noise1 > 0.5) {
            strokeFactor = mix(strokeFactor, min(min(dLine1, dLine2), dLine3), mixAmount);
        } else {
            strokeFactor = mix(strokeFactor, min(min(dLine1, dLine2), dLine5), mixAmount);
        }
    }
    if (diffuseFactor < thresh4) {
        float mixAmount = smoothstep(thresh4, thresh4 - 0.15 - noise2 * 0.1, diffuseFactor);
        strokeFactor = mix(strokeFactor, min(min(min(dLine1, dLine2), dLine3), dLine4), mixAmount);
    }
    
    // Add paper texture effect
    float paperGrain = texture(u_noise_texture, screenPos * 50.0).r;
    strokeFactor = mix(strokeFactor, strokeFactor * (0.9 + 0.1 * paperGrain), 0.3);
    
    // Always apply outline with enhanced boldness and slight variation
    if (outlineFactor < outlineThreshold) {
        // Add noise to outline for sketch-like effect
        float outlineNoise = mix(0.0, 0.15, noise2);
        
        // Make the outline vary in thickness
        float outlineVar = outlineThreshold * (1.0 + (noise3 - 0.5) * 0.3 * u_stroke_randomness);
        if (outlineFactor < outlineVar) {
            strokeFactor = outlineNoise;
        }
    }
    
    // Enhance details (eyes, facial features, etc.)
    // Dark lines will appear in areas of high detail (where detailFactor is high)
    if (detailFactor > 0.3) {
        float detailStrength = smoothstep(0.3, 0.7, detailFactor);
        // Add uneven darkening to details
        float detailNoise = 0.1 + noise1 * 0.1 * u_stroke_randomness;
        strokeFactor = mix(strokeFactor, detailNoise, detailStrength * 0.8);
    }
    
    // Calculate a grayscale tone based on diffuse factor
    vec3 tonedColor;
    if (diffuseFactor > 0.6) {
        // Brightest areas: white to light gray
        tonedColor = mix(u_mid_color * 1.5, u_light_color, smoothstep(0.7, 1.0, diffuseFactor));
    } else if (diffuseFactor > 0.4) {
        // Mid-tone areas: dark gray to light gray
        tonedColor = mix(u_mid_color * 0.7, u_mid_color * 1.5, smoothstep(0.4, 0.7, diffuseFactor));
    } else {
        // Dark areas: black to dark gray
        tonedColor = mix(u_dark_color, u_mid_color * 0.7, smoothstep(0.0, 0.4, diffuseFactor));
    }
    
    // Add slight paper color variation
    tonedColor = tonedColor * (0.97 + 0.06 * paperGrain);
    
    // Combine hatching with grayscale toning
    vec3 hatchedColor = mix(u_dark_color, u_light_color, strokeFactor);
    vec3 finalColor = mix(hatchedColor, tonedColor, u_tone_strength * diffuseFactor);
    
    // Make sure outlines and details remain pure black, with slight variation
    if (outlineFactor < outlineThreshold || detailFactor > 0.5) {
        // Slight variation in darkness for hand-drawn feel
        float darkVar = 0.05 * noise3 * u_stroke_randomness;
        finalColor = u_dark_color + vec3(darkVar);
    }
    
    // Output final color
    FragColor = vec4(finalColor, 1.0);
}