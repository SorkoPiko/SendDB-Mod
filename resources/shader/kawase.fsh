uniform vec2 resolution;
uniform vec4 screenRect;
uniform sampler2D sprite0;
uniform float time;
uniform int currentPass;

varying vec2 v_texCoord;

const float edgeSoftness = 50.0;

vec4 kawaseBlur(sampler2D tex, vec2 uv, vec2 pixelSize, float offset) {
    vec4 color = vec4(0.0);
    color += texture2D(tex, uv + vec2(offset, offset) * pixelSize);
    color += texture2D(tex, uv + vec2(-offset, offset) * pixelSize);
    color += texture2D(tex, uv + vec2(offset, -offset) * pixelSize);
    color += texture2D(tex, uv + vec2(-offset, -offset) * pixelSize);
    return color * 0.25;
}

float rectMask(vec2 fragCoord, vec4 rect, float softness) {
    vec2 rectMin = rect.xy;
    vec2 rectMax = rect.xy + rect.zw;

    vec2 distFromEdge = max(rectMin - fragCoord, fragCoord - rectMax);
    float dist = max(distFromEdge.x, distFromEdge.y);

    return 1.0 - smoothstep(-softness, 0.0, dist);
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    vec2 pixelSize = 1.0 / resolution;

    float offset = float(currentPass) + 0.5;
    vec4 blurred = kawaseBlur(sprite0, uv, pixelSize, offset);

    float mask = rectMask(gl_FragCoord.xy, screenRect, edgeSoftness);

    vec4 original = texture2D(sprite0, uv);
    gl_FragColor = mix(original, blurred, mask);
}