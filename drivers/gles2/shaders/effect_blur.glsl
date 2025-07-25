/* clang-format off */
[vertex]

#ifdef USE_GLES_OVER_GL
#define lowp
#define mediump
#define highp
#else
precision highp float;
precision highp int;
#endif

attribute vec2 vertex_attrib; // attrib:0
/* clang-format on */
attribute vec2 uv_in; // attrib:4

varying vec2 uv_interp;

#ifdef USE_BLUR_SECTION

uniform vec4 blur_section;

#endif

void main() {
	uv_interp = uv_in;
	gl_Position = vec4(vertex_attrib, 0.0, 1.0);
#ifdef USE_BLUR_SECTION

	uv_interp = blur_section.xy + uv_interp * blur_section.zw;
	gl_Position.xy = (blur_section.xy + (gl_Position.xy * 0.5 + 0.5) * blur_section.zw) * 2.0 - 1.0;
#endif
}

/* clang-format off */
[fragment]

// texture2DLodEXT and textureCubeLodEXT are fragment shader specific.
// Do not copy these defines in the vertex section.
#ifndef USE_GLES_OVER_GL
#ifdef GL_EXT_shader_texture_lod
#extension GL_EXT_shader_texture_lod : enable
#define texture2DLod(img, coord, lod) texture2DLodEXT(img, coord, lod)
#define textureCubeLod(img, coord, lod) textureCubeLodEXT(img, coord, lod)
#endif
#endif // !USE_GLES_OVER_GL

#ifdef GL_ARB_shader_texture_lod
#extension GL_ARB_shader_texture_lod : enable
#endif

#if !defined(GL_EXT_shader_texture_lod) && !defined(GL_ARB_shader_texture_lod)
#define texture2DLod(img, coord, lod) texture2D(img, coord, lod)
#define textureCubeLod(img, coord, lod) textureCube(img, coord, lod)
#endif

#ifdef USE_GLES_OVER_GL
#define lowp
#define mediump
#define highp
#else
#if defined(USE_HIGHP_PRECISION)
precision highp float;
precision highp int;
#else
precision mediump float;
precision mediump int;
#endif
#endif

varying vec2 uv_interp;
/* clang-format on */
uniform sampler2D source_color; //texunit:0

uniform float lod;
uniform vec2 pixel_size;

#if defined(GLOW_GAUSSIAN_HORIZONTAL) || defined(GLOW_GAUSSIAN_VERTICAL)

uniform float glow_strength;

#endif

#if defined(DOF_FAR_BLUR) || defined(DOF_NEAR_BLUR)

#ifdef USE_GLES_OVER_GL
#ifdef DOF_QUALITY_LOW
const int dof_kernel_size = 5;
const int dof_kernel_from = 2;
const float dof_kernel[5] = float[](0.153388, 0.221461, 0.250301, 0.221461, 0.153388);
#endif

#ifdef DOF_QUALITY_MEDIUM
const int dof_kernel_size = 11;
const int dof_kernel_from = 5;
const float dof_kernel[11] = float[](0.055037, 0.072806, 0.090506, 0.105726, 0.116061, 0.119726, 0.116061, 0.105726, 0.090506, 0.072806, 0.055037);

#endif

#ifdef DOF_QUALITY_HIGH
const int dof_kernel_size = 21;
const int dof_kernel_from = 10;
const float dof_kernel[21] = float[](0.028174, 0.032676, 0.037311, 0.041944, 0.046421, 0.050582, 0.054261, 0.057307, 0.059587, 0.060998, 0.061476, 0.060998, 0.059587, 0.057307, 0.054261, 0.050582, 0.046421, 0.041944, 0.037311, 0.032676, 0.028174);
#endif
#endif

uniform sampler2D dof_source_depth; //texunit:1
uniform float dof_begin;
uniform float dof_end;
uniform vec2 dof_dir;
uniform float dof_radius;

#endif

#ifdef GLOW_FIRST_PASS

uniform highp float luminance_cap;

uniform float glow_bloom;
uniform float glow_hdr_threshold;
uniform float glow_hdr_scale;

#endif

uniform float camera_z_far;
uniform float camera_z_near;

void main() {
#ifdef GLOW_GAUSSIAN_HORIZONTAL
	vec2 pix_size = pixel_size;
	pix_size *= 0.5; //reading from larger buffer, so use more samples
	vec4 color = texture2DLod(source_color, uv_interp + vec2(0.0, 0.0) * pix_size, lod) * 0.174938;
	color += texture2DLod(source_color, uv_interp + vec2(1.0, 0.0) * pix_size, lod) * 0.165569;
	color += texture2DLod(source_color, uv_interp + vec2(2.0, 0.0) * pix_size, lod) * 0.140367;
	color += texture2DLod(source_color, uv_interp + vec2(3.0, 0.0) * pix_size, lod) * 0.106595;
	color += texture2DLod(source_color, uv_interp + vec2(-1.0, 0.0) * pix_size, lod) * 0.165569;
	color += texture2DLod(source_color, uv_interp + vec2(-2.0, 0.0) * pix_size, lod) * 0.140367;
	color += texture2DLod(source_color, uv_interp + vec2(-3.0, 0.0) * pix_size, lod) * 0.106595;
	color *= glow_strength;
	gl_FragColor = color;
#endif

#ifdef GLOW_GAUSSIAN_VERTICAL
	vec4 color = texture2DLod(source_color, uv_interp + vec2(0.0, 0.0) * pixel_size, lod) * 0.288713;
	color += texture2DLod(source_color, uv_interp + vec2(0.0, 1.0) * pixel_size, lod) * 0.233062;
	color += texture2DLod(source_color, uv_interp + vec2(0.0, 2.0) * pixel_size, lod) * 0.122581;
	color += texture2DLod(source_color, uv_interp + vec2(0.0, -1.0) * pixel_size, lod) * 0.233062;
	color += texture2DLod(source_color, uv_interp + vec2(0.0, -2.0) * pixel_size, lod) * 0.122581;
	color *= glow_strength;
	gl_FragColor = color;
#endif

#ifndef USE_GLES_OVER_GL
#if defined(DOF_FAR_BLUR) || defined(DOF_NEAR_BLUR)

#ifdef DOF_QUALITY_LOW
	const int dof_kernel_size = 5;
	const int dof_kernel_from = 2;
	float dof_kernel[5];
	dof_kernel[0] = 0.153388;
	dof_kernel[1] = 0.221461;
	dof_kernel[2] = 0.250301;
	dof_kernel[3] = 0.221461;
	dof_kernel[4] = 0.153388;
#endif

#ifdef DOF_QUALITY_MEDIUM
	const int dof_kernel_size = 11;
	const int dof_kernel_from = 5;
	float dof_kernel[11];
	dof_kernel[0] = 0.055037;
	dof_kernel[1] = 0.072806;
	dof_kernel[2] = 0.090506;
	dof_kernel[3] = 0.105726;
	dof_kernel[4] = 0.116061;
	dof_kernel[5] = 0.119726;
	dof_kernel[6] = 0.116061;
	dof_kernel[7] = 0.105726;
	dof_kernel[8] = 0.090506;
	dof_kernel[9] = 0.072806;
	dof_kernel[10] = 0.055037;
#endif

#ifdef DOF_QUALITY_HIGH
	const int dof_kernel_size = 21;
	const int dof_kernel_from = 10;
	float dof_kernel[21];
	dof_kernel[0] = 0.028174;
	dof_kernel[1] = 0.032676;
	dof_kernel[2] = 0.037311;
	dof_kernel[3] = 0.041944;
	dof_kernel[4] = 0.046421;
	dof_kernel[5] = 0.050582;
	dof_kernel[6] = 0.054261;
	dof_kernel[7] = 0.057307;
	dof_kernel[8] = 0.059587;
	dof_kernel[9] = 0.060998;
	dof_kernel[10] = 0.061476;
	dof_kernel[11] = 0.060998;
	dof_kernel[12] = 0.059587;
	dof_kernel[13] = 0.057307;
	dof_kernel[14] = 0.054261;
	dof_kernel[15] = 0.050582;
	dof_kernel[16] = 0.046421;
	dof_kernel[17] = 0.041944;
	dof_kernel[18] = 0.037311;
	dof_kernel[19] = 0.032676;
	dof_kernel[20] = 0.028174;
#endif
#endif
#endif //!USE_GLES_OVER_GL

#ifdef DOF_FAR_BLUR

	vec4 color_accum = vec4(0.0);

	float depth = texture2DLod(dof_source_depth, uv_interp, 0.0).r;
	depth = depth * 2.0 - 1.0;
#ifdef USE_ORTHOGONAL_PROJECTION
	depth = ((depth + (camera_z_far + camera_z_near) / (camera_z_far - camera_z_near)) * (camera_z_far - camera_z_near)) / 2.0;
#else
	depth = 2.0 * camera_z_near * camera_z_far / (camera_z_far + camera_z_near - depth * (camera_z_far - camera_z_near));
#endif

	float amount = smoothstep(dof_begin, dof_end, depth);
	vec4 k_accum = vec4(0.0);

	for (int i = 0; i < dof_kernel_size; i++) {
		int int_ofs = i - dof_kernel_from;
		vec2 tap_uv = uv_interp + dof_dir * float(int_ofs) * amount * dof_radius;

		float tap_k = dof_kernel[i];

		float tap_depth = texture2D(dof_source_depth, tap_uv, 0.0).r;
		tap_depth = tap_depth * 2.0 - 1.0;
#ifdef USE_ORTHOGONAL_PROJECTION
		tap_depth = ((tap_depth + (camera_z_far + camera_z_near) / (camera_z_far - camera_z_near)) * (camera_z_far - camera_z_near)) / 2.0;
#else
		tap_depth = 2.0 * camera_z_near * camera_z_far / (camera_z_far + camera_z_near - tap_depth * (camera_z_far - camera_z_near));
#endif
		float tap_amount = int_ofs == 0 ? 1.0 : smoothstep(dof_begin, dof_end, tap_depth);
		tap_amount *= tap_amount * tap_amount; //prevent undesired glow effect
		tap_amount *= tap_k;

		vec4 tap_color = texture2DLod(source_color, tap_uv, 0.0);

		vec4 w = vec4(tap_amount) * vec4(vec3(tap_color.a), 1.0);
		k_accum += w;
		color_accum += tap_color * w;
	}

	if (k_accum.r > 0.0) {
		color_accum /= k_accum;
	}

	gl_FragColor = color_accum; ///k_accum;

#endif

#ifdef DOF_NEAR_BLUR

	vec4 color_accum = vec4(0.0);

	float max_accum = 0.0;
	float k_accum = 0.0;

	for (int i = 0; i < dof_kernel_size; i++) {
		int int_ofs = i - dof_kernel_from;
		vec2 tap_uv = uv_interp + dof_dir * float(int_ofs) * dof_radius;
		float ofs_influence = max(0.0, 1.0 - abs(float(int_ofs)) / float(dof_kernel_from));

		float tap_k = dof_kernel[i];

		vec4 tap_color = texture2DLod(source_color, tap_uv, 0.0);
		float w = tap_color.a;

		float tap_depth = texture2D(dof_source_depth, tap_uv, 0.0).r;
		tap_depth = tap_depth * 2.0 - 1.0;
#ifdef USE_ORTHOGONAL_PROJECTION
		tap_depth = ((tap_depth + (camera_z_far + camera_z_near) / (camera_z_far - camera_z_near)) * (camera_z_far - camera_z_near)) / 2.0;
#else
		tap_depth = 2.0 * camera_z_near * camera_z_far / (camera_z_far + camera_z_near - tap_depth * (camera_z_far - camera_z_near));
#endif
		float tap_amount = 1.0 - smoothstep(dof_end, dof_begin, tap_depth);
		tap_amount *= tap_amount * tap_amount; //prevent undesired glow effect

#ifdef DOF_NEAR_FIRST_TAP

		tap_color.a = 1.0 - smoothstep(dof_end, dof_begin, tap_depth);

#endif

		max_accum = max(max_accum, tap_amount * ofs_influence);

		k_accum += w;
		tap_color.rgb *= w;
		color_accum += tap_color * tap_k;
	}

	if (k_accum > 0.0) {
		color_accum.rgb /= k_accum / dof_kernel_size;
	}
	color_accum.a = max(color_accum.a, sqrt(max_accum));

	gl_FragColor = color_accum;

#endif

#ifdef GLOW_FIRST_PASS

	float luminance = max(gl_FragColor.r, max(gl_FragColor.g, gl_FragColor.b));
	float feedback = max(smoothstep(glow_hdr_threshold, glow_hdr_threshold + glow_hdr_scale, luminance), glow_bloom);

	gl_FragColor = min(gl_FragColor * feedback, vec4(luminance_cap));

#endif
}
