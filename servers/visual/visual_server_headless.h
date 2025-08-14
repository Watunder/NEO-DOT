/*************************************************************************/
/*  visual_server_headless.h                                             */
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

#ifndef VISUAL_SERVER_HEADLESS_H
#define VISUAL_SERVER_HEADLESS_H

#include "servers/visual_server.h"

class VisualServerHeadless : public VisualServer {
public:
	// TEXTURE API
	struct DummyTexture : public RID_Data {
		int width;
		int height;
		uint32_t flags;
		Image::Format format;
		Ref<Image> image;
		String path;
	};

	mutable RID_Owner<DummyTexture> texture_owner;

	RID texture_create() override {
		DummyTexture *texture = memnew(DummyTexture);
		ERR_FAIL_COND_V(!texture, RID());
		return texture_owner.make_rid(texture);
	}

	void texture_allocate(RID p_texture, int p_width, int p_height, int p_depth_3d, Image::Format p_format, TextureType p_type, uint32_t p_flags = TEXTURE_FLAGS_DEFAULT) override {
		DummyTexture *t = texture_owner.getornull(p_texture);
		ERR_FAIL_COND(!t);
		t->width = p_width;
		t->height = p_height;
		t->flags = p_flags;
		t->format = p_format;
		t->image = Ref<Image>(memnew(Image));
		t->image->create(p_width, p_height, false, p_format);
	}

	void texture_set_data(RID p_texture, const Ref<Image> &p_image, int p_layer = 0) override {
		DummyTexture *t = texture_owner.getornull(p_texture);
		ERR_FAIL_COND(!t);
		t->width = p_image->get_width();
		t->height = p_image->get_height();
		t->format = p_image->get_format();
		t->image->create(t->width, t->height, false, t->format, p_image->get_data());
	}

	void texture_set_data_partial(RID p_texture, const Ref<Image> &p_image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int p_dst_mip, int p_layer = 0) override {
		DummyTexture *t = texture_owner.get(p_texture);

		ERR_FAIL_COND(!t);
		ERR_FAIL_COND_MSG(p_image.is_null(), "It's not a reference to a valid Image object.");
		ERR_FAIL_COND(t->format != p_image->get_format());
		ERR_FAIL_COND(src_w <= 0 || src_h <= 0);
		ERR_FAIL_COND(src_x < 0 || src_y < 0 || src_x + src_w > p_image->get_width() || src_y + src_h > p_image->get_height());
		ERR_FAIL_COND(dst_x < 0 || dst_y < 0 || dst_x + src_w > t->width || dst_y + src_h > t->height);

		t->image->blit_rect(p_image, Rect2(src_x, src_y, src_w, src_h), Vector2(dst_x, dst_y));
	}

	Ref<Image> texture_get_data(RID p_texture, int p_layer = 0) const override {
		DummyTexture *t = texture_owner.getornull(p_texture);
		ERR_FAIL_COND_V(!t, Ref<Image>());
		return t->image;
	}

	void texture_set_flags(RID p_texture, uint32_t p_flags) override {
		DummyTexture *t = texture_owner.getornull(p_texture);
		ERR_FAIL_COND(!t);
		t->flags = p_flags;
	}

	uint32_t texture_get_flags(RID p_texture) const override {
		DummyTexture *t = texture_owner.getornull(p_texture);
		ERR_FAIL_COND_V(!t, 0);
		return t->flags;
	}

	Image::Format texture_get_format(RID p_texture) const override {
		DummyTexture *t = texture_owner.getornull(p_texture);
		ERR_FAIL_COND_V(!t, Image::FORMAT_RGB8);
		return t->format;
	}

	TextureType texture_get_type(RID p_texture) const override { return TEXTURE_TYPE_2D; }
	uint32_t texture_get_texid(RID p_texture) const override { return 0; }
	uint32_t texture_get_width(RID p_texture) const override { return 0; }
	uint32_t texture_get_height(RID p_texture) const override { return 0; }
	uint32_t texture_get_depth(RID p_texture) const override { return 0; }
	void texture_set_size_override(RID p_texture, int p_width, int p_height, int p_depth_3d) override {}
	void texture_bind(RID p_texture, uint32_t p_texture_no) override {}

	void texture_set_path(RID p_texture, const String &p_path) override {
		DummyTexture *t = texture_owner.getornull(p_texture);
		ERR_FAIL_COND(!t);
		t->path = p_path;
	}

	String texture_get_path(RID p_texture) const override {
		DummyTexture *t = texture_owner.getornull(p_texture);
		ERR_FAIL_COND_V(!t, String());
		return t->path;
	}

	void texture_set_shrink_all_x2_on_set_data(bool p_enable) override {}
	void texture_set_detect_3d_callback(RID p_texture, TextureDetectCallback p_callback, void *p_userdata) override {}
	void texture_set_detect_srgb_callback(RID p_texture, TextureDetectCallback p_callback, void *p_userdata) override {}
	void texture_set_detect_normal_callback(RID p_texture, TextureDetectCallback p_callback, void *p_userdata) override {}
	void texture_debug_usage(List<TextureInfo> *r_info) override {}
	void textures_keep_original(bool p_enable) override {}
	void texture_set_proxy(RID p_proxy, RID p_base) override {}
	void texture_set_force_redraw_if_visible(RID p_texture, bool p_enable) override {}

	// SKY API
	RID sky_create() override { return RID(); }
	void sky_set_texture(RID p_sky, RID p_cube_map, int p_radiance_size) override {}

	// SHADER API
	RID shader_create() override { return RID(); }
	void shader_set_code(RID p_shader, const String &p_code) override {}
	String shader_get_code(RID p_shader) const override { return String(); }
	void shader_get_param_list(RID p_shader, List<PropertyInfo> *p_param_list) const override {}
	void shader_set_default_texture_param(RID p_shader, const StringName &p_name, RID p_texture) override {}
	RID shader_get_default_texture_param(RID p_shader, const StringName &p_name) const override { return RID(); }
	void shader_add_custom_define(RID p_shader, const String &p_define) override {}
	void shader_get_custom_defines(RID p_shader, Vector<String> *p_defines) const override {}
	void shader_remove_custom_define(RID p_shader, const String &p_define) override {}

	// MATERIAL API
	RID material_create() override { return RID(); }
	void material_set_shader(RID p_shader_material, RID p_shader) override {}
	RID material_get_shader(RID p_shader_material) const override { return RID(); }
	void material_set_param(RID p_material, const StringName &p_param, const Variant &p_value) override {}
	Variant material_get_param(RID p_material, const StringName &p_param) const override { return Variant(); }
	Variant material_get_param_default(RID p_material, const StringName &p_param) const override { return Variant(); }
	void material_set_render_priority(RID p_material, int priority) override {}
	void material_set_line_width(RID p_material, float p_width) override {}
	void material_set_next_pass(RID p_material, RID p_next_material) override {}

	// MESH API
	struct DummySurface {
		uint32_t format;
		PrimitiveType primitive;
		PoolVector<uint8_t> array;
		int vertex_count;
		PoolVector<uint8_t> index_array;
		int index_count;
		AABB aabb;
		Vector<PoolVector<uint8_t>> blend_shapes;
		Vector<AABB> bone_aabbs;
	};

	struct DummyMesh : public RID_Data {
		Vector<DummySurface> surfaces;
		int blend_shape_count;
		BlendShapeMode blend_shape_mode;
	};

	mutable RID_Owner<DummyMesh> mesh_owner;

	RID mesh_create() override {
		DummyMesh *mesh = memnew(DummyMesh);
		ERR_FAIL_COND_V(!mesh, RID());
		mesh->blend_shape_count = 0;
		mesh->blend_shape_mode = BLEND_SHAPE_MODE_NORMALIZED;
		return mesh_owner.make_rid(mesh);
	}

	void mesh_add_surface(RID p_mesh, uint32_t p_format, PrimitiveType p_primitive, const PoolVector<uint8_t> &p_array, int p_vertex_count, const PoolVector<uint8_t> &p_index_array, int p_index_count, const AABB &p_aabb, const Vector<PoolVector<uint8_t>> &p_blend_shapes = Vector<PoolVector<uint8_t>>(), const Vector<AABB> &p_bone_aabbs = Vector<AABB>()) override {
		DummyMesh *m = mesh_owner.getornull(p_mesh);
		ERR_FAIL_COND(!m);

		m->surfaces.push_back(DummySurface());
		DummySurface *s = &m->surfaces.write[m->surfaces.size() - 1];
		s->format = p_format;
		s->primitive = p_primitive;
		s->array = p_array;
		s->vertex_count = p_vertex_count;
		s->index_array = p_index_array;
		s->index_count = p_index_count;
		s->aabb = p_aabb;
		s->blend_shapes = p_blend_shapes;
		s->bone_aabbs = p_bone_aabbs;
	}

	void mesh_set_blend_shape_count(RID p_mesh, int p_amount) override {
		DummyMesh *m = mesh_owner.getornull(p_mesh);
		ERR_FAIL_COND(!m);
		m->blend_shape_count = p_amount;
	}

	int mesh_get_blend_shape_count(RID p_mesh) const override {
		DummyMesh *m = mesh_owner.getornull(p_mesh);
		ERR_FAIL_COND_V(!m, 0);
		return m->blend_shape_count;
	}

	void mesh_set_blend_shape_mode(RID p_mesh, BlendShapeMode p_mode) override {
		DummyMesh *m = mesh_owner.getornull(p_mesh);
		ERR_FAIL_COND(!m);
		m->blend_shape_mode = p_mode;
	}

	BlendShapeMode mesh_get_blend_shape_mode(RID p_mesh) const override {
		DummyMesh *m = mesh_owner.getornull(p_mesh);
		ERR_FAIL_COND_V(!m, BLEND_SHAPE_MODE_NORMALIZED);
		return m->blend_shape_mode;
	}

	void mesh_surface_update_region(RID p_mesh, int p_surface, int p_offset, const PoolVector<uint8_t> &p_data) override {}
	void mesh_surface_set_material(RID p_mesh, int p_surface, RID p_material) override {}
	RID mesh_surface_get_material(RID p_mesh, int p_surface) const override { return RID(); }

	int mesh_surface_get_array_len(RID p_mesh, int p_surface) const override {
		DummyMesh *m = mesh_owner.getornull(p_mesh);
		ERR_FAIL_COND_V(!m, 0);

		return m->surfaces[p_surface].vertex_count;
	}

	int mesh_surface_get_array_index_len(RID p_mesh, int p_surface) const override {
		DummyMesh *m = mesh_owner.getornull(p_mesh);
		ERR_FAIL_COND_V(!m, 0);

		return m->surfaces[p_surface].index_count;
	}

	PoolVector<uint8_t> mesh_surface_get_array(RID p_mesh, int p_surface) const override {
		DummyMesh *m = mesh_owner.getornull(p_mesh);
		ERR_FAIL_COND_V(!m, PoolVector<uint8_t>());

		return m->surfaces[p_surface].array;
	}

	PoolVector<uint8_t> mesh_surface_get_index_array(RID p_mesh, int p_surface) const override {
		DummyMesh *m = mesh_owner.getornull(p_mesh);
		ERR_FAIL_COND_V(!m, PoolVector<uint8_t>());

		return m->surfaces[p_surface].index_array;
	}

	uint32_t mesh_surface_get_format(RID p_mesh, int p_surface) const override {
		DummyMesh *m = mesh_owner.getornull(p_mesh);
		ERR_FAIL_COND_V(!m, 0);

		return m->surfaces[p_surface].format;
	}

	PrimitiveType mesh_surface_get_primitive_type(RID p_mesh, int p_surface) const override {
		DummyMesh *m = mesh_owner.getornull(p_mesh);
		ERR_FAIL_COND_V(!m, PRIMITIVE_POINTS);

		return m->surfaces[p_surface].primitive;
	}

	AABB mesh_surface_get_aabb(RID p_mesh, int p_surface) const override {
		DummyMesh *m = mesh_owner.getornull(p_mesh);
		ERR_FAIL_COND_V(!m, AABB());

		return m->surfaces[p_surface].aabb;
	}

	Vector<PoolVector<uint8_t>> mesh_surface_get_blend_shapes(RID p_mesh, int p_surface) const override {
		DummyMesh *m = mesh_owner.getornull(p_mesh);
		ERR_FAIL_COND_V(!m, Vector<PoolVector<uint8_t>>());

		return m->surfaces[p_surface].blend_shapes;
	}

	Vector<AABB> mesh_surface_get_skeleton_aabb(RID p_mesh, int p_surface) const override {
		DummyMesh *m = mesh_owner.getornull(p_mesh);
		ERR_FAIL_COND_V(!m, Vector<AABB>());

		return m->surfaces[p_surface].bone_aabbs;
	}

	void mesh_remove_surface(RID p_mesh, int p_index) override {
		DummyMesh *m = mesh_owner.getornull(p_mesh);
		ERR_FAIL_COND(!m);
		ERR_FAIL_COND(p_index >= m->surfaces.size());

		m->surfaces.remove(p_index);
	}

	int mesh_get_surface_count(RID p_mesh) const override {
		DummyMesh *m = mesh_owner.getornull(p_mesh);
		ERR_FAIL_COND_V(!m, 0);
		return m->surfaces.size();
	}

	void mesh_set_custom_aabb(RID p_mesh, const AABB &p_aabb) override {}
	AABB mesh_get_custom_aabb(RID p_mesh) const override { return AABB(); }
	void mesh_clear(RID p_mesh) override {}

	// MULTIMESH API
	RID multimesh_create() override { return RID(); }
	void multimesh_allocate(RID p_multimesh, int p_instances, MultimeshTransformFormat p_transform_format, MultimeshColorFormat p_color_format, MultimeshCustomDataFormat p_data_format = MULTIMESH_CUSTOM_DATA_NONE) override {}
	int multimesh_get_instance_count(RID p_multimesh) const override { return 0; }
	void multimesh_set_mesh(RID p_multimesh, RID p_mesh) override {}
	void multimesh_instance_set_transform(RID p_multimesh, int p_index, const Transform &p_transform) override {}
	void multimesh_instance_set_transform_2d(RID p_multimesh, int p_index, const Transform2D &p_transform) override {}
	void multimesh_instance_set_color(RID p_multimesh, int p_index, const Color &p_color) override {}
	void multimesh_instance_set_custom_data(RID p_multimesh, int p_index, const Color &p_color) override {}
	RID multimesh_get_mesh(RID p_multimesh) const override { return RID(); }
	AABB multimesh_get_aabb(RID p_multimesh) const override { return AABB(); }
	Transform multimesh_instance_get_transform(RID p_multimesh, int p_index) const override { return Transform(); }
	Transform2D multimesh_instance_get_transform_2d(RID p_multimesh, int p_index) const override { return Transform2D(); }
	Color multimesh_instance_get_color(RID p_multimesh, int p_index) const override { return Color(); }
	Color multimesh_instance_get_custom_data(RID p_multimesh, int p_index) const override { return Color(); }
	void multimesh_set_as_bulk_array(RID p_multimesh, const PoolVector<float> &p_array) override {}
	void multimesh_set_visible_instances(RID p_multimesh, int p_visible) override {}
	int multimesh_get_visible_instances(RID p_multimesh) const override { return 0; }

	// SKELETON API
	RID skeleton_create() override { return RID(); }
	void skeleton_allocate(RID p_skeleton, int p_bones, bool p_2d_skeleton = false) override {}
	int skeleton_get_bone_count(RID p_skeleton) const override { return 0; }
	void skeleton_bone_set_transform(RID p_skeleton, int p_bone, const Transform &p_transform) override {}
	Transform skeleton_bone_get_transform(RID p_skeleton, int p_bone) const override { return Transform(); }
	void skeleton_bone_set_transform_2d(RID p_skeleton, int p_bone, const Transform2D &p_transform) override {}
	Transform2D skeleton_bone_get_transform_2d(RID p_skeleton, int p_bone) const override { return Transform2D(); }
	void skeleton_set_base_transform_2d(RID p_skeleton, const Transform2D &p_base_transform) override {}

	// LIGHT API
	RID directional_light_create() override { return RID(); }
	RID omni_light_create() override { return RID(); }
	RID spot_light_create() override { return RID(); }
	void light_set_color(RID p_light, const Color &p_color) override {}
	void light_set_param(RID p_light, LightParam p_param, float p_value) override {}
	void light_set_shadow(RID p_light, bool p_enabled) override {}
	void light_set_shadow_color(RID p_light, const Color &p_color) override {}
	void light_set_projector(RID p_light, RID p_texture) override {}
	void light_set_negative(RID p_light, bool p_enable) override {}
	void light_set_cull_mask(RID p_light, uint32_t p_mask) override {}
	void light_set_reverse_cull_face_mode(RID p_light, bool p_enabled) override {}
	void light_set_use_gi(RID p_light, bool p_enable) override {}
	void light_set_bake_mode(RID p_light, LightBakeMode p_bake_mode) override {}
	void light_omni_set_shadow_mode(RID p_light, LightOmniShadowMode p_mode) override {}
	void light_omni_set_shadow_detail(RID p_light, LightOmniShadowDetail p_detail) override {}
	void light_directional_set_shadow_mode(RID p_light, LightDirectionalShadowMode p_mode) override {}
	void light_directional_set_blend_splits(RID p_light, bool p_enable) override {}
	void light_directional_set_shadow_depth_range_mode(RID p_light, LightDirectionalShadowDepthRangeMode p_range_mode) override {}

	// PROBE API
	RID reflection_probe_create() override { return RID(); }
	void reflection_probe_set_update_mode(RID p_probe, ReflectionProbeUpdateMode p_mode) override {}
	void reflection_probe_set_intensity(RID p_probe, float p_intensity) override {}
	void reflection_probe_set_interior_ambient(RID p_probe, const Color &p_color) override {}
	void reflection_probe_set_interior_ambient_energy(RID p_probe, float p_energy) override {}
	void reflection_probe_set_interior_ambient_probe_contribution(RID p_probe, float p_contrib) override {}
	void reflection_probe_set_max_distance(RID p_probe, float p_distance) override {}
	void reflection_probe_set_extents(RID p_probe, const Vector3 &p_extents) override {}
	void reflection_probe_set_origin_offset(RID p_probe, const Vector3 &p_offset) override {}
	void reflection_probe_set_as_interior(RID p_probe, bool p_enable) override {}
	void reflection_probe_set_enable_box_projection(RID p_probe, bool p_enable) override {}
	void reflection_probe_set_enable_shadows(RID p_probe, bool p_enable) override {}
	void reflection_probe_set_cull_mask(RID p_probe, uint32_t p_layers) override {}
	void reflection_probe_set_resolution(RID p_probe, int p_resolution) override {}

	// GI PROBE API
	RID gi_probe_create() override { return RID(); }
	void gi_probe_set_bounds(RID p_probe, const AABB &p_bounds) override {}
	AABB gi_probe_get_bounds(RID p_probe) const override { return AABB(); }
	void gi_probe_set_cell_size(RID p_probe, float p_range) override {}
	float gi_probe_get_cell_size(RID p_probe) const override { return 0.0f; }
	void gi_probe_set_to_cell_xform(RID p_probe, const Transform &p_xform) override {}
	Transform gi_probe_get_to_cell_xform(RID p_probe) const override { return Transform(); }
	void gi_probe_set_dynamic_data(RID p_probe, const PoolVector<int> &p_data) override {}
	PoolVector<int> gi_probe_get_dynamic_data(RID p_probe) const override { return PoolVector<int>(); }
	void gi_probe_set_dynamic_range(RID p_probe, int p_range) override {}
	int gi_probe_get_dynamic_range(RID p_probe) const override { return 0; }
	void gi_probe_set_energy(RID p_probe, float p_range) override {}
	float gi_probe_get_energy(RID p_probe) const override { return 0.0f; }
	void gi_probe_set_bias(RID p_probe, float p_range) override {}
	float gi_probe_get_bias(RID p_probe) const override { return 0.0f; }
	void gi_probe_set_normal_bias(RID p_probe, float p_range) override {}
	float gi_probe_get_normal_bias(RID p_probe) const override { return 0.0f; }
	void gi_probe_set_propagation(RID p_probe, float p_range) override {}
	float gi_probe_get_propagation(RID p_probe) const override { return 0.0f; }
	void gi_probe_set_interior(RID p_probe, bool p_enable) override {}
	bool gi_probe_is_interior(RID p_probe) const override { return false; }
	void gi_probe_set_compress(RID p_probe, bool p_enable) override {}
	bool gi_probe_is_compressed(RID p_probe) const override { return false; }

	// LIGHTMAP CAPTURE
	RID lightmap_capture_create() override { return RID(); }
	void lightmap_capture_set_bounds(RID p_capture, const AABB &p_bounds) override {}
	AABB lightmap_capture_get_bounds(RID p_capture) const override { return AABB(); }
	void lightmap_capture_set_octree(RID p_capture, const PoolVector<uint8_t> &p_octree) override {}
	void lightmap_capture_set_octree_cell_transform(RID p_capture, const Transform &p_xform) override {}
	Transform lightmap_capture_get_octree_cell_transform(RID p_capture) const override { return Transform(); }
	void lightmap_capture_set_octree_cell_subdiv(RID p_capture, int p_subdiv) override {}
	int lightmap_capture_get_octree_cell_subdiv(RID p_capture) const override { return 0; }
	PoolVector<uint8_t> lightmap_capture_get_octree(RID p_capture) const override { return PoolVector<uint8_t>(); }
	void lightmap_capture_set_energy(RID p_capture, float p_energy) override {}
	float lightmap_capture_get_energy(RID p_capture) const override { return 0.0f; }
	void lightmap_capture_set_interior(RID p_capture, bool p_interior) override {}
	bool lightmap_capture_is_interior(RID p_capture) const override { return false; }

	// PARTICLES API
	RID particles_create() override { return RID(); }
	void particles_set_emitting(RID p_particles, bool p_emitting) override {}
	bool particles_get_emitting(RID p_particles) override { return false; }
	void particles_set_amount(RID p_particles, int p_amount) override {}
	void particles_set_lifetime(RID p_particles, float p_lifetime) override {}
	void particles_set_one_shot(RID p_particles, bool p_one_shot) override {}
	void particles_set_pre_process_time(RID p_particles, float p_time) override {}
	void particles_set_explosiveness_ratio(RID p_particles, float p_ratio) override {}
	void particles_set_randomness_ratio(RID p_particles, float p_ratio) override {}
	void particles_set_custom_aabb(RID p_particles, const AABB &p_aabb) override {}
	void particles_set_speed_scale(RID p_particles, float p_scale) override {}
	void particles_set_use_local_coordinates(RID p_particles, bool p_enable) override {}
	void particles_set_process_material(RID p_particles, RID p_material) override {}
	void particles_set_fixed_fps(RID p_particles, int p_fps) override {}
	void particles_set_fractional_delta(RID p_particles, bool p_enable) override {}
	bool particles_is_inactive(RID p_particles) override { return false; }
	void particles_request_process(RID p_particles) override {}
	void particles_restart(RID p_particles) override {}
	void particles_set_draw_order(RID p_particles, ParticlesDrawOrder p_order) override {}
	void particles_set_draw_passes(RID p_particles, int p_count) override {}
	void particles_set_draw_pass_mesh(RID p_particles, int p_pass, RID p_mesh) override {}
	AABB particles_get_current_aabb(RID p_particles) override { return AABB(); }
	void particles_set_emission_transform(RID p_particles, const Transform &p_transform) override {}

	// CAMERA API
	RID camera_create() override { return RID(); }
	void camera_set_perspective(RID p_camera, float p_fovy_degrees, float p_z_near, float p_z_far) override {}
	void camera_set_orthogonal(RID p_camera, float p_size, float p_z_near, float p_z_far) override {}
	void camera_set_frustum(RID p_camera, float p_size, Vector2 p_offset, float p_z_near, float p_z_far) override {}
	void camera_set_transform(RID p_camera, const Transform &p_transform) override {}
	void camera_set_cull_mask(RID p_camera, uint32_t p_layers) override {}
	void camera_set_environment(RID p_camera, RID p_env) override {}
	void camera_set_use_vertical_aspect(RID p_camera, bool p_enable) override {}

	// VIEWPORT TARGET API
	RID viewport_create() override { return RID(); }
	void viewport_set_use_arvr(RID p_viewport, bool p_use_arvr) override {}
	void viewport_set_size(RID p_viewport, int p_width, int p_height) override {}
	void viewport_set_active(RID p_viewport, bool p_active) override {}
	void viewport_set_parent_viewport(RID p_viewport, RID p_parent_viewport) override {}
	void viewport_attach_to_screen(RID p_viewport, const Rect2 &p_rect = Rect2(), int p_screen = 0) override {}
	void viewport_set_render_direct_to_screen(RID p_viewport, bool p_enable) override {}
	void viewport_detach(RID p_viewport) override {}
	void viewport_set_update_mode(RID p_viewport, ViewportUpdateMode p_mode) override {}
	void viewport_set_vflip(RID p_viewport, bool p_enable) override {}
	void viewport_set_clear_mode(RID p_viewport, ViewportClearMode p_clear_mode) override {}
	RID viewport_get_texture(RID p_viewport) const override { return RID(); }
	void viewport_set_hide_scenario(RID p_viewport, bool p_hide) override {}
	void viewport_set_hide_canvas(RID p_viewport, bool p_hide) override {}
	void viewport_set_disable_environment(RID p_viewport, bool p_disable) override {}
	void viewport_set_disable_3d(RID p_viewport, bool p_disable) override {}
	void viewport_set_keep_3d_linear(RID p_viewport, bool p_disable) override {}
	void viewport_attach_camera(RID p_viewport, RID p_camera) override {}
	void viewport_set_scenario(RID p_viewport, RID p_scenario) override {}
	void viewport_attach_canvas(RID p_viewport, RID p_canvas) override {}
	void viewport_remove_canvas(RID p_viewport, RID p_canvas) override {}
	void viewport_set_canvas_transform(RID p_viewport, RID p_canvas, const Transform2D &p_offset) override {}
	void viewport_set_transparent_background(RID p_viewport, bool p_enabled) override {}
	void viewport_set_global_canvas_transform(RID p_viewport, const Transform2D &p_transform) override {}
	void viewport_set_canvas_stacking(RID p_viewport, RID p_canvas, int p_layer, int p_sublayer) override {}
	void viewport_set_shadow_atlas_size(RID p_viewport, int p_size) override {}
	void viewport_set_shadow_atlas_quadrant_subdivision(RID p_viewport, int p_quadrant, int p_subdiv) override {}
	void viewport_set_msaa(RID p_viewport, ViewportMSAA p_msaa) override {}
	void viewport_set_use_fxaa(RID p_viewport, bool p_fxaa) override {}
	void viewport_set_use_debanding(RID p_viewport, bool p_debanding) override {}
	void viewport_set_hdr(RID p_viewport, bool p_enabled) override {}
	void viewport_set_usage(RID p_viewport, ViewportUsage p_usage) override {}
	int viewport_get_render_info(RID p_viewport, ViewportRenderInfo p_info) override { return 0; }
	void viewport_set_debug_draw(RID p_viewport, ViewportDebugDraw p_draw) override {}

	// ENVIRONMENT API
	RID environment_create() override { return RID(); }
	void environment_set_background(RID p_env, EnvironmentBG p_bg) override {}
	void environment_set_sky(RID p_env, RID p_sky) override {}
	void environment_set_sky_custom_fov(RID p_env, float p_scale) override {}
	void environment_set_sky_orientation(RID p_env, const Basis &p_orientation) override {}
	void environment_set_bg_color(RID p_env, const Color &p_color) override {}
	void environment_set_bg_energy(RID p_env, float p_energy) override {}
	void environment_set_canvas_max_layer(RID p_env, int p_max_layer) override {}
	void environment_set_ambient_light(RID p_env, const Color &p_color, float p_energy = 1.0, float p_sky_contribution = 0.0) override {}
	void environment_set_camera_feed_id(RID p_env, int p_camera_feed_id) override {}
	void environment_set_dof_blur_near(RID p_env, bool p_enable, float p_distance, float p_transition, float p_far_amount, EnvironmentDOFBlurQuality p_quality) override {}
	void environment_set_dof_blur_far(RID p_env, bool p_enable, float p_distance, float p_transition, float p_far_amount, EnvironmentDOFBlurQuality p_quality) override {}
	void environment_set_glow(RID p_env, bool p_enable, int p_level_flags, float p_intensity, float p_strength, float p_bloom_threshold, EnvironmentGlowBlendMode p_blend_mode, float p_hdr_bleed_threshold, float p_hdr_bleed_scale, float p_hdr_luminance_cap, bool p_bicubic_upscale) override {}
	void environment_set_tonemap(RID p_env, EnvironmentToneMapper p_tone_mapper, float p_exposure, float p_white, bool p_auto_exposure, float p_min_luminance, float p_max_luminance, float p_auto_exp_speed, float p_auto_exp_grey) override {}
	void environment_set_adjustment(RID p_env, bool p_enable, float p_brightness, float p_contrast, float p_saturation, RID p_ramp) override {}
	void environment_set_ssr(RID p_env, bool p_enable, int p_max_steps, float p_fade_in, float p_fade_out, float p_depth_tolerance, bool p_roughness) override {}
	void environment_set_ssao(RID p_env, bool p_enable, float p_radius, float p_intensity, float p_radius2, float p_intensity2, float p_bias, float p_light_affect, float p_ao_channel_affect, const Color &p_color, EnvironmentSSAOQuality p_quality, EnvironmentSSAOBlur p_blur, float p_bilateral_sharpness) override {}
	void environment_set_fog(RID p_env, bool p_enable, const Color &p_color, const Color &p_sun_color, float p_sun_amount) override {}
	void environment_set_fog_depth(RID p_env, bool p_enable, float p_depth_begin, float p_depth_end, float p_depth_curve, bool p_transmit, float p_transmit_curve) override {}
	void environment_set_fog_height(RID p_env, bool p_enable, float p_min_height, float p_max_height, float p_height_curve) override {}

	// SCENARIO API
	RID scenario_create() override { return RID(); }
	void scenario_set_debug(RID p_scenario, ScenarioDebugMode p_debug_mode) override {}
	void scenario_set_environment(RID p_scenario, RID p_environment) override {}
	void scenario_set_reflection_atlas_size(RID p_scenario, int p_size, int p_subdiv) override {}
	void scenario_set_fallback_environment(RID p_scenario, RID p_environment) override {}

	// INSTANCING API
	RID instance_create() override { return RID(); }
	void instance_set_base(RID p_instance, RID p_base) override {}
	void instance_set_scenario(RID p_instance, RID p_scenario) override {}
	void instance_set_layer_mask(RID p_instance, uint32_t p_mask) override {}
	void instance_set_transform(RID p_instance, const Transform &p_transform) override {}
	void instance_attach_object_instance_id(RID p_instance, ObjectID p_id) override {}
	void instance_set_blend_shape_weight(RID p_instance, int p_shape, float p_weight) override {}
	void instance_set_surface_material(RID p_instance, int p_surface, RID p_material) override {}
	void instance_set_visible(RID p_instance, bool p_visible) override {}
	void instance_set_use_lightmap(RID p_instance, RID p_lightmap_instance, RID p_lightmap, int p_lightmap_slice, const Rect2 &p_lightmap_uv_rect) override {}
	void instance_set_custom_aabb(RID p_instance, AABB aabb) override {}
	void instance_attach_skeleton(RID p_instance, RID p_skeleton) override {}
	void instance_set_exterior(RID p_instance, bool p_enabled) override {}
	void instance_set_extra_visibility_margin(RID p_instance, real_t p_margin) override {}
	Vector<ObjectID> instances_cull_aabb(const AABB &p_aabb, RID p_scenario = RID()) const override { return Vector<ObjectID>(); }
	Vector<ObjectID> instances_cull_ray(const Vector3 &p_from, const Vector3 &p_to, RID p_scenario = RID()) const override { return Vector<ObjectID>(); }
	Vector<ObjectID> instances_cull_convex(const Vector<Plane> &p_convex, RID p_scenario = RID()) const override { return Vector<ObjectID>(); }
	void instance_geometry_set_flag(RID p_instance, InstanceFlags p_flags, bool p_enabled) override {}
	void instance_geometry_set_cast_shadows_setting(RID p_instance, ShadowCastingSetting p_shadow_casting_setting) override {}
	void instance_geometry_set_material_override(RID p_instance, RID p_material) override {}
	void instance_geometry_set_draw_range(RID p_instance, float p_min, float p_max, float p_min_margin, float p_max_margin) override {}
	void instance_geometry_set_as_instance_lod(RID p_instance, RID p_as_lod_of_instance) override {}

	// CANVAS (2D)
	RID canvas_create() override { return RID(); }
	void canvas_set_item_mirroring(RID p_canvas, RID p_item, const Point2 &p_mirroring) override {}
	void canvas_set_modulate(RID p_canvas, const Color &p_color) override {}
	void canvas_set_parent(RID p_canvas, RID p_parent, float p_scale) override {}
	void canvas_set_disable_scale(bool p_disable) override {}
	RID canvas_item_create() override { return RID(); }
	void canvas_item_set_parent(RID p_item, RID p_parent) override {}
	void canvas_item_set_visible(RID p_item, bool p_visible) override {}
	void canvas_item_set_light_mask(RID p_item, int p_mask) override {}
	void canvas_item_set_update_when_visible(RID p_item, bool p_update) override {}
	void canvas_item_set_transform(RID p_item, const Transform2D &p_transform) override {}
	void canvas_item_set_clip(RID p_item, bool p_clip) override {}
	void canvas_item_set_distance_field_mode(RID p_item, bool p_enable) override {}
	void canvas_item_set_custom_rect(RID p_item, bool p_custom_rect, const Rect2 &p_rect = Rect2()) override {}
	void canvas_item_set_modulate(RID p_item, const Color &p_color) override {}
	void canvas_item_set_self_modulate(RID p_item, const Color &p_color) override {}
	void canvas_item_set_draw_behind_parent(RID p_item, bool p_enable) override {}
	void canvas_item_add_line(RID p_item, const Point2 &p_from, const Point2 &p_to, const Color &p_color, float p_width = 1.0, bool p_antialiased = false) override {}
	void canvas_item_add_polyline(RID p_item, const Vector<Point2> &p_points, const Vector<Color> &p_colors, float p_width = 1.0, bool p_antialiased = false) override {}
	void canvas_item_add_multiline(RID p_item, const Vector<Point2> &p_points, const Vector<Color> &p_colors, float p_width = 1.0, bool p_antialiased = false) override {}
	void canvas_item_add_rect(RID p_item, const Rect2 &p_rect, const Color &p_color) override {}
	void canvas_item_add_circle(RID p_item, const Point2 &p_pos, float p_radius, const Color &p_color) override {}
	void canvas_item_add_texture_rect(RID p_item, const Rect2 &p_rect, RID p_texture, bool p_tile = false, const Color &p_modulate = Color(1, 1, 1), bool p_transpose = false, RID p_normal_map = RID()) override {}
	void canvas_item_add_texture_rect_region(RID p_item, const Rect2 &p_rect, RID p_texture, const Rect2 &p_src_rect, const Color &p_modulate = Color(1, 1, 1), bool p_transpose = false, RID p_normal_map = RID(), bool p_clip_uv = false) override {}
	void canvas_item_add_nine_patch(RID p_item, const Rect2 &p_rect, const Rect2 &p_source, RID p_texture, const Vector2 &p_topleft, const Vector2 &p_bottomright, NinePatchAxisMode p_x_axis_mode = NINE_PATCH_STRETCH, NinePatchAxisMode p_y_axis_mode = NINE_PATCH_STRETCH, bool p_draw_center = true, const Color &p_modulate = Color(1, 1, 1), RID p_normal_map = RID()) override {}
	void canvas_item_add_primitive(RID p_item, const Vector<Point2> &p_points, const Vector<Color> &p_colors, const Vector<Point2> &p_uvs, RID p_texture, float p_width = 1.0, RID p_normal_map = RID()) override {}
	void canvas_item_add_polygon(RID p_item, const Vector<Point2> &p_points, const Vector<Color> &p_colors, const Vector<Point2> &p_uvs = Vector<Point2>(), RID p_texture = RID(), RID p_normal_map = RID(), bool p_antialiased = false) override {}
	void canvas_item_add_triangle_array(RID p_item, const Vector<int> &p_indices, const Vector<Point2> &p_points, const Vector<Color> &p_colors, const Vector<Point2> &p_uvs = Vector<Point2>(), const Vector<int> &p_bones = Vector<int>(), const Vector<float> &p_weights = Vector<float>(), RID p_texture = RID(), int p_count = -1, RID p_normal_map = RID(), bool p_antialiased = false, bool p_antialiasing_use_indices = false) override {}
	void canvas_item_add_mesh(RID p_item, const RID &p_mesh, const Transform2D &p_transform = Transform2D(), const Color &p_modulate = Color(1, 1, 1), RID p_texture = RID(), RID p_normal_map = RID()) override {}
	void canvas_item_add_multimesh(RID p_item, RID p_mesh, RID p_texture = RID(), RID p_normal_map = RID()) override {}
	void canvas_item_add_particles(RID p_item, RID p_particles, RID p_texture, RID p_normal_map) override {}
	void canvas_item_add_set_transform(RID p_item, const Transform2D &p_transform) override {}
	void canvas_item_add_clip_ignore(RID p_item, bool p_ignore) override {}
	void canvas_item_set_sort_children_by_y(RID p_item, bool p_enable) override {}
	void canvas_item_set_z_index(RID p_item, int p_z) override {}
	void canvas_item_set_z_as_relative_to_parent(RID p_item, bool p_enable) override {}
	void canvas_item_set_copy_to_backbuffer(RID p_item, bool p_enable, const Rect2 &p_rect) override {}
	void canvas_item_attach_skeleton(RID p_item, RID p_skeleton) override {}
	void canvas_item_clear(RID p_item) override {}
	void canvas_item_set_draw_index(RID p_item, int p_index) override {}
	void canvas_item_set_material(RID p_item, RID p_material) override {}
	void canvas_item_set_use_parent_material(RID p_item, bool p_enable) override {}
	RID canvas_light_create() override { return RID(); }
	void canvas_light_attach_to_canvas(RID p_light, RID p_canvas) override {}
	void canvas_light_set_enabled(RID p_light, bool p_enabled) override {}
	void canvas_light_set_scale(RID p_light, float p_scale) override {}
	void canvas_light_set_transform(RID p_light, const Transform2D &p_transform) override {}
	void canvas_light_set_texture(RID p_light, RID p_texture) override {}
	void canvas_light_set_texture_offset(RID p_light, const Vector2 &p_offset) override {}
	void canvas_light_set_color(RID p_light, const Color &p_color) override {}
	void canvas_light_set_height(RID p_light, float p_height) override {}
	void canvas_light_set_energy(RID p_light, float p_energy) override {}
	void canvas_light_set_z_range(RID p_light, int p_min_z, int p_max_z) override {}
	void canvas_light_set_layer_range(RID p_light, int p_min_layer, int p_max_layer) override {}
	void canvas_light_set_item_cull_mask(RID p_light, int p_mask) override {}
	void canvas_light_set_item_shadow_cull_mask(RID p_light, int p_mask) override {}
	void canvas_light_set_mode(RID p_light, CanvasLightMode p_mode) override {}
	void canvas_light_set_shadow_enabled(RID p_light, bool p_enabled) override {}
	void canvas_light_set_shadow_buffer_size(RID p_light, int p_size) override {}
	void canvas_light_set_shadow_gradient_length(RID p_light, float p_length) override {}
	void canvas_light_set_shadow_filter(RID p_light, CanvasLightShadowFilter p_filter) override {}
	void canvas_light_set_shadow_color(RID p_light, const Color &p_color) override {}
	void canvas_light_set_shadow_smooth(RID p_light, float p_smooth) override {}
	RID canvas_light_occluder_create() override { return RID(); }
	void canvas_light_occluder_attach_to_canvas(RID p_occluder, RID p_canvas) override {}
	void canvas_light_occluder_set_enabled(RID p_occluder, bool p_enabled) override {}
	void canvas_light_occluder_set_polygon(RID p_occluder, RID p_polygon) override {}
	void canvas_light_occluder_set_transform(RID p_occluder, const Transform2D &p_xform) override {}
	void canvas_light_occluder_set_light_mask(RID p_occluder, int p_mask) override {}
	RID canvas_occluder_polygon_create() override { return RID(); }
	void canvas_occluder_polygon_set_shape(RID p_occluder_polygon, const PoolVector<Vector2> &p_shape, bool p_closed) override {}
	void canvas_occluder_polygon_set_shape_as_lines(RID p_occluder_polygon, const PoolVector<Vector2> &p_shape) override {}
	void canvas_occluder_polygon_set_cull_mode(RID p_occluder_polygon, CanvasOccluderPolygonCullMode p_mode) override {}

	// CUSTOM TITLE BAR
	void custom_title_bar_set_image(RID p_texture) override {}

	// BLACK BARS
	void black_bars_set_margins(int p_left, int p_top, int p_right, int p_bottom) override {}
	void black_bars_set_images(RID p_left, RID p_top, RID p_right, RID p_bottom) override {}

	// FREE
	void free(RID p_rid) override {
		if (texture_owner.owns(p_rid)) {
			// delete the texture
			DummyTexture *texture = texture_owner.get(p_rid);
			texture_owner.free(p_rid);
			memdelete(texture);
		} else if (mesh_owner.owns(p_rid)) {
			// delete the mesh
			DummyMesh *mesh = mesh_owner.getornull(p_rid);
			mesh_owner.free(p_rid);
			memdelete(mesh);
		}
	}

	void request_frame_drawn_callback(Object *p_where, const StringName &p_method, const Variant &p_userdata) override {}

	// EVENT QUEUING
	void draw(bool p_swap_buffers = true, double frame_step = 0.0) override {}
	void sync() override {}
	bool has_changed() const override { return false; }
	void init() override {}
	void finish() override {}

	// STATUS INFORMATION
	uint64_t get_render_info(RenderInfo p_info) override { return 0; }
	String get_video_adapter_name() const override { return String(); }
	String get_video_adapter_vendor() const override { return String(); }

	// TESTING
	RID get_test_cube() override { return RID(); }

	void set_boot_image(const Ref<Image> &p_image, const Color &p_color, bool p_scale, bool p_use_filter = true) override {}
	void set_default_clear_color(const Color &p_color) override {}
	void set_shader_time_scale(float p_scale) override {}

	bool has_feature(Features p_feature) const override { return false; }
	bool has_os_feature(const String &p_feature) const override { return false; }
	void set_debug_generate_wireframes(bool p_generate) override {}
	void call_set_use_vsync(bool p_enable) override {}
	bool is_low_end() const override { return false; }
};

#endif
