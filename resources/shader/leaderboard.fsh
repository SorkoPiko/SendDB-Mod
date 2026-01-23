uniform vec2 resolution;
uniform float time;

float random(vec2 st) {
	st /= 4;
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453);
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
	
	vec3 colour = vec3(0.1, 0.3, 0.07); // base colour
	float cell_size = 50; // square max size
	float dither = 1; // dither strength
	vec3 void_colour = vec3(0, 0, 0); // black ink-y coloured stuff

	colour.b += fbm(uv + (time / 10)) / 3;

	float noise = fbm(uv * 10 + (time * 0.1)) + fbm(-uv * 2 + (time * 0.0763342)); 
	
	vec2 cellpos = vec2(gl_FragCoord.x, gl_FragCoord.y - (sin(uv.x + time) * 50));

	vec2 cell = floor(cellpos / cell_size) * cell_size;
	cell.y += sin(uv.x + time) * 50;

	float square_size = sin((cell.x / 1.6) + time) * (cell_size / 2) + (cell_size / 2);
	
	vec2 dist = vec2(
		distance(cell.x, gl_FragCoord.x),
		distance(cell.y, gl_FragCoord.y)
	);

	float rand = random(uv) / ((0.03 * square_size) + 1);

	if (dist.x < square_size && dist.y < square_size) {
		noise += (rand / (100 * dither)) * (cell_size - square_size);

		if (noise > 0.8) colour += 0.04;

		if (noise < 0.8) colour /= 1.2;
		else colour *= 1.2;

		if (
			(dist.x > square_size - 2 || dist.y > square_size - 2) ||
			(dist.x < 2 || dist.y < 2)
		) {
			if (noise < 0.8) colour /= 1.2;
			else colour *= 1.2;
		}
	} else if (noise > 0.8) colour += 0.04;

	colour /= (uv.y - 0.24) / 3 + 1;

	float new_noise = fbm((uv * 3) + (time / 20));
	new_noise += fbm(vec2(uv.y * 3, uv.x * 3) + (time / 10));

	if (new_noise < 0.8) colour = void_colour;
	else {
		for (int i = 0; i < 2; i++) {
			new_noise *= new_noise + 0.35;
		}
		colour /= clamp(new_noise - 1, 0, 1.6);
	}

    gl_FragColor = vec4(colour, 1);
}