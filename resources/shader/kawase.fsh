uniform vec2 resolution;
uniform sampler2D sprite0;
uniform float time;
uniform int currentPass;

varying vec2 v_texCoord;

vec4 kawaseBlur(sampler2D tex, vec2 uv, vec2 pixelSize, float offset) {
    vec4 color = vec4(0.0);
    color += texture2D(tex, uv + vec2(offset, offset) * pixelSize);
    color += texture2D(tex, uv + vec2(-offset, offset) * pixelSize);
    color += texture2D(tex, uv + vec2(offset, -offset) * pixelSize);
    color += texture2D(tex, uv + vec2(-offset, -offset) * pixelSize);
    return color * 0.25;
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    vec2 pixelSize = 1.0 / resolution;

    float offset = float(currentPass) + 0.5;
    vec4 color = kawaseBlur(sprite0, uv, pixelSize, offset);

    gl_FragColor = color;
}