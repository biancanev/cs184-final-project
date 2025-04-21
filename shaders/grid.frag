#version 330 core

in vec3 worldPos;

out vec4 FragColor;

uniform vec3 gridColor = vec3(0.2, 0.2, 0.2);
uniform float gridSpacing = 1.0;
uniform float lineWidth = 1.0;  // Thickness in pixels

void main() {
    // Scale world position to grid space
    vec2 coord = worldPos.xz / gridSpacing;

    // Compute anti-aliased grid lines
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
    float line = min(grid.x, grid.y);
    float gridLine = 1.0 - smoothstep(0.0, lineWidth, line);

    // Output the grid color with alpha based on visibility
    FragColor = vec4(gridColor, gridLine);
}
