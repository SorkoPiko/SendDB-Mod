uniform vec2 resolution;
uniform float time;

uniform sampler2D sprite1;
uniform sampler2D sprite2;
uniform int sprites;

float random(vec2 st) {
	st /= 4.0;
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
	float cell_size = 50.0; // square max size
	float dither = 1.0; // dither strength
	vec3 void_colour = vec3(0.0, 0.0, 0.0); // black ink-y coloured stuff

	colour.b += fbm(uv + (time / 10.0)) / 3.0;

	float noise = fbm(uv * 10.0 + (time * 0.1)) + fbm(-uv * 2.0 + (time * 0.0763342));

	vec2 cellpos = vec2(gl_FragCoord.x, gl_FragCoord.y - (sin(uv.x + time) * 50.0));

	vec2 cell = floor(cellpos / cell_size) * cell_size;
	cell.y += sin(uv.x + time) * 50.0;

	float square_size = sin((cell.x / 1.6) + time) * (cell_size / 2.0) + (cell_size / 2.0);

	vec2 dist = vec2(
		distance(cell.x, gl_FragCoord.x),
		distance(cell.y, gl_FragCoord.y)
	);

	float rand = random(uv) / ((0.03 * square_size) + 1.0);

	if (dist.x < square_size && dist.y < square_size) {
		noise += (rand / (100.0 * dither)) * (cell_size - square_size);

		if (noise > 0.8) colour += 0.04;

		if (noise < 0.8) colour /= 1.2;
		else colour *= 1.2;

		if (
			(dist.x > square_size - 2.0 || dist.y > square_size - 2.0) ||
			(dist.x < 2.0 || dist.y < 2.0)
		) {
			if (noise < 0.8) colour /= 1.2;
			else colour *= 1.2;
		}
	} else if (noise > 0.8) colour += 0.04;

	colour /= (uv.y - 0.24) / 3.0 + 1.0;

	float new_noise = fbm((uv * 3.0) + (time / 20.0));
	new_noise += fbm(vec2(uv.y * 3.0, uv.x * 3.0) + (time / 10.0));

	vec2 random_pos1 = gl_FragCoord.xy;
	float multiplier = (sin(time + (uv.x * 1.5)) + 1.0) / 2.0;
	random_pos1.x = gl_FragCoord.x + random(uv) * 30.0 * multiplier;
	random_pos1.y += random(gl_FragCoord.xy) * 15.0 * multiplier;

	vec2 random_pos2 = gl_FragCoord.xy;
	random_pos2.x = gl_FragCoord.x + 450.0 + random(uv) * 30.0 * multiplier;
	random_pos2.y += random(gl_FragCoord.xy) * 15.0 * multiplier;

	vec2 icon_uv = vec2(
		fract((random_pos1.x + (time * 75.0)) / 900.0) * 2.0,
		1.0 - fract((random_pos1.y + (time * 25.0))/ 900.0) * 2.0
	);
	vec2 icon_uv2 = vec2(
		fract((random_pos2.x + (time * 75.0)) / 900.0) * 2.0,
		2.0 - fract((random_pos2.y + (time * 25.0)) / 900.0) * 2.0
	);

	if (new_noise < 0.8) {
		colour = void_colour;

		if ((icon_uv.x <= 1.0 && icon_uv.y <= 1.0) || (icon_uv2.x <= 1.0 && icon_uv2.y <= 1.0)) {
			if (sprites == 1) {
				vec4 tex1 = texture2D(sprite1, icon_uv);
				colour = vec3((tex1.r + tex1.g + tex1.b) / 6.0);
				if (colour.r > 0.1) {
					colour.g += 0.1;
					colour.b += 0.04;
				}

			} else if (sprites == 2) {
				vec4 tex1 = texture2D(sprite1, icon_uv);
				vec4 tex2 = texture2D(sprite2, icon_uv2);
				colour = vec3((tex1.r + tex1.g + tex1.b) / 6.0);
				colour += vec3((tex2.r + tex2.g + tex2.b) / 6.0);
				if (colour.r > 0.1) {
					colour.g += 0.1;
					colour.b += 0.04;
				}
			}
		}
	} else {
		if ((icon_uv.x <= 1.0 && icon_uv.y <= 1.0) || (icon_uv2.x <= 1.0 && icon_uv2.y <= 1.0)) {
			if (sprites == 1) {
				vec4 tex1 = texture2D(sprite1, icon_uv);
				if (tex1.r > 0.04 && tex1.a > 0.9) {
					colour /= 2.0;
					colour.g += 0.08;
					colour.b += 0.05;
				}
			} else if (sprites == 2) {
				vec4 tex1 = texture2D(sprite1, icon_uv);
				vec4 tex2 = texture2D(sprite2, icon_uv2);
				if (tex1.r > 0.04 && tex1.a > 0.9) {
					colour /= 2.0;
					colour.g += 0.08;
					colour.b += 0.05;
				}
				if (tex2.r > 0.04 && tex2.a > 0.9) {
					colour /= 2.0;
					colour.g += 0.08;
					colour.b += 0.05;
				}
			}
		}

		for (int i = 0; i < 2; i++) {
			new_noise *= new_noise + 0.35;
		}
		colour /= clamp(new_noise - 1.0, 0.0, 1.6);
	}

	gl_FragColor = vec4(colour, 1.0);
}