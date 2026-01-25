uniform vec2 resolution;
uniform float time;

float random(vec2 st) {
    st /= 4.0;
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453);
}

float noise(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float smoothNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(noise(i), noise(i + vec2(1.0, 0.0)), u.x), mix(noise(i + vec2(0.0, 1.0)), noise(i + vec2(1.0, 1.0)), u.x), u.y);
}

float fbm(vec2 p) {
    float total = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    for (int i = 0; i < 3; i++) {
        total += amplitude * smoothNoise(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return total;
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;

    uv.x += sin((time * 3.0) + (uv.y * 100.0)) / 400.0;
    vec2 fragcoord = uv * resolution.xy;

    vec3 colour = vec3(0.7, 0.2, 0.0);
    vec3 void_colour = vec3(0.0, 0.0, 0.0);
    colour *= 1.0 - uv.y;

    float noise = fbm((uv * 3.0) + (time / 20.0));
    noise += fbm(vec2(uv.y * 3.0, uv.x * 3.0) + (time / 10.0));
    float new_noise = noise;
    float init_noise = noise;

    if (noise < 0.8) {
        if (noise > 0.799) colour = vec3(1.0, 1.0, 1.0);
        else colour = void_colour;
    } else {
        for (int i = 0; i < 2; i++) {
            new_noise *= new_noise + 0.35;
        }
        colour /= clamp(new_noise - 1.0, 0.0, 1.6);
    }
    noise += random(gl_FragCoord.xy) / 300.0;
    if (noise < 0.9 && noise > 0.85) {
        colour = mix(colour, void_colour, 0.5);
    }
    noise += random(gl_FragCoord.xy) / 200.0;
    if (noise < 0.96 && noise > 0.93) {
        colour = mix(colour, void_colour, 0.2);
    }
    noise += random(gl_FragCoord.xy) / 200.0;
    if (noise < 0.98 && noise > 0.97) {
        colour = mix(colour, void_colour, 0.07);
    }

    colour.r += fbm((uv * 10.0) + (time / 3.0)) / 3.0;
    colour.g += fbm((uv * 11.2) + (time / 5.4)) / 20.0;

    vec2 cell = floor(fragcoord / 3.0) * 3.0;
    if (random(cell) + random(-cell * 2.0) > 1.93) {
        colour += (random(cell) / 22.0);
    }

    float rand = random(gl_FragCoord.xy) / 340.0;
    float particles = random(floor(vec2((uv.x * 4.0) + rand, ((uv.y + rand) / 10.0) - (time / 100.0)) * 100.0));
    if (particles > 0.98) {
        colour += 0.01 * particles;
    }

    float noise_range = 0.75 + (sin(time / 3.0) / 2.0);

    if (init_noise < noise_range + 0.01 && init_noise > noise_range - 0.01) {
        colour += vec3(0.3, 0.1, 0.0) / 10.0;
    }

    float cellSize = 80.0;

    vec2 rounded_uv = (floor(gl_FragCoord.xy / cellSize) + 0.5) * cellSize;

    float dotsize = 16.0 + (sin(((rounded_uv.x + rounded_uv.y) * 30.0) + time) * 16.0);

    float dist = distance(gl_FragCoord.xy, rounded_uv);
    if (dist < dotsize) {
        colour -= 0.016;
    }

    gl_FragColor = vec4(colour, 1.0);
}