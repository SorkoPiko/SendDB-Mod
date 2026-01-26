// ported from https://github.com/OverShifted/LiquidGlass

uniform vec2 resolution;
uniform vec4 screenRect;
uniform sampler2D sprite0;
uniform float time;
uniform int currentPass;
uniform int passes;

varying vec2 v_texCoord;

#define M_E 2.718281828459045
#define M_TAU 6.28318530718

const float u_a = 0.7;
const float u_b = 2.3;
const float u_c = 5.2;
const float u_d = 6.9;

const float u_powerFactor = 3.0;
const float u_fPower = 3.0;
const float u_noise = 0.05;

const float u_glowWeight = 0.3;
const float u_glowBias = 0.0;
const float u_glowEdge0 = 0.06;
const float u_glowEdge1 = 0.0;

const float u_blurRadius = 1.4;

float sdSuperellipse(vec2 p, float n, float r) {
    vec2 p_abs = abs(p);

    float numerator = pow(p_abs.x, n) + pow(p_abs.y, n) - pow(r, n);

    float den_x = pow(p_abs.x, 2.0 * n - 2.0);
    float den_y = pow(p_abs.y, 2.0 * n - 2.0);
    float denominator = n * sqrt(den_x + den_y) + 0.00001;

    return numerator / denominator;
}

float f(float x) {
    return 1.0 - u_b * pow(u_c * M_E, -u_d * x - u_a);
}

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float glow(vec2 texCoord) {
    return sin(atan(texCoord.y * 2.0 - 1.0, texCoord.x * 2.0 - 1.0) - 0.5);
}

vec4 blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
    vec4 color = vec4(0.0);
    vec2 off1 = vec2(1.411764705882353) * direction;
    vec2 off2 = vec2(3.2941176470588234) * direction;
    vec2 off3 = vec2(5.176470588235294) * direction;

    color += texture2D(image, uv) * 0.1964825501511404;
    color += texture2D(image, uv + (off1 / resolution)) * 0.2969069646728344;
    color += texture2D(image, uv - (off1 / resolution)) * 0.2969069646728344;
    color += texture2D(image, uv + (off2 / resolution)) * 0.09447039785044732;
    color += texture2D(image, uv - (off2 / resolution)) * 0.09447039785044732;
    color += texture2D(image, uv + (off3 / resolution)) * 0.010381362401148057;
    color += texture2D(image, uv - (off3 / resolution)) * 0.010381362401148057;

    return color;
}

vec4 liquidGlassRefraction() {
    vec2 rectCenter = screenRect.xy + screenRect.zw * 0.5;
    vec2 rectSize = screenRect.zw;

    vec2 p = (gl_FragCoord.xy - rectCenter) / (rectSize * 0.5);

    float r = 1.0;
    float d = sdSuperellipse(p, u_powerFactor, r);

    if (d > 0.0) discard;

    float dist = -d;

    vec2 sampleP = p * pow(f(dist), u_fPower);
    vec2 samplePos = sampleP * (rectSize * 0.5) + rectCenter;
    vec2 coord = samplePos / resolution;

    if (max(coord.x, coord.y) > 1.0 || min(coord.x, coord.y) < 0.0)
    return vec4(1.0, 0.0, 1.0, 1.0);

    vec4 noise = vec4(vec3(rand(gl_FragCoord.xy * 1e-3) - 0.5), 0.0);
    vec4 color = texture2D(sprite0, coord) + noise * u_noise;

    float gray = dot(color.rgb, vec3(0.299, 0.213, 0.114));
    color.rgb = mix(color.rgb, vec3(gray), 0.2);

    color.rgb *= 1.1;

    vec2 glowCoord = p * 0.5 + 0.5;
    float mul = glow(glowCoord) * u_glowWeight * smoothstep(u_glowEdge0, u_glowEdge1, dist) + 1.0 + u_glowBias;

    return color * vec4(vec3(mul), 1.0);
}

vec4 passthrough() {
    vec2 rectCenter = screenRect.xy + screenRect.zw * 0.5;
    vec2 rectSize = screenRect.zw;

    vec2 p = (gl_FragCoord.xy - rectCenter) / (rectSize * 0.5);

    float r = 1.0;
    float d = sdSuperellipse(p, u_powerFactor, r);

    if (d > 0.0) discard;

    return texture2D(sprite0, gl_FragCoord.xy / resolution);
}

int mod(int a, int b) {
    return a - (a / b) * b;
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;

    int finalPass = passes - 1;
    bool ignoreLast = false;
    if (mod(passes, 2) == 0) {
        finalPass -= 1;
        ignoreLast = true;
    }

    if (currentPass == finalPass) {
        gl_FragColor = liquidGlassRefraction();
    } else if (ignoreLast && currentPass == finalPass + 1) {
        gl_FragColor = passthrough();
    } else if (mod(currentPass, 2) == 0) {
        gl_FragColor = blur13(sprite0, uv, resolution, vec2(u_blurRadius, 0.0));
    } else if (mod(currentPass, 2) == 1) {
        gl_FragColor = blur13(sprite0, uv, resolution, vec2(0.0, u_blurRadius));
    }
}