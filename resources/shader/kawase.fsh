uniform vec2 resolution;
uniform vec4 screenRect;
uniform sampler2D sprite0;
uniform float time;
uniform int currentPass;

varying vec2 v_texCoord;

const float edgeSoftness = 50.0;

vec4 kawaseBlur(sampler2D tex, vec2 uv, vec2 pixelSize, float offset) {
    vec4 blur = vec4(0.0);
    blur += texture2D(tex, uv + vec2(offset, offset) * pixelSize);
    blur += texture2D(tex, uv + vec2(-offset, offset) * pixelSize);
    blur += texture2D(tex, uv + vec2(offset, -offset) * pixelSize);
    blur += texture2D(tex, uv + vec2(-offset, -offset) * pixelSize);
    return blur * 0.25;
}

float rectMask(vec2 p, vec4 r, float softness) {
    vec2 d = max(r.xy - p, p - (r.xy + r.zw));
    return 1.0 - smoothstep(-softness, 0.0, max(d.x, d.y));
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    vec2 pixelSize = 1.0 / resolution;
    
    vec4 colour = texture2D(sprite0, uv);

    float offset = float(currentPass) + 0.5;
    float mask = rectMask(gl_FragCoord.xy, screenRect, edgeSoftness);

    if (mask != 0.0) {
        colour = kawaseBlur(sprite0, uv, pixelSize, offset);
    }

    gl_FragColor = colour;
}