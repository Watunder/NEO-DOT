/*************************************************************************/
/*  cpu_particles.cpp                                                    */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-present Godot Engine contributors (cf. AUTHORS.md).*/
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "cpu_particles.h"

#include "scene/3d/camera.h"
#include "scene/3d/particles.h"
#include "scene/resources/particles_material.h"
#include "servers/visual_server.h"

AABB CPUParticles::get_aabb() const {
	return AABB();
}
PoolVector<Face3> CPUParticles::get_faces(uint32_t p_usage_flags) const {
	return PoolVector<Face3>();
}

void CPUParticles::set_emitting(bool p_emitting) {
	if (emitting == p_emitting)
		return;

	emitting = p_emitting;
	if (emitting) {
		set_process_internal(true);

		// first update before rendering to avoid one frame delay after emitting starts
		if (time == 0)
			_update_internal();
	}
}

void CPUParticles::set_amount(int p_amount) {
	ERR_FAIL_COND_MSG(p_amount < 1, "Amount of particles must be greater than 0.");

	particles.resize(p_amount);
	{
		PoolVector<Particle>::Write w = particles.write();

		for (int i = 0; i < p_amount; i++) {
			w[i].active = false;
			w[i].custom[3] = 0.0; // Make sure w component isn't garbage data
		}
	}

	particle_data.resize((12 + 4 + 1) * p_amount);
	VS::get_singleton()->multimesh_allocate(multimesh, p_amount, VS::MULTIMESH_TRANSFORM_3D, VS::MULTIMESH_COLOR_8BIT, VS::MULTIMESH_CUSTOM_DATA_FLOAT);

	particle_order.resize(p_amount);
}
void CPUParticles::set_lifetime(float p_lifetime) {
	ERR_FAIL_COND_MSG(p_lifetime <= 0, "Particles lifetime must be greater than 0.");
	lifetime = p_lifetime;
}

void CPUParticles::set_one_shot(bool p_one_shot) {
	one_shot = p_one_shot;
}

void CPUParticles::set_pre_process_time(float p_time) {
	pre_process_time = p_time;
}
void CPUParticles::set_explosiveness_ratio(float p_ratio) {
	explosiveness_ratio = p_ratio;
}
void CPUParticles::set_randomness_ratio(float p_ratio) {
	randomness_ratio = p_ratio;
}
void CPUParticles::set_lifetime_randomness(float p_random) {
	lifetime_randomness = p_random;
}
void CPUParticles::set_use_local_coordinates(bool p_enable) {
	local_coords = p_enable;
}
void CPUParticles::set_speed_scale(float p_scale) {
	speed_scale = p_scale;
}

bool CPUParticles::is_emitting() const {
	return emitting;
}
int CPUParticles::get_amount() const {
	return particles.size();
}
float CPUParticles::get_lifetime() const {
	return lifetime;
}
bool CPUParticles::get_one_shot() const {
	return one_shot;
}

float CPUParticles::get_pre_process_time() const {
	return pre_process_time;
}
float CPUParticles::get_explosiveness_ratio() const {
	return explosiveness_ratio;
}
float CPUParticles::get_randomness_ratio() const {
	return randomness_ratio;
}
float CPUParticles::get_lifetime_randomness() const {
	return lifetime_randomness;
}

bool CPUParticles::get_use_local_coordinates() const {
	return local_coords;
}

float CPUParticles::get_speed_scale() const {
	return speed_scale;
}

void CPUParticles::set_draw_order(DrawOrder p_order) {
	ERR_FAIL_INDEX(p_order, DRAW_ORDER_MAX);
	draw_order = p_order;
}

CPUParticles::DrawOrder CPUParticles::get_draw_order() const {
	return draw_order;
}

void CPUParticles::set_mesh(const Ref<Mesh> &p_mesh) {
	mesh = p_mesh;
	if (mesh.is_valid()) {
		VS::get_singleton()->multimesh_set_mesh(multimesh, mesh->get_rid());
	} else {
		VS::get_singleton()->multimesh_set_mesh(multimesh, RID());
	}
}

Ref<Mesh> CPUParticles::get_mesh() const {
	return mesh;
}

void CPUParticles::set_fixed_fps(int p_count) {
	fixed_fps = p_count;
}

int CPUParticles::get_fixed_fps() const {
	return fixed_fps;
}

void CPUParticles::set_fractional_delta(bool p_enable) {
	fractional_delta = p_enable;
}

bool CPUParticles::get_fractional_delta() const {
	return fractional_delta;
}

String CPUParticles::get_configuration_warning() const {
	String warnings = GeometryInstance::get_configuration_warning();

	bool mesh_found = false;
	bool anim_material_found = false;

	if (get_mesh().is_valid()) {
		mesh_found = true;
		for (int j = 0; j < get_mesh()->get_surface_count(); j++) {
			anim_material_found = Object::cast_to<ShaderMaterial>(get_mesh()->surface_get_material(j).ptr()) != NULL;
			SpatialMaterial *spat = Object::cast_to<SpatialMaterial>(get_mesh()->surface_get_material(j).ptr());
			anim_material_found = anim_material_found || (spat && spat->get_billboard_mode() == SpatialMaterial::BILLBOARD_PARTICLES);
		}
	}

	anim_material_found = anim_material_found || Object::cast_to<ShaderMaterial>(get_material_override().ptr()) != NULL;
	SpatialMaterial *spat = Object::cast_to<SpatialMaterial>(get_material_override().ptr());
	anim_material_found = anim_material_found || (spat && spat->get_billboard_mode() == SpatialMaterial::BILLBOARD_PARTICLES);

	if (!mesh_found) {
		if (warnings != String())
			warnings += "\n";
		warnings += "- " + TTR("Nothing is visible because no mesh has been assigned.");
	}

	if (!anim_material_found && (get_param(PARAM_ANIM_SPEED) != 0.0 || get_param(PARAM_ANIM_OFFSET) != 0.0 || get_param_curve(PARAM_ANIM_SPEED).is_valid() || get_param_curve(PARAM_ANIM_OFFSET).is_valid())) {
		if (warnings != String())
			warnings += "\n";
		warnings += "- " + TTR("CPUParticles animation requires the usage of a SpatialMaterial whose Billboard Mode is set to \"Particle Billboard\".");
	}

	return warnings;
}

void CPUParticles::restart() {
	time = 0;
	inactive_time = 0;
	frame_remainder = 0;
	cycle = 0;
	emitting = false;

	{
		int pc = particles.size();
		PoolVector<Particle>::Write w = particles.write();

		for (int i = 0; i < pc; i++) {
			w[i].active = false;
		}
	}

	set_emitting(true);
}

void CPUParticles::set_direction(Vector3 p_direction) {
	direction = p_direction;
}

Vector3 CPUParticles::get_direction() const {
	return direction;
}

void CPUParticles::set_spread(float p_spread) {
	spread = p_spread;
}

float CPUParticles::get_spread() const {
	return spread;
}

void CPUParticles::set_flatness(float p_flatness) {
	flatness = p_flatness;
}
float CPUParticles::get_flatness() const {
	return flatness;
}

void CPUParticles::set_param(Parameter p_param, float p_value) {
	ERR_FAIL_INDEX(p_param, PARAM_MAX);

	parameters[p_param] = p_value;
}
float CPUParticles::get_param(Parameter p_param) const {
	ERR_FAIL_INDEX_V(p_param, PARAM_MAX, 0);

	return parameters[p_param];
}

void CPUParticles::set_param_randomness(Parameter p_param, float p_value) {
	ERR_FAIL_INDEX(p_param, PARAM_MAX);

	randomness[p_param] = p_value;
}
float CPUParticles::get_param_randomness(Parameter p_param) const {
	ERR_FAIL_INDEX_V(p_param, PARAM_MAX, 0);

	return randomness[p_param];
}

static void _adjust_curve_range(const Ref<Curve> &p_curve, float p_min, float p_max) {
	Ref<Curve> curve = p_curve;
	if (!curve.is_valid())
		return;

	curve->ensure_default_setup(p_min, p_max);
}

void CPUParticles::set_param_curve(Parameter p_param, const Ref<Curve> &p_curve) {
	ERR_FAIL_INDEX(p_param, PARAM_MAX);

	curve_parameters[p_param] = p_curve;

	switch (p_param) {
		case PARAM_INITIAL_LINEAR_VELOCITY: {
			//do none for this one
		} break;
		case PARAM_ANGULAR_VELOCITY: {
			_adjust_curve_range(p_curve, -360, 360);
		} break;
		case PARAM_ORBIT_VELOCITY: {
			_adjust_curve_range(p_curve, -500, 500);
		} break;
		case PARAM_LINEAR_ACCEL: {
			_adjust_curve_range(p_curve, -200, 200);
		} break;
		case PARAM_RADIAL_ACCEL: {
			_adjust_curve_range(p_curve, -200, 200);
		} break;
		case PARAM_TANGENTIAL_ACCEL: {
			_adjust_curve_range(p_curve, -200, 200);
		} break;
		case PARAM_DAMPING: {
			_adjust_curve_range(p_curve, 0, 100);
		} break;
		case PARAM_ANGLE: {
			_adjust_curve_range(p_curve, -360, 360);
		} break;
		case PARAM_SCALE: {
		} break;
		case PARAM_HUE_VARIATION: {
			_adjust_curve_range(p_curve, -1, 1);
		} break;
		case PARAM_ANIM_SPEED: {
			_adjust_curve_range(p_curve, 0, 200);
		} break;
		case PARAM_ANIM_OFFSET: {
		} break;
		default: {
		}
	}
}
Ref<Curve> CPUParticles::get_param_curve(Parameter p_param) const {
	ERR_FAIL_INDEX_V(p_param, PARAM_MAX, Ref<Curve>());

	return curve_parameters[p_param];
}

void CPUParticles::set_color(const Color &p_color) {
	color = p_color;
}

Color CPUParticles::get_color() const {
	return color;
}

void CPUParticles::set_color_ramp(const Ref<Gradient> &p_ramp) {
	color_ramp = p_ramp;
}

Ref<Gradient> CPUParticles::get_color_ramp() const {
	return color_ramp;
}

void CPUParticles::set_particle_flag(Flags p_flag, bool p_enable) {
	ERR_FAIL_INDEX(p_flag, FLAG_MAX);
	flags[p_flag] = p_enable;
	if (p_flag == FLAG_DISABLE_Z) {
		_change_notify();
	}
}

bool CPUParticles::get_particle_flag(Flags p_flag) const {
	ERR_FAIL_INDEX_V(p_flag, FLAG_MAX, false);
	return flags[p_flag];
}

void CPUParticles::set_emission_shape(EmissionShape p_shape) {
	ERR_FAIL_INDEX(p_shape, EMISSION_SHAPE_MAX);
	emission_shape = p_shape;
}

void CPUParticles::set_emission_sphere_radius(float p_radius) {
	emission_sphere_radius = p_radius;
}

void CPUParticles::set_emission_box_extents(Vector3 p_extents) {
	emission_box_extents = p_extents;
}

void CPUParticles::set_emission_points(const PoolVector<Vector3> &p_points) {
	emission_points = p_points;
}

void CPUParticles::set_emission_normals(const PoolVector<Vector3> &p_normals) {
	emission_normals = p_normals;
}

void CPUParticles::set_emission_colors(const PoolVector<Color> &p_colors) {
	emission_colors = p_colors;
}

float CPUParticles::get_emission_sphere_radius() const {
	return emission_sphere_radius;
}
Vector3 CPUParticles::get_emission_box_extents() const {
	return emission_box_extents;
}
PoolVector<Vector3> CPUParticles::get_emission_points() const {
	return emission_points;
}
PoolVector<Vector3> CPUParticles::get_emission_normals() const {
	return emission_normals;
}

PoolVector<Color> CPUParticles::get_emission_colors() const {
	return emission_colors;
}

CPUParticles::EmissionShape CPUParticles::get_emission_shape() const {
	return emission_shape;
}
void CPUParticles::set_gravity(const Vector3 &p_gravity) {
	gravity = p_gravity;
}

Vector3 CPUParticles::get_gravity() const {
	return gravity;
}

void CPUParticles::_validate_property(PropertyInfo &property) const {
	if (property.name == "color" && color_ramp.is_valid()) {
		property.usage = 0;
	}

	if (property.name == "emission_sphere_radius" && emission_shape != EMISSION_SHAPE_SPHERE) {
		property.usage = 0;
	}

	if (property.name == "emission_box_extents" && emission_shape != EMISSION_SHAPE_BOX) {
		property.usage = 0;
	}

	if ((property.name == "emission_point_texture" || property.name == "emission_color_texture") && (emission_shape < EMISSION_SHAPE_POINTS)) {
		property.usage = 0;
	}

	if (property.name == "emission_normals" && emission_shape != EMISSION_SHAPE_DIRECTED_POINTS) {
		property.usage = 0;
	}

	if (property.name.begins_with("orbit_") && !flags[FLAG_DISABLE_Z]) {
		property.usage = 0;
	}
}

static uint32_t idhash(uint32_t x) {
	x = ((x >> uint32_t(16)) ^ x) * uint32_t(0x45d9f3b);
	x = ((x >> uint32_t(16)) ^ x) * uint32_t(0x45d9f3b);
	x = (x >> uint32_t(16)) ^ x;
	return x;
}

static float rand_from_seed(uint32_t &seed) {
	int k;
	int s = int(seed);
	if (s == 0)
		s = 305420679;
	k = s / 127773;
	s = 16807 * (s - k * 127773) - 2836 * k;
	if (s < 0)
		s += 2147483647;
	seed = uint32_t(s);
	return float(seed % uint32_t(65536)) / 65535.0;
}

void CPUParticles::_update_internal() {
	if (particles.size() == 0 || !is_visible_in_tree()) {
		_set_redraw(false);
		return;
	}

	float delta = get_process_delta_time();
	if (emitting) {
		inactive_time = 0;
	} else {
		inactive_time += delta;
		if (inactive_time > lifetime * 1.2) {
			set_process_internal(false);
			_set_redraw(false);

			//reset variables
			time = 0;
			inactive_time = 0;
			frame_remainder = 0;
			cycle = 0;
			return;
		}
	}
	_set_redraw(true);

	bool processed = false;

	if (time == 0 && pre_process_time > 0.0) {
		float frame_time;
		if (fixed_fps > 0)
			frame_time = 1.0 / fixed_fps;
		else
			frame_time = 1.0 / 30.0;

		float todo = pre_process_time;

		while (todo >= 0) {
			_particles_process(frame_time);
			processed = true;
			todo -= frame_time;
		}
	}

	if (fixed_fps > 0) {
		float frame_time = 1.0 / fixed_fps;
		float decr = frame_time;

		float ldelta = delta;
		if (ldelta > 0.1) { //avoid recursive stalls if fps goes below 10
			ldelta = 0.1;
		} else if (ldelta <= 0.0) { //unlikely but..
			ldelta = 0.001;
		}
		float todo = frame_remainder + ldelta;

		while (todo >= frame_time) {
			_particles_process(frame_time);
			processed = true;
			todo -= decr;
		}

		frame_remainder = todo;

	} else {
		_particles_process(delta);
		processed = true;
	}

	if (processed) {
		_update_particle_data_buffer();
	}
}

void CPUParticles::_particles_process(float p_delta) {
	p_delta *= speed_scale;

	int pcount = particles.size();
	PoolVector<Particle>::Write w = particles.write();

	Particle *parray = w.ptr();

	float prev_time = time;
	time += p_delta;
	if (time > lifetime) {
		time = Math::fmod(time, lifetime);
		cycle++;
		if (one_shot && cycle > 0) {
			set_emitting(false);
			_change_notify();
		}
	}

	Transform emission_xform;
	Basis velocity_xform;
	if (!local_coords) {
		emission_xform = get_global_transform();
		velocity_xform = emission_xform.basis;
	}

	float system_phase = time / lifetime;

	for (int i = 0; i < pcount; i++) {
		Particle &p = parray[i];

		if (!emitting && !p.active)
			continue;

		float local_delta = p_delta;

		// The phase is a ratio between 0 (birth) and 1 (end of life) for each particle.
		// While we use time in tests later on, for randomness we use the phase as done in the
		// original shader code, and we later multiply by lifetime to get the time.
		float restart_phase = float(i) / float(pcount);

		if (randomness_ratio > 0.0) {
			uint32_t seed = cycle;
			if (restart_phase >= system_phase) {
				seed -= uint32_t(1);
			}
			seed *= uint32_t(pcount);
			seed += uint32_t(i);
			float random = float(idhash(seed) % uint32_t(65536)) / 65536.0;
			restart_phase += randomness_ratio * random * 1.0 / float(pcount);
		}

		restart_phase *= (1.0 - explosiveness_ratio);
		float restart_time = restart_phase * lifetime;
		bool restart = false;

		if (time > prev_time) {
			// restart_time >= prev_time is used so particles emit in the first frame they are processed

			if (restart_time >= prev_time && restart_time < time) {
				restart = true;
				if (fractional_delta) {
					local_delta = time - restart_time;
				}
			}

		} else if (local_delta > 0.0) {
			if (restart_time >= prev_time) {
				restart = true;
				if (fractional_delta) {
					local_delta = lifetime - restart_time + time;
				}

			} else if (restart_time < time) {
				restart = true;
				if (fractional_delta) {
					local_delta = time - restart_time;
				}
			}
		}

		if (p.time * (1.0 - explosiveness_ratio) > p.lifetime) {
			restart = true;
		}

		float tv = 0.0;

		if (restart) {
			if (!emitting) {
				p.active = false;
				continue;
			}
			p.active = true;

			/*float tex_linear_velocity = 0;
			if (curve_parameters[PARAM_INITIAL_LINEAR_VELOCITY].is_valid()) {
				tex_linear_velocity = curve_parameters[PARAM_INITIAL_LINEAR_VELOCITY]->interpolate(0);
			}*/

			float tex_angle = 0.0;
			if (curve_parameters[PARAM_ANGLE].is_valid()) {
				tex_angle = curve_parameters[PARAM_ANGLE]->interpolate(tv);
			}

			float tex_anim_offset = 0.0;
			if (curve_parameters[PARAM_ANGLE].is_valid()) {
				tex_anim_offset = curve_parameters[PARAM_ANGLE]->interpolate(tv);
			}

			p.seed = Math::rand();

			p.angle_rand = Math::randf();
			p.scale_rand = Math::randf();
			p.hue_rot_rand = Math::randf();
			p.anim_offset_rand = Math::randf();

			if (flags[FLAG_DISABLE_Z]) {
				float angle1_rad = Math::atan2(direction.y, direction.x) + (Math::randf() * 2.0 - 1.0) * Math_PI * spread / 180.0;
				Vector3 rot = Vector3(Math::cos(angle1_rad), Math::sin(angle1_rad), 0.0);
				p.velocity = rot * parameters[PARAM_INITIAL_LINEAR_VELOCITY] * Math::lerp(1.0f, float(Math::randf()), randomness[PARAM_INITIAL_LINEAR_VELOCITY]);
			} else {
				//initiate velocity spread in 3D
				float angle1_rad = Math::atan2(direction.x, direction.z) + (Math::randf() * 2.0 - 1.0) * Math_PI * spread / 180.0;
				float angle2_rad = Math::atan2(direction.y, Math::abs(direction.z)) + (Math::randf() * 2.0 - 1.0) * (1.0 - flatness) * Math_PI * spread / 180.0;

				Vector3 direction_xz = Vector3(Math::sin(angle1_rad), 0, Math::cos(angle1_rad));
				Vector3 direction_yz = Vector3(0, Math::sin(angle2_rad), Math::cos(angle2_rad));
				direction_yz.z = direction_yz.z / MAX(0.0001, Math::sqrt(ABS(direction_yz.z))); //better uniform distribution
				Vector3 direction = Vector3(direction_xz.x * direction_yz.z, direction_yz.y, direction_xz.z * direction_yz.z);
				direction.normalize();
				p.velocity = direction * parameters[PARAM_INITIAL_LINEAR_VELOCITY] * Math::lerp(1.0f, float(Math::randf()), randomness[PARAM_INITIAL_LINEAR_VELOCITY]);
			}

			float base_angle = (parameters[PARAM_ANGLE] + tex_angle) * Math::lerp(1.0f, p.angle_rand, randomness[PARAM_ANGLE]);
			p.custom[0] = Math::deg2rad(base_angle); //angle
			p.custom[1] = 0.0; //phase
			p.custom[2] = (parameters[PARAM_ANIM_OFFSET] + tex_anim_offset) * Math::lerp(1.0f, p.anim_offset_rand, randomness[PARAM_ANIM_OFFSET]); //animation offset (0-1)
			p.transform = Transform();
			p.time = 0;
			p.lifetime = lifetime * (1.0 - Math::randf() * lifetime_randomness);
			p.base_color = Color(1, 1, 1, 1);

			switch (emission_shape) {
				case EMISSION_SHAPE_POINT: {
					//do none
				} break;
				case EMISSION_SHAPE_SPHERE: {
					float s = 2.0 * Math::randf() - 1.0, t = 2.0 * Math_PI * Math::randf();
					float radius = emission_sphere_radius * Math::sqrt(1.0 - s * s);
					p.transform.origin = Vector3(radius * Math::cos(t), radius * Math::sin(t), emission_sphere_radius * s);
				} break;
				case EMISSION_SHAPE_BOX: {
					p.transform.origin = Vector3(Math::randf() * 2.0 - 1.0, Math::randf() * 2.0 - 1.0, Math::randf() * 2.0 - 1.0) * emission_box_extents;
				} break;
				case EMISSION_SHAPE_POINTS:
				case EMISSION_SHAPE_DIRECTED_POINTS: {
					int pc = emission_points.size();
					if (pc == 0)
						break;

					int random_idx = Math::rand() % pc;

					p.transform.origin = emission_points.get(random_idx);

					if (emission_shape == EMISSION_SHAPE_DIRECTED_POINTS && emission_normals.size() == pc) {
						if (flags[FLAG_DISABLE_Z]) {
							Vector3 normal = emission_normals.get(random_idx);
							Vector2 normal_2d(normal.x, normal.y);
							Transform2D m2;
							m2.set_axis(0, normal_2d);
							m2.set_axis(1, normal_2d.tangent());
							Vector2 velocity_2d(p.velocity.x, p.velocity.y);
							velocity_2d = m2.basis_xform(velocity_2d);
							p.velocity.x = velocity_2d.x;
							p.velocity.y = velocity_2d.y;
						} else {
							Vector3 normal = emission_normals.get(random_idx);
							Vector3 v0 = Math::abs(normal.z) < 0.999 ? Vector3(0.0, 0.0, 1.0) : Vector3(0, 1.0, 0.0);
							Vector3 tangent = v0.cross(normal).normalized();
							Vector3 bitangent = tangent.cross(normal).normalized();
							Basis m3;
							m3.set_axis(0, tangent);
							m3.set_axis(1, bitangent);
							m3.set_axis(2, normal);
							p.velocity = m3.xform(p.velocity);
						}
					}

					if (emission_colors.size() == pc) {
						p.base_color = emission_colors.get(random_idx);
					}
				} break;
				case EMISSION_SHAPE_MAX: { // Max value for validity check.
					break;
				}
			}

			if (!local_coords) {
				p.velocity = velocity_xform.xform(p.velocity);
				p.transform = emission_xform * p.transform;
			}

			if (flags[FLAG_DISABLE_Z]) {
				p.velocity.z = 0.0;
				p.transform.origin.z = 0.0;
			}

		} else if (!p.active) {
			continue;
		} else if (p.time > p.lifetime) {
			p.active = false;
			tv = 1.0;
		} else {
			uint32_t alt_seed = p.seed;

			p.time += local_delta;
			p.custom[1] = p.time / lifetime;
			tv = p.time / p.lifetime;

			float tex_linear_velocity = 0.0;
			if (curve_parameters[PARAM_INITIAL_LINEAR_VELOCITY].is_valid()) {
				tex_linear_velocity = curve_parameters[PARAM_INITIAL_LINEAR_VELOCITY]->interpolate(tv);
			}

			float tex_orbit_velocity = 0.0;
			if (flags[FLAG_DISABLE_Z]) {
				if (curve_parameters[PARAM_ORBIT_VELOCITY].is_valid()) {
					tex_orbit_velocity = curve_parameters[PARAM_ORBIT_VELOCITY]->interpolate(tv);
				}
			}

			float tex_angular_velocity = 0.0;
			if (curve_parameters[PARAM_ANGULAR_VELOCITY].is_valid()) {
				tex_angular_velocity = curve_parameters[PARAM_ANGULAR_VELOCITY]->interpolate(tv);
			}

			float tex_linear_accel = 0.0;
			if (curve_parameters[PARAM_LINEAR_ACCEL].is_valid()) {
				tex_linear_accel = curve_parameters[PARAM_LINEAR_ACCEL]->interpolate(tv);
			}

			float tex_tangential_accel = 0.0;
			if (curve_parameters[PARAM_TANGENTIAL_ACCEL].is_valid()) {
				tex_tangential_accel = curve_parameters[PARAM_TANGENTIAL_ACCEL]->interpolate(tv);
			}

			float tex_radial_accel = 0.0;
			if (curve_parameters[PARAM_RADIAL_ACCEL].is_valid()) {
				tex_radial_accel = curve_parameters[PARAM_RADIAL_ACCEL]->interpolate(tv);
			}

			float tex_damping = 0.0;
			if (curve_parameters[PARAM_DAMPING].is_valid()) {
				tex_damping = curve_parameters[PARAM_DAMPING]->interpolate(tv);
			}

			float tex_angle = 0.0;
			if (curve_parameters[PARAM_ANGLE].is_valid()) {
				tex_angle = curve_parameters[PARAM_ANGLE]->interpolate(tv);
			}
			float tex_anim_speed = 0.0;
			if (curve_parameters[PARAM_ANIM_SPEED].is_valid()) {
				tex_anim_speed = curve_parameters[PARAM_ANIM_SPEED]->interpolate(tv);
			}

			float tex_anim_offset = 0.0;
			if (curve_parameters[PARAM_ANIM_OFFSET].is_valid()) {
				tex_anim_offset = curve_parameters[PARAM_ANIM_OFFSET]->interpolate(tv);
			}

			Vector3 force = gravity;
			Vector3 position = p.transform.origin;
			if (flags[FLAG_DISABLE_Z]) {
				position.z = 0.0;
			}
			//apply linear acceleration
			force += p.velocity.length() > 0.0 ? p.velocity.normalized() * (parameters[PARAM_LINEAR_ACCEL] + tex_linear_accel) * Math::lerp(1.0f, rand_from_seed(alt_seed), randomness[PARAM_LINEAR_ACCEL]) : Vector3();
			//apply radial acceleration
			Vector3 org = emission_xform.origin;
			Vector3 diff = position - org;
			force += diff.length() > 0.0 ? diff.normalized() * (parameters[PARAM_RADIAL_ACCEL] + tex_radial_accel) * Math::lerp(1.0f, rand_from_seed(alt_seed), randomness[PARAM_RADIAL_ACCEL]) : Vector3();
			//apply tangential acceleration;
			if (flags[FLAG_DISABLE_Z]) {
				Vector2 yx = Vector2(diff.y, diff.x);
				Vector2 yx2 = (yx * Vector2(-1.0, 1.0)).normalized();
				force += yx.length() > 0.0 ? Vector3(yx2.x, yx2.y, 0.0) * ((parameters[PARAM_TANGENTIAL_ACCEL] + tex_tangential_accel) * Math::lerp(1.0f, rand_from_seed(alt_seed), randomness[PARAM_TANGENTIAL_ACCEL])) : Vector3();

			} else {
				Vector3 crossDiff = diff.normalized().cross(gravity.normalized());
				force += crossDiff.length() > 0.0 ? crossDiff.normalized() * ((parameters[PARAM_TANGENTIAL_ACCEL] + tex_tangential_accel) * Math::lerp(1.0f, rand_from_seed(alt_seed), randomness[PARAM_TANGENTIAL_ACCEL])) : Vector3();
			}
			//apply attractor forces
			p.velocity += force * local_delta;
			//orbit velocity
			if (flags[FLAG_DISABLE_Z]) {
				float orbit_amount = (parameters[PARAM_ORBIT_VELOCITY] + tex_orbit_velocity) * Math::lerp(1.0f, rand_from_seed(alt_seed), randomness[PARAM_ORBIT_VELOCITY]);
				if (orbit_amount != 0.0) {
					float ang = orbit_amount * local_delta * Math_PI * 2.0;
					// Not sure why the ParticlesMaterial code uses a clockwise rotation matrix,
					// but we use -ang here to reproduce its behavior.
					Transform2D rot = Transform2D(-ang, Vector2());
					Vector2 rotv = rot.basis_xform(Vector2(diff.x, diff.y));
					p.transform.origin -= Vector3(diff.x, diff.y, 0);
					p.transform.origin += Vector3(rotv.x, rotv.y, 0);
				}
			}
			if (curve_parameters[PARAM_INITIAL_LINEAR_VELOCITY].is_valid()) {
				p.velocity = p.velocity.normalized() * tex_linear_velocity;
			}
			if (parameters[PARAM_DAMPING] + tex_damping > 0.0) {
				float v = p.velocity.length();
				float damp = (parameters[PARAM_DAMPING] + tex_damping) * Math::lerp(1.0f, rand_from_seed(alt_seed), randomness[PARAM_DAMPING]);
				v -= damp * local_delta;
				if (v < 0.0) {
					p.velocity = Vector3();
				} else {
					p.velocity = p.velocity.normalized() * v;
				}
			}
			float base_angle = (parameters[PARAM_ANGLE] + tex_angle) * Math::lerp(1.0f, p.angle_rand, randomness[PARAM_ANGLE]);
			base_angle += p.custom[1] * lifetime * (parameters[PARAM_ANGULAR_VELOCITY] + tex_angular_velocity) * Math::lerp(1.0f, rand_from_seed(alt_seed) * 2.0f - 1.0f, randomness[PARAM_ANGULAR_VELOCITY]);
			p.custom[0] = Math::deg2rad(base_angle); //angle
			p.custom[2] = (parameters[PARAM_ANIM_OFFSET] + tex_anim_offset) * Math::lerp(1.0f, p.anim_offset_rand, randomness[PARAM_ANIM_OFFSET]) + p.custom[1] * (parameters[PARAM_ANIM_SPEED] + tex_anim_speed) * Math::lerp(1.0f, rand_from_seed(alt_seed), randomness[PARAM_ANIM_SPEED]); //angle
		}
		//apply color
		//apply hue rotation

		float tex_scale = 1.0;
		if (curve_parameters[PARAM_SCALE].is_valid()) {
			tex_scale = curve_parameters[PARAM_SCALE]->interpolate(tv);
		}

		float tex_hue_variation = 0.0;
		if (curve_parameters[PARAM_HUE_VARIATION].is_valid()) {
			tex_hue_variation = curve_parameters[PARAM_HUE_VARIATION]->interpolate(tv);
		}

		float hue_rot_angle = (parameters[PARAM_HUE_VARIATION] + tex_hue_variation) * Math_PI * 2.0 * Math::lerp(1.0f, p.hue_rot_rand * 2.0f - 1.0f, randomness[PARAM_HUE_VARIATION]);
		float hue_rot_c = Math::cos(hue_rot_angle);
		float hue_rot_s = Math::sin(hue_rot_angle);

		Basis hue_rot_mat;
		{
			Basis mat1(0.299, 0.587, 0.114, 0.299, 0.587, 0.114, 0.299, 0.587, 0.114);
			Basis mat2(0.701, -0.587, -0.114, -0.299, 0.413, -0.114, -0.300, -0.588, 0.886);
			Basis mat3(0.168, 0.330, -0.497, -0.328, 0.035, 0.292, 1.250, -1.050, -0.203);

			for (int j = 0; j < 3; j++) {
				hue_rot_mat[j] = mat1[j] + mat2[j] * hue_rot_c + mat3[j] * hue_rot_s;
			}
		}

		if (color_ramp.is_valid()) {
			p.color = color_ramp->get_color_at_offset(tv) * color;
		} else {
			p.color = color;
		}

		Vector3 color_rgb = hue_rot_mat.xform_inv(Vector3(p.color.r, p.color.g, p.color.b));
		p.color.r = color_rgb.x;
		p.color.g = color_rgb.y;
		p.color.b = color_rgb.z;

		p.color *= p.base_color;

		if (flags[FLAG_DISABLE_Z]) {
			if (flags[FLAG_ALIGN_Y_TO_VELOCITY]) {
				if (p.velocity.length() > 0.0) {
					p.transform.basis.set_axis(1, p.velocity.normalized());
				} else {
					p.transform.basis.set_axis(1, p.transform.basis.get_axis(1));
				}
				p.transform.basis.set_axis(0, p.transform.basis.get_axis(1).cross(p.transform.basis.get_axis(2)).normalized());
				p.transform.basis.set_axis(2, Vector3(0, 0, 1));

			} else {
				p.transform.basis.set_axis(0, Vector3(Math::cos(p.custom[0]), -Math::sin(p.custom[0]), 0.0));
				p.transform.basis.set_axis(1, Vector3(Math::sin(p.custom[0]), Math::cos(p.custom[0]), 0.0));
				p.transform.basis.set_axis(2, Vector3(0, 0, 1));
			}

		} else {
			//orient particle Y towards velocity
			if (flags[FLAG_ALIGN_Y_TO_VELOCITY]) {
				if (p.velocity.length() > 0.0) {
					p.transform.basis.set_axis(1, p.velocity.normalized());
				} else {
					p.transform.basis.set_axis(1, p.transform.basis.get_axis(1).normalized());
				}
				if (p.transform.basis.get_axis(1) == p.transform.basis.get_axis(0)) {
					p.transform.basis.set_axis(0, p.transform.basis.get_axis(1).cross(p.transform.basis.get_axis(2)).normalized());
					p.transform.basis.set_axis(2, p.transform.basis.get_axis(0).cross(p.transform.basis.get_axis(1)).normalized());
				} else {
					p.transform.basis.set_axis(2, p.transform.basis.get_axis(0).cross(p.transform.basis.get_axis(1)).normalized());
					p.transform.basis.set_axis(0, p.transform.basis.get_axis(1).cross(p.transform.basis.get_axis(2)).normalized());
				}
			} else {
				p.transform.basis.orthonormalize();
			}

			//turn particle by rotation in Y
			if (flags[FLAG_ROTATE_Y]) {
				Basis rot_y(Vector3(0, 1, 0), p.custom[0]);
				p.transform.basis = p.transform.basis * rot_y;
			}
		}

		//scale by scale
		float base_scale = tex_scale * Math::lerp(parameters[PARAM_SCALE], 1.0f, p.scale_rand * randomness[PARAM_SCALE]);
		if (base_scale < 0.000001)
			base_scale = 0.000001;

		p.transform.basis.scale(Vector3(1, 1, 1) * base_scale);

		if (flags[FLAG_DISABLE_Z]) {
			p.velocity.z = 0.0;
			p.transform.origin.z = 0.0;
		}

		p.transform.origin += p.velocity * local_delta;
	}
}

void CPUParticles::_update_particle_data_buffer() {
	update_mutex.lock();

	{
		int pc = particles.size();

		PoolVector<int>::Write ow;
		int *order = NULL;

		PoolVector<float>::Write w = particle_data.write();
		PoolVector<Particle>::Read r = particles.read();
		float *ptr = w.ptr();

		if (draw_order != DRAW_ORDER_INDEX) {
			ow = particle_order.write();
			order = ow.ptr();

			for (int i = 0; i < pc; i++) {
				order[i] = i;
			}
			if (draw_order == DRAW_ORDER_LIFETIME) {
				SortArray<int, SortLifetime> sorter;
				sorter.compare.particles = r.ptr();
				sorter.sort(order, pc);
			} else if (draw_order == DRAW_ORDER_VIEW_DEPTH) {
				ERR_FAIL_NULL(get_viewport());
				Camera *c = get_viewport()->get_camera();
				if (c) {
					Vector3 dir = c->get_global_transform().basis.get_axis(2); //far away to close

					if (local_coords) {
						// will look different from Particles in editor as this is based on the camera in the scenetree
						// and not the editor camera
						dir = inv_emission_transform.xform(dir).normalized();
					} else {
						dir = dir.normalized();
					}

					SortArray<int, SortAxis> sorter;
					sorter.compare.particles = r.ptr();
					sorter.compare.axis = dir;
					sorter.sort(order, pc);
				}
			}
		}

		for (int i = 0; i < pc; i++) {
			int idx = order ? order[i] : i;

			Transform t = r[idx].transform;

			if (!local_coords) {
				t = inv_emission_transform * t;
			}

			if (r[idx].active) {
				ptr[0] = t.basis.elements[0][0];
				ptr[1] = t.basis.elements[0][1];
				ptr[2] = t.basis.elements[0][2];
				ptr[3] = t.origin.x;
				ptr[4] = t.basis.elements[1][0];
				ptr[5] = t.basis.elements[1][1];
				ptr[6] = t.basis.elements[1][2];
				ptr[7] = t.origin.y;
				ptr[8] = t.basis.elements[2][0];
				ptr[9] = t.basis.elements[2][1];
				ptr[10] = t.basis.elements[2][2];
				ptr[11] = t.origin.z;
			} else {
				zeromem(ptr, sizeof(float) * 12);
			}

			Color c = r[idx].color;
			uint8_t *data8 = (uint8_t *)&ptr[12];
			data8[0] = CLAMP(c.r * 255.0, 0, 255);
			data8[1] = CLAMP(c.g * 255.0, 0, 255);
			data8[2] = CLAMP(c.b * 255.0, 0, 255);
			data8[3] = CLAMP(c.a * 255.0, 0, 255);

			ptr[13] = r[idx].custom[0];
			ptr[14] = r[idx].custom[1];
			ptr[15] = r[idx].custom[2];
			ptr[16] = r[idx].custom[3];

			ptr += 17;
		}

		can_update.set();
	}

	update_mutex.unlock();
}

void CPUParticles::_set_redraw(bool p_redraw) {
	if (redraw == p_redraw)
		return;
	redraw = p_redraw;
	update_mutex.lock();
	if (redraw) {
		VS::get_singleton()->connect("frame_pre_draw", this, "_update_render_thread");
		VS::get_singleton()->instance_geometry_set_flag(get_instance(), VS::INSTANCE_FLAG_DRAW_NEXT_FRAME_IF_VISIBLE, true);
		VS::get_singleton()->multimesh_set_visible_instances(multimesh, -1);
	} else {
		if (VS::get_singleton()->is_connected("frame_pre_draw", this, "_update_render_thread")) {
			VS::get_singleton()->disconnect("frame_pre_draw", this, "_update_render_thread");
		}
		VS::get_singleton()->instance_geometry_set_flag(get_instance(), VS::INSTANCE_FLAG_DRAW_NEXT_FRAME_IF_VISIBLE, false);
		VS::get_singleton()->multimesh_set_visible_instances(multimesh, 0);
	}
	update_mutex.unlock();
}

void CPUParticles::_update_render_thread() {
	update_mutex.lock();

	if (can_update.is_set()) {
		VS::get_singleton()->multimesh_set_as_bulk_array(multimesh, particle_data);
		can_update.clear(); //wait for next time
	}

	update_mutex.unlock();
}

void CPUParticles::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE) {
		set_process_internal(emitting);

		// first update before rendering to avoid one frame delay after emitting starts
		if (emitting && (time == 0))
			_update_internal();
	}

	if (p_what == NOTIFICATION_EXIT_TREE) {
		_set_redraw(false);
	}

	if (p_what == NOTIFICATION_VISIBILITY_CHANGED) {
		// first update before rendering to avoid one frame delay after emitting starts
		if (emitting && (time == 0))
			_update_internal();
	}

	if (p_what == NOTIFICATION_INTERNAL_PROCESS) {
		_update_internal();
	}

	if (p_what == NOTIFICATION_TRANSFORM_CHANGED) {
		inv_emission_transform = get_global_transform().affine_inverse();

		if (!local_coords) {
			int pc = particles.size();

			PoolVector<float>::Write w = particle_data.write();
			PoolVector<Particle>::Read r = particles.read();
			float *ptr = w.ptr();

			for (int i = 0; i < pc; i++) {
				Transform t = inv_emission_transform * r[i].transform;

				if (r[i].active) {
					ptr[0] = t.basis.elements[0][0];
					ptr[1] = t.basis.elements[0][1];
					ptr[2] = t.basis.elements[0][2];
					ptr[3] = t.origin.x;
					ptr[4] = t.basis.elements[1][0];
					ptr[5] = t.basis.elements[1][1];
					ptr[6] = t.basis.elements[1][2];
					ptr[7] = t.origin.y;
					ptr[8] = t.basis.elements[2][0];
					ptr[9] = t.basis.elements[2][1];
					ptr[10] = t.basis.elements[2][2];
					ptr[11] = t.origin.z;
				} else {
					zeromem(ptr, sizeof(float) * 12);
				}

				ptr += 17;
			}

			can_update.set();
		}
	}
}

void CPUParticles::convert_from_particles(Node *p_particles) {
	Particles *particles = Object::cast_to<Particles>(p_particles);
	ERR_FAIL_COND_MSG(!particles, "Only Particles nodes can be converted to CPUParticles.");

	set_emitting(particles->is_emitting());
	set_amount(particles->get_amount());
	set_lifetime(particles->get_lifetime());
	set_one_shot(particles->get_one_shot());
	set_pre_process_time(particles->get_pre_process_time());
	set_explosiveness_ratio(particles->get_explosiveness_ratio());
	set_randomness_ratio(particles->get_randomness_ratio());
	set_use_local_coordinates(particles->get_use_local_coordinates());
	set_fixed_fps(particles->get_fixed_fps());
	set_fractional_delta(particles->get_fractional_delta());
	set_speed_scale(particles->get_speed_scale());
	set_draw_order(DrawOrder(particles->get_draw_order()));
	set_mesh(particles->get_draw_pass_mesh(0));

	Ref<ParticlesMaterial> material = particles->get_process_material();
	if (material.is_null())
		return;

	set_direction(material->get_direction());
	set_spread(material->get_spread());
	set_flatness(material->get_flatness());

	set_color(material->get_color());

	Ref<GradientTexture> gt = material->get_color_ramp();
	if (gt.is_valid()) {
		set_color_ramp(gt->get_gradient());
	}

	set_particle_flag(FLAG_ALIGN_Y_TO_VELOCITY, material->get_flag(ParticlesMaterial::FLAG_ALIGN_Y_TO_VELOCITY));
	set_particle_flag(FLAG_ROTATE_Y, material->get_flag(ParticlesMaterial::FLAG_ROTATE_Y));
	set_particle_flag(FLAG_DISABLE_Z, material->get_flag(ParticlesMaterial::FLAG_DISABLE_Z));

	set_emission_shape(EmissionShape(material->get_emission_shape()));
	set_emission_sphere_radius(material->get_emission_sphere_radius());
	set_emission_box_extents(material->get_emission_box_extents());

	set_gravity(material->get_gravity());
	set_lifetime_randomness(material->get_lifetime_randomness());

#define CONVERT_PARAM(m_param)                                                            \
	set_param(m_param, material->get_param(ParticlesMaterial::m_param));                  \
	{                                                                                     \
		Ref<CurveTexture> ctex = material->get_param_texture(ParticlesMaterial::m_param); \
		if (ctex.is_valid())                                                              \
			set_param_curve(m_param, ctex->get_curve());                                  \
	}                                                                                     \
	set_param_randomness(m_param, material->get_param_randomness(ParticlesMaterial::m_param));

	CONVERT_PARAM(PARAM_INITIAL_LINEAR_VELOCITY);
	CONVERT_PARAM(PARAM_ANGULAR_VELOCITY);
	CONVERT_PARAM(PARAM_ORBIT_VELOCITY);
	CONVERT_PARAM(PARAM_LINEAR_ACCEL);
	CONVERT_PARAM(PARAM_RADIAL_ACCEL);
	CONVERT_PARAM(PARAM_TANGENTIAL_ACCEL);
	CONVERT_PARAM(PARAM_DAMPING);
	CONVERT_PARAM(PARAM_ANGLE);
	CONVERT_PARAM(PARAM_SCALE);
	CONVERT_PARAM(PARAM_HUE_VARIATION);
	CONVERT_PARAM(PARAM_ANIM_SPEED);
	CONVERT_PARAM(PARAM_ANIM_OFFSET);

#undef CONVERT_PARAM
}

void CPUParticles::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_emitting", "emitting"), &CPUParticles::set_emitting);
	ClassDB::bind_method(D_METHOD("set_amount", "amount"), &CPUParticles::set_amount);
	ClassDB::bind_method(D_METHOD("set_lifetime", "secs"), &CPUParticles::set_lifetime);
	ClassDB::bind_method(D_METHOD("set_one_shot", "enable"), &CPUParticles::set_one_shot);
	ClassDB::bind_method(D_METHOD("set_pre_process_time", "secs"), &CPUParticles::set_pre_process_time);
	ClassDB::bind_method(D_METHOD("set_explosiveness_ratio", "ratio"), &CPUParticles::set_explosiveness_ratio);
	ClassDB::bind_method(D_METHOD("set_randomness_ratio", "ratio"), &CPUParticles::set_randomness_ratio);
	ClassDB::bind_method(D_METHOD("set_lifetime_randomness", "random"), &CPUParticles::set_lifetime_randomness);
	ClassDB::bind_method(D_METHOD("set_use_local_coordinates", "enable"), &CPUParticles::set_use_local_coordinates);
	ClassDB::bind_method(D_METHOD("set_fixed_fps", "fps"), &CPUParticles::set_fixed_fps);
	ClassDB::bind_method(D_METHOD("set_fractional_delta", "enable"), &CPUParticles::set_fractional_delta);
	ClassDB::bind_method(D_METHOD("set_speed_scale", "scale"), &CPUParticles::set_speed_scale);

	ClassDB::bind_method(D_METHOD("is_emitting"), &CPUParticles::is_emitting);
	ClassDB::bind_method(D_METHOD("get_amount"), &CPUParticles::get_amount);
	ClassDB::bind_method(D_METHOD("get_lifetime"), &CPUParticles::get_lifetime);
	ClassDB::bind_method(D_METHOD("get_one_shot"), &CPUParticles::get_one_shot);
	ClassDB::bind_method(D_METHOD("get_pre_process_time"), &CPUParticles::get_pre_process_time);
	ClassDB::bind_method(D_METHOD("get_explosiveness_ratio"), &CPUParticles::get_explosiveness_ratio);
	ClassDB::bind_method(D_METHOD("get_randomness_ratio"), &CPUParticles::get_randomness_ratio);
	ClassDB::bind_method(D_METHOD("get_lifetime_randomness"), &CPUParticles::get_lifetime_randomness);
	ClassDB::bind_method(D_METHOD("get_use_local_coordinates"), &CPUParticles::get_use_local_coordinates);
	ClassDB::bind_method(D_METHOD("get_fixed_fps"), &CPUParticles::get_fixed_fps);
	ClassDB::bind_method(D_METHOD("get_fractional_delta"), &CPUParticles::get_fractional_delta);
	ClassDB::bind_method(D_METHOD("get_speed_scale"), &CPUParticles::get_speed_scale);

	ClassDB::bind_method(D_METHOD("set_draw_order", "order"), &CPUParticles::set_draw_order);

	ClassDB::bind_method(D_METHOD("get_draw_order"), &CPUParticles::get_draw_order);

	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &CPUParticles::set_mesh);
	ClassDB::bind_method(D_METHOD("get_mesh"), &CPUParticles::get_mesh);

	ClassDB::bind_method(D_METHOD("restart"), &CPUParticles::restart);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "emitting"), "set_emitting", "is_emitting");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "amount", PROPERTY_HINT_EXP_RANGE, "1,1000000,1"), "set_amount", "get_amount");
	ADD_GROUP("Time", "");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "lifetime", PROPERTY_HINT_EXP_RANGE, "0.01,600.0,0.01,or_greater"), "set_lifetime", "get_lifetime");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "one_shot"), "set_one_shot", "get_one_shot");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "preprocess", PROPERTY_HINT_EXP_RANGE, "0.00,600.0,0.01"), "set_pre_process_time", "get_pre_process_time");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "speed_scale", PROPERTY_HINT_RANGE, "0,64,0.01"), "set_speed_scale", "get_speed_scale");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "explosiveness", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_explosiveness_ratio", "get_explosiveness_ratio");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "randomness", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_randomness_ratio", "get_randomness_ratio");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "lifetime_randomness", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_lifetime_randomness", "get_lifetime_randomness");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "fixed_fps", PROPERTY_HINT_RANGE, "0,1000,1"), "set_fixed_fps", "get_fixed_fps");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "fract_delta"), "set_fractional_delta", "get_fractional_delta");
	ADD_GROUP("Drawing", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "local_coords"), "set_use_local_coordinates", "get_use_local_coordinates");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "draw_order", PROPERTY_HINT_ENUM, "Index,Lifetime,View Depth"), "set_draw_order", "get_draw_order");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");

	BIND_ENUM_CONSTANT(DRAW_ORDER_INDEX);
	BIND_ENUM_CONSTANT(DRAW_ORDER_LIFETIME);
	BIND_ENUM_CONSTANT(DRAW_ORDER_VIEW_DEPTH);

	////////////////////////////////

	ClassDB::bind_method(D_METHOD("set_direction", "direction"), &CPUParticles::set_direction);
	ClassDB::bind_method(D_METHOD("get_direction"), &CPUParticles::get_direction);

	ClassDB::bind_method(D_METHOD("set_spread", "degrees"), &CPUParticles::set_spread);
	ClassDB::bind_method(D_METHOD("get_spread"), &CPUParticles::get_spread);

	ClassDB::bind_method(D_METHOD("set_flatness", "amount"), &CPUParticles::set_flatness);
	ClassDB::bind_method(D_METHOD("get_flatness"), &CPUParticles::get_flatness);

	ClassDB::bind_method(D_METHOD("set_param", "param", "value"), &CPUParticles::set_param);
	ClassDB::bind_method(D_METHOD("get_param", "param"), &CPUParticles::get_param);

	ClassDB::bind_method(D_METHOD("set_param_randomness", "param", "randomness"), &CPUParticles::set_param_randomness);
	ClassDB::bind_method(D_METHOD("get_param_randomness", "param"), &CPUParticles::get_param_randomness);

	ClassDB::bind_method(D_METHOD("set_param_curve", "param", "curve"), &CPUParticles::set_param_curve);
	ClassDB::bind_method(D_METHOD("get_param_curve", "param"), &CPUParticles::get_param_curve);

	ClassDB::bind_method(D_METHOD("set_color", "color"), &CPUParticles::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &CPUParticles::get_color);

	ClassDB::bind_method(D_METHOD("set_color_ramp", "ramp"), &CPUParticles::set_color_ramp);
	ClassDB::bind_method(D_METHOD("get_color_ramp"), &CPUParticles::get_color_ramp);

	ClassDB::bind_method(D_METHOD("set_particle_flag", "flag", "enable"), &CPUParticles::set_particle_flag);
	ClassDB::bind_method(D_METHOD("get_particle_flag", "flag"), &CPUParticles::get_particle_flag);

	ClassDB::bind_method(D_METHOD("set_emission_shape", "shape"), &CPUParticles::set_emission_shape);
	ClassDB::bind_method(D_METHOD("get_emission_shape"), &CPUParticles::get_emission_shape);

	ClassDB::bind_method(D_METHOD("set_emission_sphere_radius", "radius"), &CPUParticles::set_emission_sphere_radius);
	ClassDB::bind_method(D_METHOD("get_emission_sphere_radius"), &CPUParticles::get_emission_sphere_radius);

	ClassDB::bind_method(D_METHOD("set_emission_box_extents", "extents"), &CPUParticles::set_emission_box_extents);
	ClassDB::bind_method(D_METHOD("get_emission_box_extents"), &CPUParticles::get_emission_box_extents);

	ClassDB::bind_method(D_METHOD("set_emission_points", "array"), &CPUParticles::set_emission_points);
	ClassDB::bind_method(D_METHOD("get_emission_points"), &CPUParticles::get_emission_points);

	ClassDB::bind_method(D_METHOD("set_emission_normals", "array"), &CPUParticles::set_emission_normals);
	ClassDB::bind_method(D_METHOD("get_emission_normals"), &CPUParticles::get_emission_normals);

	ClassDB::bind_method(D_METHOD("set_emission_colors", "array"), &CPUParticles::set_emission_colors);
	ClassDB::bind_method(D_METHOD("get_emission_colors"), &CPUParticles::get_emission_colors);

	ClassDB::bind_method(D_METHOD("get_gravity"), &CPUParticles::get_gravity);
	ClassDB::bind_method(D_METHOD("set_gravity", "accel_vec"), &CPUParticles::set_gravity);

	ClassDB::bind_method(D_METHOD("convert_from_particles", "particles"), &CPUParticles::convert_from_particles);

	ClassDB::bind_method(D_METHOD("_update_render_thread"), &CPUParticles::_update_render_thread);

	ADD_GROUP("Emission Shape", "emission_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "emission_shape", PROPERTY_HINT_ENUM, "Point,Sphere,Box,Points,Directed Points", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED), "set_emission_shape", "get_emission_shape");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "emission_sphere_radius", PROPERTY_HINT_RANGE, "0.01,128,0.01"), "set_emission_sphere_radius", "get_emission_sphere_radius");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "emission_box_extents"), "set_emission_box_extents", "get_emission_box_extents");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR3_ARRAY, "emission_points"), "set_emission_points", "get_emission_points");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR3_ARRAY, "emission_normals"), "set_emission_normals", "get_emission_normals");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_COLOR_ARRAY, "emission_colors"), "set_emission_colors", "get_emission_colors");
	ADD_GROUP("Flags", "flag_");
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "flag_align_y"), "set_particle_flag", "get_particle_flag", FLAG_ALIGN_Y_TO_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "flag_rotate_y"), "set_particle_flag", "get_particle_flag", FLAG_ROTATE_Y);
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "flag_disable_z"), "set_particle_flag", "get_particle_flag", FLAG_DISABLE_Z);
	ADD_GROUP("Direction", "");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "direction"), "set_direction", "get_direction");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "spread", PROPERTY_HINT_RANGE, "0,180,0.01"), "set_spread", "get_spread");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "flatness", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_flatness", "get_flatness");
	ADD_GROUP("Gravity", "");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "gravity"), "set_gravity", "get_gravity");
	ADD_GROUP("Initial Velocity", "initial_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "initial_velocity", PROPERTY_HINT_RANGE, "0,1000,0.01,or_greater"), "set_param", "get_param", PARAM_INITIAL_LINEAR_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "initial_velocity_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_INITIAL_LINEAR_VELOCITY);
	ADD_GROUP("Angular Velocity", "angular_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "angular_velocity", PROPERTY_HINT_RANGE, "-720,720,0.01,or_lesser,or_greater"), "set_param", "get_param", PARAM_ANGULAR_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "angular_velocity_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_ANGULAR_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "angular_velocity_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_ANGULAR_VELOCITY);
	ADD_GROUP("Orbit Velocity", "orbit_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "orbit_velocity", PROPERTY_HINT_RANGE, "-1000,1000,0.01,or_lesser,or_greater"), "set_param", "get_param", PARAM_ORBIT_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "orbit_velocity_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_ORBIT_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "orbit_velocity_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_ORBIT_VELOCITY);
	ADD_GROUP("Linear Accel", "linear_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "linear_accel", PROPERTY_HINT_RANGE, "-100,100,0.01,or_lesser,or_greater"), "set_param", "get_param", PARAM_LINEAR_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "linear_accel_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_LINEAR_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "linear_accel_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_LINEAR_ACCEL);
	ADD_GROUP("Radial Accel", "radial_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "radial_accel", PROPERTY_HINT_RANGE, "-100,100,0.01,or_lesser,or_greater"), "set_param", "get_param", PARAM_RADIAL_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "radial_accel_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_RADIAL_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "radial_accel_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_RADIAL_ACCEL);
	ADD_GROUP("Tangential Accel", "tangential_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "tangential_accel", PROPERTY_HINT_RANGE, "-100,100,0.01,or_lesser,or_greater"), "set_param", "get_param", PARAM_TANGENTIAL_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "tangential_accel_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_TANGENTIAL_ACCEL);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "tangential_accel_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_TANGENTIAL_ACCEL);
	ADD_GROUP("Damping", "");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "damping", PROPERTY_HINT_RANGE, "0,100,0.01"), "set_param", "get_param", PARAM_DAMPING);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "damping_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_DAMPING);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "damping_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_DAMPING);
	ADD_GROUP("Angle", "");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "angle", PROPERTY_HINT_RANGE, "-720,720,0.1,or_lesser,or_greater"), "set_param", "get_param", PARAM_ANGLE);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "angle_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_ANGLE);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "angle_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_ANGLE);
	ADD_GROUP("Scale", "");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "scale_amount", PROPERTY_HINT_RANGE, "0,1000,0.01,or_greater"), "set_param", "get_param", PARAM_SCALE);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "scale_amount_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_SCALE);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "scale_amount_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_SCALE);
	ADD_GROUP("Color", "");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "color_ramp", PROPERTY_HINT_RESOURCE_TYPE, "Gradient"), "set_color_ramp", "get_color_ramp");

	ADD_GROUP("Hue Variation", "hue_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "hue_variation", PROPERTY_HINT_RANGE, "-1,1,0.01"), "set_param", "get_param", PARAM_HUE_VARIATION);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "hue_variation_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_HUE_VARIATION);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "hue_variation_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_HUE_VARIATION);
	ADD_GROUP("Animation", "anim_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "anim_speed", PROPERTY_HINT_RANGE, "0,128,0.01,or_greater"), "set_param", "get_param", PARAM_ANIM_SPEED);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "anim_speed_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_ANIM_SPEED);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "anim_speed_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_ANIM_SPEED);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "anim_offset", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param", "get_param", PARAM_ANIM_OFFSET);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "anim_offset_random", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_param_randomness", "get_param_randomness", PARAM_ANIM_OFFSET);
	ADD_PROPERTYI(PropertyInfo(Variant::OBJECT, "anim_offset_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_param_curve", "get_param_curve", PARAM_ANIM_OFFSET);

	BIND_ENUM_CONSTANT(PARAM_INITIAL_LINEAR_VELOCITY);
	BIND_ENUM_CONSTANT(PARAM_ANGULAR_VELOCITY);
	BIND_ENUM_CONSTANT(PARAM_ORBIT_VELOCITY);
	BIND_ENUM_CONSTANT(PARAM_LINEAR_ACCEL);
	BIND_ENUM_CONSTANT(PARAM_RADIAL_ACCEL);
	BIND_ENUM_CONSTANT(PARAM_TANGENTIAL_ACCEL);
	BIND_ENUM_CONSTANT(PARAM_DAMPING);
	BIND_ENUM_CONSTANT(PARAM_ANGLE);
	BIND_ENUM_CONSTANT(PARAM_SCALE);
	BIND_ENUM_CONSTANT(PARAM_HUE_VARIATION);
	BIND_ENUM_CONSTANT(PARAM_ANIM_SPEED);
	BIND_ENUM_CONSTANT(PARAM_ANIM_OFFSET);
	BIND_ENUM_CONSTANT(PARAM_MAX);

	BIND_ENUM_CONSTANT(FLAG_ALIGN_Y_TO_VELOCITY);
	BIND_ENUM_CONSTANT(FLAG_ROTATE_Y);
	BIND_ENUM_CONSTANT(FLAG_DISABLE_Z);
	BIND_ENUM_CONSTANT(FLAG_MAX);

	BIND_ENUM_CONSTANT(EMISSION_SHAPE_POINT);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_SPHERE);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_BOX);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_POINTS);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_DIRECTED_POINTS);
	BIND_ENUM_CONSTANT(EMISSION_SHAPE_MAX);
}

CPUParticles::CPUParticles() {
	time = 0;
	inactive_time = 0;
	frame_remainder = 0;
	cycle = 0;
	redraw = false;
	emitting = false;

	set_notify_transform(true);

	multimesh = VisualServer::get_singleton()->multimesh_create();
	VisualServer::get_singleton()->multimesh_set_visible_instances(multimesh, 0);
	set_base(multimesh);

	set_emitting(true);
	set_one_shot(false);
	set_amount(8);
	set_lifetime(1);
	set_fixed_fps(0);
	set_fractional_delta(true);
	set_pre_process_time(0);
	set_explosiveness_ratio(0);
	set_randomness_ratio(0);
	set_lifetime_randomness(0);
	set_use_local_coordinates(true);

	set_draw_order(DRAW_ORDER_INDEX);
	set_speed_scale(1);

	set_direction(Vector3(1, 0, 0));
	set_spread(45);
	set_flatness(0);
	set_param(PARAM_INITIAL_LINEAR_VELOCITY, 0);
	set_param(PARAM_ANGULAR_VELOCITY, 0);
	set_param(PARAM_ORBIT_VELOCITY, 0);
	set_param(PARAM_LINEAR_ACCEL, 0);
	set_param(PARAM_RADIAL_ACCEL, 0);
	set_param(PARAM_TANGENTIAL_ACCEL, 0);
	set_param(PARAM_DAMPING, 0);
	set_param(PARAM_ANGLE, 0);
	set_param(PARAM_SCALE, 1);
	set_param(PARAM_HUE_VARIATION, 0);
	set_param(PARAM_ANIM_SPEED, 0);
	set_param(PARAM_ANIM_OFFSET, 0);
	set_emission_shape(EMISSION_SHAPE_POINT);
	set_emission_sphere_radius(1);
	set_emission_box_extents(Vector3(1, 1, 1));

	set_gravity(Vector3(0, -9.8, 0));

	for (int i = 0; i < PARAM_MAX; i++) {
		set_param_randomness(Parameter(i), 0);
	}

	for (int i = 0; i < FLAG_MAX; i++) {
		flags[i] = false;
	}

	set_color(Color(1, 1, 1, 1));
}

CPUParticles::~CPUParticles() {
	VS::get_singleton()->free(multimesh);
}
