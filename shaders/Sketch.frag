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
uniform float time;                   // Keep this uniform even if we don't use it

// Enhanced grayscale shader parameters with stronger hatching
uniform vec3 u_light_color = vec3(1.0, 1.0, 1.0);      
uniform vec3 u_dark_color = vec3(0.0, 0.0, 0.0);       // Pure black for outlines
uniform vec3 u_mid_color = vec3(0.75, 0.75, 0.75);     // Lighter mid-color
uniform float u_stroke_density = 180.0;                // Higher density for more prominent lines
uniform float u_line_thickness = 0.25;                 // Thicker lines for visibility
uniform float u_outline_thickness = 0.7;               
uniform float u_tone_strength = 0.5;                   // Lower tone strength to make hatching more visible
uniform float u_detail_enhancement = 4.0;    
uniform float u_edge_contrast = 2.0;            
uniform float u_stroke_randomness = 0.5;              
uniform float u_hatching_opacity = 0.9;                // Higher opacity for stronger hatching
uniform float u_gradient_strength = 2.2;               // Steeper gradient

uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;

// Generate random value based on position
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Generate thin line pattern with enhanced randomness and organic feel
float lineMask(float coord, float thickness, vec2 pos) {
    // Add randomness to line position
    float noise = texture(u_noise_texture, pos * 25.0).r;
    float noiseOffset = (noise - 0.5) * u_stroke_randomness * 0.2;
    
    // Add randomness to line thickness
    float thicknessVar = thickness * (1.0 + (noise - 0.5) * u_stroke_randomness * 0.6);
    
    // Add more pronounced waviness to the lines - INCREASED EFFECT
    float waviness = sin(pos.y * 50.0 + cos(pos.x * 30.0)) * u_stroke_randomness * 0.25;
    waviness += cos(pos.y * 35.0 + sin(pos.x * 20.0)) * u_stroke_randomness * 0.12;
    
    float lineCenter = fract(coord + noiseOffset + waviness);
    
    // Create uneven line edges for hand-drawn feel - ENHANCED
    float edgeNoise1 = texture(u_noise_texture, pos * 80.0).r * 0.06 * u_stroke_randomness;
    float edgeNoise2 = texture(u_noise_texture, pos * 90.0).r * 0.06 * u_stroke_randomness;
    
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
    
    // Create a more pronounced light-to-dark gradient
    float enhancedDiffuse = pow(diffuseFactor, u_gradient_strength);
    
    // Enhanced outline effect - stronger outline based on view angle
    float outlineFactor = dot(viewDir, normal);
    outlineFactor = smoothstep(0.0, u_outline_thickness, outlineFactor);
    
    // Make outline bolder by widening the outline threshold
    float outlineThreshold = 0.5;
    
    // Calculate screen-space coordinates for stable hatching
    vec2 screenPos = gl_FragCoord.xy / 1000.0;
    
    // Remove time-varying factor and replace with a constant value
    // This makes the shader static while keeping the effect
    float staticOffset = 0.02; // Small constant offset instead of time-based one
    
    // Add noise for nice random sketch look
    float noise1 = texture(u_noise_texture, screenPos * 5.0).r;
    float noise2 = texture(u_noise_texture, screenPos * 15.0).r;
    float noise3 = texture(u_noise_texture, screenPos * 30.0).r;
    
    // Detail detection - using curvature approximation
    float curvature = 1.0 - abs(dot(normal, viewDir));
    float detailFactor = pow(curvature, u_detail_enhancement) * u_edge_contrast;
    
    // VARIED ANGLES: Calculate varied line patterns with randomness
    // Use noise to vary the angle of each group of lines
    // Replaced timeVar with staticOffset
    float angle1 = noise1 * 0.3 + 0.2 + staticOffset; // Range ~0.2-0.5
    float angle2 = angle1 + 0.5 + (noise2 - 0.5) * 0.7; // ~90° offset with variation
    float angle3 = angle1 - 0.2 + (noise3 - 0.5) * 0.3; // Similar angle to first but varied
    float angle4 = angle2 + 0.2 + (noise1 - 0.5) * 0.25; // Similar angle to second but varied
    float angle5 = angle1 + 0.8 + (noise2 - 0.5) * 0.75; // Different angle with variation
    
    // Create rotation matrices for varied line angles
    vec2 rotateUV1 = vec2(cos(angle1 * 3.14159), sin(angle1 * 3.14159));
    vec2 rotateUV2 = vec2(cos(angle2 * 3.14159), sin(angle2 * 3.14159));
    vec2 rotateUV3 = vec2(cos(angle3 * 3.14159), sin(angle3 * 3.14159));
    vec2 rotateUV4 = vec2(cos(angle4 * 3.14159), sin(angle4 * 3.14159));
    vec2 rotateUV5 = vec2(cos(angle5 * 3.14159), sin(angle5 * 3.14159));
    
    // Create line patterns at varied angles and spacings
    // Vary the stroke density slightly for each set of lines
    float densityVar1 = u_stroke_density * (0.9 + noise1 * 0.2);
    float densityVar2 = u_stroke_density * (0.85 + noise2 * 0.3);
    float densityVar3 = u_stroke_density * (1.1 + noise3 * 0.2);
    float densityVar4 = u_stroke_density * (0.95 + noise1 * 0.15);
    float densityVar5 = u_stroke_density * (1.05 + noise2 * 0.25);
    
    // Calculate dot products for varied line patterns
    float dLine1 = 1.0 - lineMask(dot(screenPos, rotateUV1) * densityVar1, 
                              u_line_thickness * (0.9 + noise1 * 0.2), screenPos * 1.1);
    float dLine2 = 1.0 - lineMask(dot(screenPos, rotateUV2) * densityVar2, 
                              u_line_thickness * (0.85 + noise2 * 0.3), screenPos * 2.3);
    float dLine3 = 1.0 - lineMask(dot(screenPos, rotateUV3) * densityVar3, 
                              u_line_thickness * (1.1 + noise3 * 0.15), screenPos * 3.7);
    float dLine4 = 1.0 - lineMask(dot(screenPos, rotateUV4) * densityVar4, 
                              u_line_thickness * (0.8 + noise1 * 0.4), screenPos * 5.2);
    float dLine5 = 1.0 - lineMask(dot(screenPos, rotateUV5) * densityVar5, 
                              u_line_thickness * (1.05 + noise2 * 0.25), screenPos * 6.1);
    
    // Add occasional disruptive strokes in different directions at random locations
    vec2 distortedPos = screenPos + vec2(noise3 - 0.5, noise1 - 0.5) * u_stroke_randomness * 0.15;
    float disruption = 0.0;
    
    // Create random disruption strokes only in certain areas
    if(noise1 > 0.85) {
        float randAngle = noise2 * 3.14159 * 2.0; // Random direction
        vec2 disruptDir = vec2(cos(randAngle), sin(randAngle));
        disruption = 1.0 - lineMask(dot(distortedPos, disruptDir) * u_stroke_density * 1.2, 
                                u_line_thickness * 1.2, screenPos * 7.3);
    }
    
    // Start with white (no hatches)
    float strokeFactor = 1.0;
    
    // Apply hatching based on enhanced diffuse factor for more pronounced gradient
    // Adjust thresholds for faster falloff
    float thresh1 = 0.98;  // Almost white areas still get very light hatching
    float thresh2 = 0.7;   // Higher threshold for second layer
    float thresh3 = 0.4;   // Medium threshold
    float thresh4 = 0.2;   // Dark areas threshold
    
    // First hatching layer - even in very light areas
    if (enhancedDiffuse < thresh1) {
        float mixAmount = smoothstep(thresh1, thresh1 - 0.3, enhancedDiffuse) * u_hatching_opacity;
        strokeFactor = mix(strokeFactor, dLine1, mixAmount);
    }
    
    // Second hatching layer - light to medium areas
    if (enhancedDiffuse < thresh2) {
        float mixAmount = smoothstep(thresh2, thresh2 - 0.25, enhancedDiffuse) * u_hatching_opacity;
        // Mix between similar angle lines rather than perpendicular
        float lineChoice = noise1 > 0.5 ? dLine1 : dLine3;
        strokeFactor = mix(strokeFactor, min(lineChoice, dLine5), mixAmount * 1.2);
    }
    
    // Third hatching layer - medium areas
    if (enhancedDiffuse < thresh3) {
        float mixAmount = smoothstep(thresh3, thresh3 - 0.2, enhancedDiffuse) * u_hatching_opacity;
        // Vary which lines are used based on noise - more organic combinations
        if (noise1 > 0.7) {
            strokeFactor = mix(strokeFactor, min(min(dLine1, dLine3), dLine5), mixAmount * 1.4);
        } else if (noise1 > 0.3) {
            strokeFactor = mix(strokeFactor, min(min(dLine2, dLine5), dLine3), mixAmount * 1.4);
        } else {
            strokeFactor = mix(strokeFactor, min(min(dLine3, dLine4), dLine1), mixAmount * 1.4);
        }
    }
    
    // Fourth hatching layer - dark areas
    if (enhancedDiffuse < thresh4) {
        float mixAmount = smoothstep(thresh4, thresh4 - 0.15, enhancedDiffuse) * u_hatching_opacity;
        // Include disruption lines in darkest areas and vary which lines combine
        strokeFactor = mix(strokeFactor, min(min(min(dLine1, dLine3), dLine5), 
                         noise1 > 0.7 ? dLine2 : dLine4) * (1.0 - disruption * 0.3), mixAmount * 1.6);
    }
    
    // Add occasional thicker, bolder sketch lines for emphasis
    if (noise3 > 0.92 && noise1 < 0.4) {
        float boldLine = 1.0 - lineMask(dot(screenPos, rotateUV1) * u_stroke_density * 0.5, 
                                    u_line_thickness * 3.0, screenPos * 0.7);
        strokeFactor = min(strokeFactor, boldLine);
    }
    
    // Add paper texture effect
    float paperGrain = texture(u_noise_texture, screenPos * 50.0).r;
    strokeFactor = mix(strokeFactor, strokeFactor * (0.9 + 0.1 * paperGrain), 0.3);
    
    // Add smudge-like effect in random areas for more natural look
    if (noise2 > 0.85 && noise1 < 0.5) {
        float smudge = texture(u_noise_texture, screenPos * 9.0 + vec2(noise3, noise1)).r;
        strokeFactor = mix(strokeFactor, strokeFactor * 0.7 + 0.1 * smudge, 0.4 * noise3);
    }
    
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
    
    // Add subtle value variation across the drawing
    float valueVar = texture(u_noise_texture, screenPos * 3.5).r * 0.04;
    tonedColor = tonedColor * (0.98 + valueVar);
    
    // Combine hatching with grayscale toning - reduce toning strength for more prominent hatching
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