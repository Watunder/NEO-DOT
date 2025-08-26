/*************************************************************************/
/*  visual_server_dummy.h                                                */
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

#ifndef VISUAL_SERVER_DUMMY_H
#define VISUAL_SERVER_DUMMY_H

#include "drivers/dummy/rasterizer_dummy.h"
#include "servers/visual_server.h"

class VisualServerCanvasDummy {
public:
	RID canvas_create() { return RID(); }
	void canvas_set_item_mirroring(RID p_canvas, RID p_item, const Point2 &p_mirroring) {}
	void canvas_set_modulate(RID p_canvas, const Color &p_color) {}
	void canvas_set_parent(RID p_canvas, RID p_parent, float p_scale) {}
	void canvas_set_disable_scale(bool p_disable) {}

	RID canvas_item_create() { return RID(); }
	void canvas_item_set_parent(RID p_item, RID p_parent) {}
	void canvas_item_set_visible(RID p_item, bool p_visible) {}
	void canvas_item_set_light_mask(RID p_item, int p_mask) {}
	void canvas_item_set_transform(RID p_item, const Transform2D &p_transform) {}
	void canvas_item_set_clip(RID p_item, bool p_clip) {}
	void canvas_item_set_distance_field_mode(RID p_item, bool p_enable) {}
	void canvas_item_set_custom_rect(RID p_item, bool p_custom_rect, const Rect2 &p_rect = Rect2()) {}
	void canvas_item_set_modulate(RID p_item, const Color &p_color) {}
	void canvas_item_set_self_modulate(RID p_item, const Color &p_color) {}
	void canvas_item_set_draw_behind_parent(RID p_item, bool p_enable) {}
	void canvas_item_set_update_when_visible(RID p_item, bool p_update) {}
	void canvas_item_add_line(RID p_item, const Point2 &p_from, const Point2 &p_to, const Color &p_color, float p_width = 1.0, bool p_antialiased = false) {}
	void canvas_item_add_polyline(RID p_item, const Vector<Point2> &p_points, const Vector<Color> &p_colors, float p_width = 1.0, bool p_antialiased = false) {}
	void canvas_item_add_multiline(RID p_item, const Vector<Point2> &p_points, const Vector<Color> &p_colors, float p_width = 1.0, bool p_antialiased = false) {}
	void canvas_item_add_rect(RID p_item, const Rect2 &p_rect, const Color &p_color) {}
	void canvas_item_add_circle(RID p_item, const Point2 &p_pos, float p_radius, const Color &p_color) {}
	void canvas_item_add_texture_rect(RID p_item, const Rect2 &p_rect, RID p_texture, bool p_tile = false, const Color &p_modulate = Color(1, 1, 1), bool p_transpose = false, RID p_normal_map = RID()) {}
	void canvas_item_add_texture_rect_region(RID p_item, const Rect2 &p_rect, RID p_texture, const Rect2 &p_src_rect, const Color &p_modulate = Color(1, 1, 1), bool p_transpose = false, RID p_normal_map = RID(), bool p_clip_uv = false) {}
	void canvas_item_add_nine_patch(RID p_item, const Rect2 &p_rect, const Rect2 &p_source, RID p_texture, const Vector2 &p_topleft, const Vector2 &p_bottomright, VS::NinePatchAxisMode p_x_axis_mode = VS::NINE_PATCH_STRETCH, VS::NinePatchAxisMode p_y_axis_mode = VS::NINE_PATCH_STRETCH, bool p_draw_center = true, const Color &p_modulate = Color(1, 1, 1), RID p_normal_map = RID()) {}
	void canvas_item_add_primitive(RID p_item, const Vector<Point2> &p_points, const Vector<Color> &p_colors, const Vector<Point2> &p_uvs, RID p_texture, float p_width = 1.0, RID p_normal_map = RID()) {}
	void canvas_item_add_polygon(RID p_item, const Vector<Point2> &p_points, const Vector<Color> &p_colors, const Vector<Point2> &p_uvs = Vector<Point2>(), RID p_texture = RID(), RID p_normal_map = RID(), bool p_antialiased = false) {}
	void canvas_item_add_triangle_array(RID p_item, const Vector<int> &p_indices, const Vector<Point2> &p_points, const Vector<Color> &p_colors, const Vector<Point2> &p_uvs = Vector<Point2>(), const Vector<int> &p_bones = Vector<int>(), const Vector<float> &p_weights = Vector<float>(), RID p_texture = RID(), int p_count = -1, RID p_normal_map = RID(), bool p_antialiased = false, bool p_antialiasing_use_indices = false) {}
	void canvas_item_add_mesh(RID p_item, const RID &p_mesh, const Transform2D &p_transform = Transform2D(), const Color &p_modulate = Color(1, 1, 1), RID p_texture = RID(), RID p_normal_map = RID()) {}
	void canvas_item_add_multimesh(RID p_item, RID p_mesh, RID p_texture = RID(), RID p_normal_map = RID()) {}
	void canvas_item_add_particles(RID p_item, RID p_particles, RID p_texture, RID p_normal) {}
	void canvas_item_add_set_transform(RID p_item, const Transform2D &p_transform) {}
	void canvas_item_add_clip_ignore(RID p_item, bool p_ignore) {}
	void canvas_item_set_sort_children_by_y(RID p_item, bool p_enable) {}
	void canvas_item_set_z_index(RID p_item, int p_z) {}
	void canvas_item_set_z_as_relative_to_parent(RID p_item, bool p_enable) {}
	void canvas_item_set_copy_to_backbuffer(RID p_item, bool p_enable, const Rect2 &p_rect) {}
	void canvas_item_attach_skeleton(RID p_item, RID p_skeleton) {}
	void canvas_item_clear(RID p_item) {}
	void canvas_item_set_draw_index(RID p_item, int p_index) {}
	void canvas_item_set_material(RID p_item, RID p_material) {}
	void canvas_item_set_use_parent_material(RID p_item, bool p_enable) {}

	RID canvas_light_create() { return RID(); }
	void canvas_light_attach_to_canvas(RID p_light, RID p_canvas) {}
	void canvas_light_set_enabled(RID p_light, bool p_enabled) {}
	void canvas_light_set_scale(RID p_light, float p_scale) {}
	void canvas_light_set_transform(RID p_light, const Transform2D &p_transform) {}
	void canvas_light_set_texture(RID p_light, RID p_texture) {}
	void canvas_light_set_texture_offset(RID p_light, const Vector2 &p_offset) {}
	void canvas_light_set_color(RID p_light, const Color &p_color) {}
	void canvas_light_set_height(RID p_light, float p_height) {}
	void canvas_light_set_energy(RID p_light, float p_energy) {}
	void canvas_light_set_z_range(RID p_light, int p_min_z, int p_max_z) {}
	void canvas_light_set_layer_range(RID p_light, int p_min_layer, int p_max_layer) {}
	void canvas_light_set_item_cull_mask(RID p_light, int p_mask) {}
	void canvas_light_set_item_shadow_cull_mask(RID p_light, int p_mask) {}
	void canvas_light_set_mode(RID p_light, VS::CanvasLightMode p_mode) {}
	void canvas_light_set_shadow_enabled(RID p_light, bool p_enabled) {}
	void canvas_light_set_shadow_buffer_size(RID p_light, int p_size) {}
	void canvas_light_set_shadow_gradient_length(RID p_light, float p_length) {}
	void canvas_light_set_shadow_filter(RID p_light, VS::CanvasLightShadowFilter p_filter) {}
	void canvas_light_set_shadow_color(RID p_light, const Color &p_color) {}
	void canvas_light_set_shadow_smooth(RID p_light, float p_smooth) {}

	RID canvas_light_occluder_create() { return RID(); }
	void canvas_light_occluder_attach_to_canvas(RID p_occluder, RID p_canvas) {}
	void canvas_light_occluder_set_enabled(RID p_occluder, bool p_enabled) {}
	void canvas_light_occluder_set_polygon(RID p_occluder, RID p_polygon) {}
	void canvas_light_occluder_set_transform(RID p_occluder, const Transform2D &p_xform) {}
	void canvas_light_occluder_set_light_mask(RID p_occluder, int p_mask) {}

	RID canvas_occluder_polygon_create() { return RID(); }
	void canvas_occluder_polygon_set_shape(RID p_occluder_polygon, const PoolVector<Vector2> &p_shape, bool p_closed) {}
	void canvas_occluder_polygon_set_shape_as_lines(RID p_occluder_polygon, const PoolVector<Vector2> &p_shape) {}
	void canvas_occluder_polygon_set_cull_mode(RID p_occluder_polygon, VS::CanvasOccluderPolygonCullMode p_mode) {}
};

class VisualServerViewportDummy {
public:
	RID viewport_create() { return RID(); }
	void viewport_set_use_arvr(RID p_viewport, bool p_use_arvr) {}
	void viewport_set_size(RID p_viewport, int p_width, int p_height) {}
	void viewport_attach_to_screen(RID p_viewport, const Rect2 &p_rect = Rect2(), int p_screen = 0) {}
	void viewport_set_render_direct_to_screen(RID p_viewport, bool p_enable) {}
	void viewport_detach(RID p_viewport) {}
	void viewport_set_active(RID p_viewport, bool p_active) {}
	void viewport_set_parent_viewport(RID p_viewport, RID p_parent_viewport) {}
	void viewport_set_update_mode(RID p_viewport, VS::ViewportUpdateMode p_mode) {}
	void viewport_set_vflip(RID p_viewport, bool p_enable) {}
	void viewport_set_clear_mode(RID p_viewport, VS::ViewportClearMode p_clear_mode) {}
	RID viewport_get_texture(RID p_viewport) const { return RID(); }
	void viewport_set_hide_scenario(RID p_viewport, bool p_hide) {}
	void viewport_set_hide_canvas(RID p_viewport, bool p_hide) {}
	void viewport_set_disable_environment(RID p_viewport, bool p_disable) {}
	void viewport_set_disable_3d(RID p_viewport, bool p_disable) {}
	void viewport_set_keep_3d_linear(RID p_viewport, bool p_keep_3d_linear) {}
	void viewport_attach_camera(RID p_viewport, RID p_camera) {}
	void viewport_set_scenario(RID p_viewport, RID p_scenario) {}
	void viewport_attach_canvas(RID p_viewport, RID p_canvas) {}
	void viewport_remove_canvas(RID p_viewport, RID p_canvas) {}
	void viewport_set_canvas_transform(RID p_viewport, RID p_canvas, const Transform2D &p_offset) {}
	void viewport_set_transparent_background(RID p_viewport, bool p_enabled) {}
	void viewport_set_global_canvas_transform(RID p_viewport, const Transform2D &p_transform) {}
	void viewport_set_canvas_stacking(RID p_viewport, RID p_canvas, int p_layer, int p_sublayer) {}
	void viewport_set_shadow_atlas_size(RID p_viewport, int p_size) {}
	void viewport_set_shadow_atlas_quadrant_subdivision(RID p_viewport, int p_quadrant, int p_subdiv) {}
	void viewport_set_msaa(RID p_viewport, VS::ViewportMSAA p_msaa) {}
	void viewport_set_use_fxaa(RID p_viewport, bool p_fxaa) {}
	void viewport_set_use_debanding(RID p_viewport, bool p_debanding) {}
	void viewport_set_hdr(RID p_viewport, bool p_enabled) {}
	void viewport_set_usage(RID p_viewport, VS::ViewportUsage p_usage) {}
	int viewport_get_render_info(RID p_viewport, VS::ViewportRenderInfo p_info) { return 0; }
	void viewport_set_debug_draw(RID p_viewport, VS::ViewportDebugDraw p_draw) {}
};

class VisualServerSceneDummy {
public:
	RID camera_create() { return RID(); }
	void camera_set_perspective(RID p_camera, float p_fovy_degrees, float p_z_near, float p_z_far) {}
	void camera_set_orthogonal(RID p_camera, float p_size, float p_z_near, float p_z_far) {}
	void camera_set_frustum(RID p_camera, float p_size, Vector2 p_offset, float p_z_near, float p_z_far) {}
	void camera_set_transform(RID p_camera, const Transform &p_transform) {}
	void camera_set_cull_mask(RID p_camera, uint32_t p_layers) {}
	void camera_set_environment(RID p_camera, RID p_env) {}
	void camera_set_use_vertical_aspect(RID p_camera, bool p_enable) {}

	RID scenario_create() { return RID(); }
	void scenario_set_debug(RID p_scenario, VS::ScenarioDebugMode p_debug_mode) {}
	void scenario_set_environment(RID p_scenario, RID p_environment) {}
	void scenario_set_fallback_environment(RID p_scenario, RID p_environment) {}
	void scenario_set_reflection_atlas_size(RID p_scenario, int p_size, int p_subdiv) {}

	RID instance_create() { return RID(); }
	void instance_set_base(RID p_instance, RID p_base) {}
	void instance_set_scenario(RID p_instance, RID p_scenario) {}
	void instance_set_layer_mask(RID p_instance, uint32_t p_mask) {}
	void instance_set_transform(RID p_instance, const Transform &p_transform) {}
	void instance_attach_object_instance_id(RID p_instance, ObjectID p_id) {}
	void instance_set_blend_shape_weight(RID p_instance, int p_shape, float p_weight) {}
	void instance_set_surface_material(RID p_instance, int p_surface, RID p_material) {}
	void instance_set_visible(RID p_instance, bool p_visible) {}
	void instance_set_use_lightmap(RID p_instance, RID p_lightmap_instance, RID p_lightmap, int p_lightmap_slice, const Rect2 &p_lightmap_uv_rect) {}
	void instance_set_custom_aabb(RID p_instance, AABB p_aabb) {}
	void instance_attach_skeleton(RID p_instance, RID p_skeleton) {}
	void instance_set_exterior(RID p_instance, bool p_enabled) {}
	void instance_set_extra_visibility_margin(RID p_instance, real_t p_margin) {}
	Vector<ObjectID> instances_cull_aabb(const AABB &p_aabb, RID p_scenario = RID()) const { return Vector<ObjectID>(); }
	Vector<ObjectID> instances_cull_ray(const Vector3 &p_from, const Vector3 &p_to, RID p_scenario = RID()) const { return Vector<ObjectID>(); }
	Vector<ObjectID> instances_cull_convex(const Vector<Plane> &p_convex, RID p_scenario = RID()) const { return Vector<ObjectID>(); }
	void instance_geometry_set_flag(RID p_instance, VS::InstanceFlags p_flags, bool p_enabled) {}
	void instance_geometry_set_cast_shadows_setting(RID p_instance, VS::ShadowCastingSetting p_shadow_casting_setting) {}
	void instance_geometry_set_material_override(RID p_instance, RID p_material) {}
	void instance_geometry_set_draw_range(RID p_instance, float p_min, float p_max, float p_min_margin, float p_max_margin) {}
	void instance_geometry_set_as_instance_lod(RID p_instance, RID p_as_lod_of_instance) {}
};

class VisualServerGlobalsDummy {
public:
	static RasterizerStorage *storage;
	static RasterizerCanvas *canvas_render;
	static RasterizerScene *scene_render;
	static Rasterizer *rasterizer;

	static VisualServerCanvasDummy *canvas;
	static VisualServerViewportDummy *viewport;
	static VisualServerSceneDummy *scene;
};

RasterizerStorage *VisualServerGlobalsDummy::storage = NULL;
RasterizerCanvas *VisualServerGlobalsDummy::canvas_render = NULL;
RasterizerScene *VisualServerGlobalsDummy::scene_render = NULL;
Rasterizer *VisualServerGlobalsDummy::rasterizer = NULL;
VisualServerCanvasDummy *VisualServerGlobalsDummy::canvas = NULL;
VisualServerViewportDummy *VisualServerGlobalsDummy::viewport = NULL;
VisualServerSceneDummy *VisualServerGlobalsDummy::scene = NULL;

#define VSG_DUMMY VisualServerGlobalsDummy

class VisualServerDummy : public VisualServer {
	static int changes;

public:
#define DISPLAY_CHANGED_DUMMY \
	changes++;

#define BIND0R_DUMMY(m_r, m_name) \
	m_r m_name() { return BINDBASE_DUMMY->m_name(); }
#define BIND1R_DUMMY(m_r, m_name, m_type1) \
	m_r m_name(m_type1 arg1) { return BINDBASE_DUMMY->m_name(arg1); }
#define BIND1RC_DUMMY(m_r, m_name, m_type1) \
	m_r m_name(m_type1 arg1) const { return BINDBASE_DUMMY->m_name(arg1); }
#define BIND2R_DUMMY(m_r, m_name, m_type1, m_type2) \
	m_r m_name(m_type1 arg1, m_type2 arg2) { return BINDBASE_DUMMY->m_name(arg1, arg2); }
#define BIND2RC_DUMMY(m_r, m_name, m_type1, m_type2) \
	m_r m_name(m_type1 arg1, m_type2 arg2) const { return BINDBASE_DUMMY->m_name(arg1, arg2); }
#define BIND3RC_DUMMY(m_r, m_name, m_type1, m_type2, m_type3) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3) const { return BINDBASE_DUMMY->m_name(arg1, arg2, arg3); }
#define BIND4RC_DUMMY(m_r, m_name, m_type1, m_type2, m_type3, m_type4) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4) const { return BINDBASE_DUMMY->m_name(arg1, arg2, arg3, arg4); }

#define BIND1_DUMMY(m_name, m_type1) \
	void m_name(m_type1 arg1) { DISPLAY_CHANGED_DUMMY BINDBASE_DUMMY->m_name(arg1); }
#define BIND2_DUMMY(m_name, m_type1, m_type2) \
	void m_name(m_type1 arg1, m_type2 arg2) { DISPLAY_CHANGED_DUMMY BINDBASE_DUMMY->m_name(arg1, arg2); }
#define BIND2C_DUMMY(m_name, m_type1, m_type2) \
	void m_name(m_type1 arg1, m_type2 arg2) const { BINDBASE_DUMMY->m_name(arg1, arg2); }
#define BIND3_DUMMY(m_name, m_type1, m_type2, m_type3) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3) { DISPLAY_CHANGED_DUMMY BINDBASE_DUMMY->m_name(arg1, arg2, arg3); }
#define BIND4_DUMMY(m_name, m_type1, m_type2, m_type3, m_type4) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4) { DISPLAY_CHANGED_DUMMY BINDBASE_DUMMY->m_name(arg1, arg2, arg3, arg4); }
#define BIND5_DUMMY(m_name, m_type1, m_type2, m_type3, m_type4, m_type5) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5) { DISPLAY_CHANGED_DUMMY BINDBASE_DUMMY->m_name(arg1, arg2, arg3, arg4, arg5); }
#define BIND6_DUMMY(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6) { DISPLAY_CHANGED_DUMMY BINDBASE_DUMMY->m_name(arg1, arg2, arg3, arg4, arg5, arg6); }
#define BIND7_DUMMY(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7) { DISPLAY_CHANGED_DUMMY BINDBASE_DUMMY->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7); }
#define BIND8_DUMMY(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8) { DISPLAY_CHANGED_DUMMY BINDBASE_DUMMY->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8); }
#define BIND9_DUMMY(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9) { DISPLAY_CHANGED_DUMMY BINDBASE_DUMMY->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9); }
#define BIND10_DUMMY(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9, m_type10) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9, m_type10 arg10) { DISPLAY_CHANGED_DUMMY BINDBASE_DUMMY->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10); }
#define BIND11_DUMMY(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9, m_type10, m_type11) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9, m_type10 arg10, m_type11 arg11) { DISPLAY_CHANGED_DUMMY BINDBASE_DUMMY->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11); }
#define BIND12_DUMMY(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9, m_type10, m_type11, m_type12) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9, m_type10 arg10, m_type11 arg11, m_type12 arg12) { DISPLAY_CHANGED_DUMMY BINDBASE_DUMMY->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12); }
#define BIND13_DUMMY(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9, m_type10, m_type11, m_type12, m_type13) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9, m_type10 arg10, m_type11 arg11, m_type12 arg12, m_type13 arg13) { DISPLAY_CHANGED_DUMMY BINDBASE_DUMMY->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13); }

//from now on, calls forwarded to this singleton
#define BINDBASE_DUMMY VSG_DUMMY::storage

	/* TEXTURE API */

	BIND0R_DUMMY(RID, texture_create)
	BIND7_DUMMY(texture_allocate, RID, int, int, int, Image::Format, TextureType, uint32_t)
	BIND3_DUMMY(texture_set_data, RID, const Ref<Image> &, int)
	BIND10_DUMMY(texture_set_data_partial, RID, const Ref<Image> &, int, int, int, int, int, int, int, int)
	BIND2RC_DUMMY(Ref<Image>, texture_get_data, RID, int)
	BIND2_DUMMY(texture_set_flags, RID, uint32_t)
	BIND1RC_DUMMY(uint32_t, texture_get_flags, RID)
	BIND1RC_DUMMY(Image::Format, texture_get_format, RID)
	BIND1RC_DUMMY(TextureType, texture_get_type, RID)
	BIND1RC_DUMMY(uint32_t, texture_get_texid, RID)
	BIND1RC_DUMMY(uint32_t, texture_get_width, RID)
	BIND1RC_DUMMY(uint32_t, texture_get_height, RID)
	BIND1RC_DUMMY(uint32_t, texture_get_depth, RID)
	BIND4_DUMMY(texture_set_size_override, RID, int, int, int)
	BIND2_DUMMY(texture_bind, RID, uint32_t)

	BIND3_DUMMY(texture_set_detect_3d_callback, RID, TextureDetectCallback, void *)
	BIND3_DUMMY(texture_set_detect_srgb_callback, RID, TextureDetectCallback, void *)
	BIND3_DUMMY(texture_set_detect_normal_callback, RID, TextureDetectCallback, void *)

	BIND2_DUMMY(texture_set_path, RID, const String &)
	BIND1RC_DUMMY(String, texture_get_path, RID)
	BIND1_DUMMY(texture_set_shrink_all_x2_on_set_data, bool)
	BIND1_DUMMY(texture_debug_usage, List<TextureInfo> *)

	BIND1_DUMMY(textures_keep_original, bool)

	BIND2_DUMMY(texture_set_proxy, RID, RID)

	BIND2_DUMMY(texture_set_force_redraw_if_visible, RID, bool)

	/* SKY API */

	BIND0R_DUMMY(RID, sky_create)
	BIND3_DUMMY(sky_set_texture, RID, RID, int)

	/* SHADER API */

	BIND0R_DUMMY(RID, shader_create)

	BIND2_DUMMY(shader_set_code, RID, const String &)
	BIND1RC_DUMMY(String, shader_get_code, RID)

	BIND2C_DUMMY(shader_get_param_list, RID, List<PropertyInfo> *)

	BIND3_DUMMY(shader_set_default_texture_param, RID, const StringName &, RID)
	BIND2RC_DUMMY(RID, shader_get_default_texture_param, RID, const StringName &)

	BIND2_DUMMY(shader_add_custom_define, RID, const String &)
	BIND2C_DUMMY(shader_get_custom_defines, RID, Vector<String> *)
	BIND2_DUMMY(shader_remove_custom_define, RID, const String &)

	/* COMMON MATERIAL API */

	BIND0R_DUMMY(RID, material_create)

	BIND2_DUMMY(material_set_shader, RID, RID)
	BIND1RC_DUMMY(RID, material_get_shader, RID)

	BIND3_DUMMY(material_set_param, RID, const StringName &, const Variant &)
	BIND2RC_DUMMY(Variant, material_get_param, RID, const StringName &)
	BIND2RC_DUMMY(Variant, material_get_param_default, RID, const StringName &)

	BIND2_DUMMY(material_set_render_priority, RID, int)
	BIND2_DUMMY(material_set_line_width, RID, float)
	BIND2_DUMMY(material_set_next_pass, RID, RID)

	/* MESH API */

	BIND0R_DUMMY(RID, mesh_create)

	BIND10_DUMMY(mesh_add_surface, RID, uint32_t, PrimitiveType, const PoolVector<uint8_t> &, int, const PoolVector<uint8_t> &, int, const AABB &, const Vector<PoolVector<uint8_t>> &, const Vector<AABB> &)

	BIND2_DUMMY(mesh_set_blend_shape_count, RID, int)
	BIND1RC_DUMMY(int, mesh_get_blend_shape_count, RID)

	BIND2_DUMMY(mesh_set_blend_shape_mode, RID, BlendShapeMode)
	BIND1RC_DUMMY(BlendShapeMode, mesh_get_blend_shape_mode, RID)

	BIND4_DUMMY(mesh_surface_update_region, RID, int, int, const PoolVector<uint8_t> &)

	BIND3_DUMMY(mesh_surface_set_material, RID, int, RID)
	BIND2RC_DUMMY(RID, mesh_surface_get_material, RID, int)

	BIND2RC_DUMMY(int, mesh_surface_get_array_len, RID, int)
	BIND2RC_DUMMY(int, mesh_surface_get_array_index_len, RID, int)

	BIND2RC_DUMMY(PoolVector<uint8_t>, mesh_surface_get_array, RID, int)
	BIND2RC_DUMMY(PoolVector<uint8_t>, mesh_surface_get_index_array, RID, int)

	BIND2RC_DUMMY(uint32_t, mesh_surface_get_format, RID, int)
	BIND2RC_DUMMY(PrimitiveType, mesh_surface_get_primitive_type, RID, int)

	BIND2RC_DUMMY(AABB, mesh_surface_get_aabb, RID, int)
	BIND2RC_DUMMY(Vector<PoolVector<uint8_t>>, mesh_surface_get_blend_shapes, RID, int)
	BIND2RC_DUMMY(Vector<AABB>, mesh_surface_get_skeleton_aabb, RID, int)

	BIND2_DUMMY(mesh_remove_surface, RID, int)
	BIND1RC_DUMMY(int, mesh_get_surface_count, RID)

	BIND2_DUMMY(mesh_set_custom_aabb, RID, const AABB &)
	BIND1RC_DUMMY(AABB, mesh_get_custom_aabb, RID)

	BIND1_DUMMY(mesh_clear, RID)

	/* MULTIMESH API */

	BIND0R_DUMMY(RID, multimesh_create)

	BIND5_DUMMY(multimesh_allocate, RID, int, MultimeshTransformFormat, MultimeshColorFormat, MultimeshCustomDataFormat)
	BIND1RC_DUMMY(int, multimesh_get_instance_count, RID)

	BIND2_DUMMY(multimesh_set_mesh, RID, RID)
	BIND3_DUMMY(multimesh_instance_set_transform, RID, int, const Transform &)
	BIND3_DUMMY(multimesh_instance_set_transform_2d, RID, int, const Transform2D &)
	BIND3_DUMMY(multimesh_instance_set_color, RID, int, const Color &)
	BIND3_DUMMY(multimesh_instance_set_custom_data, RID, int, const Color &)

	BIND1RC_DUMMY(RID, multimesh_get_mesh, RID)
	BIND1RC_DUMMY(AABB, multimesh_get_aabb, RID)

	BIND2RC_DUMMY(Transform, multimesh_instance_get_transform, RID, int)
	BIND2RC_DUMMY(Transform2D, multimesh_instance_get_transform_2d, RID, int)
	BIND2RC_DUMMY(Color, multimesh_instance_get_color, RID, int)
	BIND2RC_DUMMY(Color, multimesh_instance_get_custom_data, RID, int)

	BIND2_DUMMY(multimesh_set_as_bulk_array, RID, const PoolVector<float> &)

	BIND2_DUMMY(multimesh_set_visible_instances, RID, int)
	BIND1RC_DUMMY(int, multimesh_get_visible_instances, RID)

	/* IMMEDIATE API */

	BIND0R_DUMMY(RID, immediate_create)
	BIND3_DUMMY(immediate_begin, RID, PrimitiveType, RID)
	BIND2_DUMMY(immediate_vertex, RID, const Vector3 &)
	BIND2_DUMMY(immediate_normal, RID, const Vector3 &)
	BIND2_DUMMY(immediate_tangent, RID, const Plane &)
	BIND2_DUMMY(immediate_color, RID, const Color &)
	BIND2_DUMMY(immediate_uv, RID, const Vector2 &)
	BIND2_DUMMY(immediate_uv2, RID, const Vector2 &)
	BIND1_DUMMY(immediate_end, RID)
	BIND1_DUMMY(immediate_clear, RID)
	BIND2_DUMMY(immediate_set_material, RID, RID)
	BIND1RC_DUMMY(RID, immediate_get_material, RID)

	/* SKELETON API */

	BIND0R_DUMMY(RID, skeleton_create)
	BIND3_DUMMY(skeleton_allocate, RID, int, bool)
	BIND1RC_DUMMY(int, skeleton_get_bone_count, RID)
	BIND3_DUMMY(skeleton_bone_set_transform, RID, int, const Transform &)
	BIND2RC_DUMMY(Transform, skeleton_bone_get_transform, RID, int)
	BIND3_DUMMY(skeleton_bone_set_transform_2d, RID, int, const Transform2D &)
	BIND2RC_DUMMY(Transform2D, skeleton_bone_get_transform_2d, RID, int)
	BIND2_DUMMY(skeleton_set_base_transform_2d, RID, const Transform2D &)

	/* Light API */

	BIND0R_DUMMY(RID, directional_light_create)
	BIND0R_DUMMY(RID, omni_light_create)
	BIND0R_DUMMY(RID, spot_light_create)

	BIND2_DUMMY(light_set_color, RID, const Color &)
	BIND3_DUMMY(light_set_param, RID, LightParam, float)
	BIND2_DUMMY(light_set_shadow, RID, bool)
	BIND2_DUMMY(light_set_shadow_color, RID, const Color &)
	BIND2_DUMMY(light_set_projector, RID, RID)
	BIND2_DUMMY(light_set_negative, RID, bool)
	BIND2_DUMMY(light_set_cull_mask, RID, uint32_t)
	BIND2_DUMMY(light_set_reverse_cull_face_mode, RID, bool)
	BIND2_DUMMY(light_set_use_gi, RID, bool)
	BIND2_DUMMY(light_set_bake_mode, RID, LightBakeMode)

	BIND2_DUMMY(light_omni_set_shadow_mode, RID, LightOmniShadowMode)
	BIND2_DUMMY(light_omni_set_shadow_detail, RID, LightOmniShadowDetail)

	BIND2_DUMMY(light_directional_set_shadow_mode, RID, LightDirectionalShadowMode)
	BIND2_DUMMY(light_directional_set_blend_splits, RID, bool)
	BIND2_DUMMY(light_directional_set_shadow_depth_range_mode, RID, LightDirectionalShadowDepthRangeMode)

	/* PROBE API */

	BIND0R_DUMMY(RID, reflection_probe_create)

	BIND2_DUMMY(reflection_probe_set_update_mode, RID, ReflectionProbeUpdateMode)
	BIND2_DUMMY(reflection_probe_set_intensity, RID, float)
	BIND2_DUMMY(reflection_probe_set_interior_ambient, RID, const Color &)
	BIND2_DUMMY(reflection_probe_set_interior_ambient_energy, RID, float)
	BIND2_DUMMY(reflection_probe_set_interior_ambient_probe_contribution, RID, float)
	BIND2_DUMMY(reflection_probe_set_max_distance, RID, float)
	BIND2_DUMMY(reflection_probe_set_extents, RID, const Vector3 &)
	BIND2_DUMMY(reflection_probe_set_origin_offset, RID, const Vector3 &)
	BIND2_DUMMY(reflection_probe_set_as_interior, RID, bool)
	BIND2_DUMMY(reflection_probe_set_enable_box_projection, RID, bool)
	BIND2_DUMMY(reflection_probe_set_enable_shadows, RID, bool)
	BIND2_DUMMY(reflection_probe_set_cull_mask, RID, uint32_t)
	BIND2_DUMMY(reflection_probe_set_resolution, RID, int)

	/* BAKED LIGHT API */

	BIND0R_DUMMY(RID, gi_probe_create)

	BIND2_DUMMY(gi_probe_set_bounds, RID, const AABB &)
	BIND1RC_DUMMY(AABB, gi_probe_get_bounds, RID)

	BIND2_DUMMY(gi_probe_set_cell_size, RID, float)
	BIND1RC_DUMMY(float, gi_probe_get_cell_size, RID)

	BIND2_DUMMY(gi_probe_set_to_cell_xform, RID, const Transform &)
	BIND1RC_DUMMY(Transform, gi_probe_get_to_cell_xform, RID)

	BIND2_DUMMY(gi_probe_set_dynamic_range, RID, int)
	BIND1RC_DUMMY(int, gi_probe_get_dynamic_range, RID)

	BIND2_DUMMY(gi_probe_set_energy, RID, float)
	BIND1RC_DUMMY(float, gi_probe_get_energy, RID)

	BIND2_DUMMY(gi_probe_set_bias, RID, float)
	BIND1RC_DUMMY(float, gi_probe_get_bias, RID)

	BIND2_DUMMY(gi_probe_set_normal_bias, RID, float)
	BIND1RC_DUMMY(float, gi_probe_get_normal_bias, RID)

	BIND2_DUMMY(gi_probe_set_propagation, RID, float)
	BIND1RC_DUMMY(float, gi_probe_get_propagation, RID)

	BIND2_DUMMY(gi_probe_set_interior, RID, bool)
	BIND1RC_DUMMY(bool, gi_probe_is_interior, RID)

	BIND2_DUMMY(gi_probe_set_compress, RID, bool)
	BIND1RC_DUMMY(bool, gi_probe_is_compressed, RID)

	BIND2_DUMMY(gi_probe_set_dynamic_data, RID, const PoolVector<int> &)
	BIND1RC_DUMMY(PoolVector<int>, gi_probe_get_dynamic_data, RID)

	/* LIGHTMAP CAPTURE */

	BIND0R_DUMMY(RID, lightmap_capture_create)

	BIND2_DUMMY(lightmap_capture_set_bounds, RID, const AABB &)
	BIND1RC_DUMMY(AABB, lightmap_capture_get_bounds, RID)

	BIND2_DUMMY(lightmap_capture_set_octree, RID, const PoolVector<uint8_t> &)
	BIND1RC_DUMMY(PoolVector<uint8_t>, lightmap_capture_get_octree, RID)

	BIND2_DUMMY(lightmap_capture_set_octree_cell_transform, RID, const Transform &)
	BIND1RC_DUMMY(Transform, lightmap_capture_get_octree_cell_transform, RID)
	BIND2_DUMMY(lightmap_capture_set_octree_cell_subdiv, RID, int)
	BIND1RC_DUMMY(int, lightmap_capture_get_octree_cell_subdiv, RID)

	BIND2_DUMMY(lightmap_capture_set_energy, RID, float)
	BIND1RC_DUMMY(float, lightmap_capture_get_energy, RID)

	BIND2_DUMMY(lightmap_capture_set_interior, RID, bool)
	BIND1RC_DUMMY(bool, lightmap_capture_is_interior, RID)

	/* PARTICLES */

	BIND0R_DUMMY(RID, particles_create)

	BIND2_DUMMY(particles_set_emitting, RID, bool)
	BIND1R_DUMMY(bool, particles_get_emitting, RID)
	BIND2_DUMMY(particles_set_amount, RID, int)
	BIND2_DUMMY(particles_set_lifetime, RID, float)
	BIND2_DUMMY(particles_set_one_shot, RID, bool)
	BIND2_DUMMY(particles_set_pre_process_time, RID, float)
	BIND2_DUMMY(particles_set_explosiveness_ratio, RID, float)
	BIND2_DUMMY(particles_set_randomness_ratio, RID, float)
	BIND2_DUMMY(particles_set_custom_aabb, RID, const AABB &)
	BIND2_DUMMY(particles_set_speed_scale, RID, float)
	BIND2_DUMMY(particles_set_use_local_coordinates, RID, bool)
	BIND2_DUMMY(particles_set_process_material, RID, RID)
	BIND2_DUMMY(particles_set_fixed_fps, RID, int)
	BIND2_DUMMY(particles_set_fractional_delta, RID, bool)
	BIND1R_DUMMY(bool, particles_is_inactive, RID)
	BIND1_DUMMY(particles_request_process, RID)
	BIND1_DUMMY(particles_restart, RID)

	BIND2_DUMMY(particles_set_draw_order, RID, ParticlesDrawOrder)

	BIND2_DUMMY(particles_set_draw_passes, RID, int)
	BIND3_DUMMY(particles_set_draw_pass_mesh, RID, int, RID)

	BIND1R_DUMMY(AABB, particles_get_current_aabb, RID)
	BIND2_DUMMY(particles_set_emission_transform, RID, const Transform &)

#undef BINDBASE_DUMMY
//from now on, calls forwarded to this singleton
#define BINDBASE_DUMMY VSG_DUMMY::scene

	/* CAMERA API */

	BIND0R_DUMMY(RID, camera_create)
	BIND4_DUMMY(camera_set_perspective, RID, float, float, float)
	BIND4_DUMMY(camera_set_orthogonal, RID, float, float, float)
	BIND5_DUMMY(camera_set_frustum, RID, float, Vector2, float, float)
	BIND2_DUMMY(camera_set_transform, RID, const Transform &)
	BIND2_DUMMY(camera_set_cull_mask, RID, uint32_t)
	BIND2_DUMMY(camera_set_environment, RID, RID)
	BIND2_DUMMY(camera_set_use_vertical_aspect, RID, bool)

#undef BINDBASE_DUMMY
//from now on, calls forwarded to this singleton
#define BINDBASE_DUMMY VSG_DUMMY::viewport

	/* VIEWPORT TARGET API */

	BIND0R_DUMMY(RID, viewport_create)

	BIND2_DUMMY(viewport_set_use_arvr, RID, bool)
	BIND3_DUMMY(viewport_set_size, RID, int, int)

	BIND2_DUMMY(viewport_set_active, RID, bool)
	BIND2_DUMMY(viewport_set_parent_viewport, RID, RID)

	BIND2_DUMMY(viewport_set_clear_mode, RID, ViewportClearMode)

	BIND3_DUMMY(viewport_attach_to_screen, RID, const Rect2 &, int)
	BIND2_DUMMY(viewport_set_render_direct_to_screen, RID, bool)
	BIND1_DUMMY(viewport_detach, RID)

	BIND2_DUMMY(viewport_set_update_mode, RID, ViewportUpdateMode)
	BIND2_DUMMY(viewport_set_vflip, RID, bool)

	BIND1RC_DUMMY(RID, viewport_get_texture, RID)

	BIND2_DUMMY(viewport_set_hide_scenario, RID, bool)
	BIND2_DUMMY(viewport_set_hide_canvas, RID, bool)
	BIND2_DUMMY(viewport_set_disable_environment, RID, bool)
	BIND2_DUMMY(viewport_set_disable_3d, RID, bool)
	BIND2_DUMMY(viewport_set_keep_3d_linear, RID, bool)

	BIND2_DUMMY(viewport_attach_camera, RID, RID)
	BIND2_DUMMY(viewport_set_scenario, RID, RID)
	BIND2_DUMMY(viewport_attach_canvas, RID, RID)

	BIND2_DUMMY(viewport_remove_canvas, RID, RID)
	BIND3_DUMMY(viewport_set_canvas_transform, RID, RID, const Transform2D &)
	BIND2_DUMMY(viewport_set_transparent_background, RID, bool)

	BIND2_DUMMY(viewport_set_global_canvas_transform, RID, const Transform2D &)
	BIND4_DUMMY(viewport_set_canvas_stacking, RID, RID, int, int)
	BIND2_DUMMY(viewport_set_shadow_atlas_size, RID, int)
	BIND3_DUMMY(viewport_set_shadow_atlas_quadrant_subdivision, RID, int, int)
	BIND2_DUMMY(viewport_set_msaa, RID, ViewportMSAA)
	BIND2_DUMMY(viewport_set_use_fxaa, RID, bool)
	BIND2_DUMMY(viewport_set_use_debanding, RID, bool)
	BIND2_DUMMY(viewport_set_hdr, RID, bool)
	BIND2_DUMMY(viewport_set_usage, RID, ViewportUsage)

	BIND2R_DUMMY(int, viewport_get_render_info, RID, ViewportRenderInfo)
	BIND2_DUMMY(viewport_set_debug_draw, RID, ViewportDebugDraw)

	/* ENVIRONMENT API */

#undef BINDBASE_DUMMY
//from now on, calls forwarded to this singleton
#define BINDBASE_DUMMY VSG_DUMMY::scene_render

	BIND0R_DUMMY(RID, environment_create)

	BIND2_DUMMY(environment_set_background, RID, EnvironmentBG)
	BIND2_DUMMY(environment_set_sky, RID, RID)
	BIND2_DUMMY(environment_set_sky_custom_fov, RID, float)
	BIND2_DUMMY(environment_set_sky_orientation, RID, const Basis &)
	BIND2_DUMMY(environment_set_bg_color, RID, const Color &)
	BIND2_DUMMY(environment_set_bg_energy, RID, float)
	BIND2_DUMMY(environment_set_canvas_max_layer, RID, int)
	BIND4_DUMMY(environment_set_ambient_light, RID, const Color &, float, float)
	BIND2_DUMMY(environment_set_camera_feed_id, RID, int)
	BIND7_DUMMY(environment_set_ssr, RID, bool, int, float, float, float, bool)
	BIND13_DUMMY(environment_set_ssao, RID, bool, float, float, float, float, float, float, float, const Color &, EnvironmentSSAOQuality, EnvironmentSSAOBlur, float)

	BIND6_DUMMY(environment_set_dof_blur_near, RID, bool, float, float, float, EnvironmentDOFBlurQuality)
	BIND6_DUMMY(environment_set_dof_blur_far, RID, bool, float, float, float, EnvironmentDOFBlurQuality)
	BIND11_DUMMY(environment_set_glow, RID, bool, int, float, float, float, EnvironmentGlowBlendMode, float, float, float, bool)

	BIND9_DUMMY(environment_set_tonemap, RID, EnvironmentToneMapper, float, float, bool, float, float, float, float)

	BIND6_DUMMY(environment_set_adjustment, RID, bool, float, float, float, RID)

	BIND5_DUMMY(environment_set_fog, RID, bool, const Color &, const Color &, float)
	BIND7_DUMMY(environment_set_fog_depth, RID, bool, float, float, float, bool, float)
	BIND5_DUMMY(environment_set_fog_height, RID, bool, float, float, float)

	/* SCENARIO API */

#undef BINDBASE_DUMMY
#define BINDBASE_DUMMY VSG_DUMMY::scene

	BIND0R_DUMMY(RID, scenario_create)

	BIND2_DUMMY(scenario_set_debug, RID, ScenarioDebugMode)
	BIND2_DUMMY(scenario_set_environment, RID, RID)
	BIND3_DUMMY(scenario_set_reflection_atlas_size, RID, int, int)
	BIND2_DUMMY(scenario_set_fallback_environment, RID, RID)

	/* INSTANCING API */
	BIND0R_DUMMY(RID, instance_create)

	BIND2_DUMMY(instance_set_base, RID, RID)
	BIND2_DUMMY(instance_set_scenario, RID, RID)
	BIND2_DUMMY(instance_set_layer_mask, RID, uint32_t)
	BIND2_DUMMY(instance_set_transform, RID, const Transform &)
	BIND2_DUMMY(instance_attach_object_instance_id, RID, ObjectID)
	BIND3_DUMMY(instance_set_blend_shape_weight, RID, int, float)
	BIND3_DUMMY(instance_set_surface_material, RID, int, RID)
	BIND2_DUMMY(instance_set_visible, RID, bool)
	BIND5_DUMMY(instance_set_use_lightmap, RID, RID, RID, int, const Rect2 &)

	BIND2_DUMMY(instance_set_custom_aabb, RID, AABB)

	BIND2_DUMMY(instance_attach_skeleton, RID, RID)
	BIND2_DUMMY(instance_set_exterior, RID, bool)

	BIND2_DUMMY(instance_set_extra_visibility_margin, RID, real_t)

	// don't use these in a game!
	BIND2RC_DUMMY(Vector<ObjectID>, instances_cull_aabb, const AABB &, RID)
	BIND3RC_DUMMY(Vector<ObjectID>, instances_cull_ray, const Vector3 &, const Vector3 &, RID)
	BIND2RC_DUMMY(Vector<ObjectID>, instances_cull_convex, const Vector<Plane> &, RID)

	BIND3_DUMMY(instance_geometry_set_flag, RID, InstanceFlags, bool)
	BIND2_DUMMY(instance_geometry_set_cast_shadows_setting, RID, ShadowCastingSetting)
	BIND2_DUMMY(instance_geometry_set_material_override, RID, RID)

	BIND5_DUMMY(instance_geometry_set_draw_range, RID, float, float, float, float)
	BIND2_DUMMY(instance_geometry_set_as_instance_lod, RID, RID)

#undef BINDBASE_DUMMY
//from now on, calls forwarded to this singleton
#define BINDBASE_DUMMY VSG_DUMMY::canvas

	/* CANVAS (2D) */

	BIND0R_DUMMY(RID, canvas_create)
	BIND3_DUMMY(canvas_set_item_mirroring, RID, RID, const Point2 &)
	BIND2_DUMMY(canvas_set_modulate, RID, const Color &)
	BIND3_DUMMY(canvas_set_parent, RID, RID, float)
	BIND1_DUMMY(canvas_set_disable_scale, bool)

	BIND0R_DUMMY(RID, canvas_item_create)
	BIND2_DUMMY(canvas_item_set_parent, RID, RID)

	BIND2_DUMMY(canvas_item_set_visible, RID, bool)
	BIND2_DUMMY(canvas_item_set_light_mask, RID, int)

	BIND2_DUMMY(canvas_item_set_update_when_visible, RID, bool)

	BIND2_DUMMY(canvas_item_set_transform, RID, const Transform2D &)
	BIND2_DUMMY(canvas_item_set_clip, RID, bool)
	BIND2_DUMMY(canvas_item_set_distance_field_mode, RID, bool)
	BIND3_DUMMY(canvas_item_set_custom_rect, RID, bool, const Rect2 &)
	BIND2_DUMMY(canvas_item_set_modulate, RID, const Color &)
	BIND2_DUMMY(canvas_item_set_self_modulate, RID, const Color &)

	BIND2_DUMMY(canvas_item_set_draw_behind_parent, RID, bool)

	BIND6_DUMMY(canvas_item_add_line, RID, const Point2 &, const Point2 &, const Color &, float, bool)
	BIND5_DUMMY(canvas_item_add_polyline, RID, const Vector<Point2> &, const Vector<Color> &, float, bool)
	BIND5_DUMMY(canvas_item_add_multiline, RID, const Vector<Point2> &, const Vector<Color> &, float, bool)
	BIND3_DUMMY(canvas_item_add_rect, RID, const Rect2 &, const Color &)
	BIND4_DUMMY(canvas_item_add_circle, RID, const Point2 &, float, const Color &)
	BIND7_DUMMY(canvas_item_add_texture_rect, RID, const Rect2 &, RID, bool, const Color &, bool, RID)
	BIND8_DUMMY(canvas_item_add_texture_rect_region, RID, const Rect2 &, RID, const Rect2 &, const Color &, bool, RID, bool)
	BIND11_DUMMY(canvas_item_add_nine_patch, RID, const Rect2 &, const Rect2 &, RID, const Vector2 &, const Vector2 &, NinePatchAxisMode, NinePatchAxisMode, bool, const Color &, RID)
	BIND7_DUMMY(canvas_item_add_primitive, RID, const Vector<Point2> &, const Vector<Color> &, const Vector<Point2> &, RID, float, RID)
	BIND7_DUMMY(canvas_item_add_polygon, RID, const Vector<Point2> &, const Vector<Color> &, const Vector<Point2> &, RID, RID, bool)
	BIND12_DUMMY(canvas_item_add_triangle_array, RID, const Vector<int> &, const Vector<Point2> &, const Vector<Color> &, const Vector<Point2> &, const Vector<int> &, const Vector<float> &, RID, int, RID, bool, bool)
	BIND6_DUMMY(canvas_item_add_mesh, RID, const RID &, const Transform2D &, const Color &, RID, RID)
	BIND4_DUMMY(canvas_item_add_multimesh, RID, RID, RID, RID)
	BIND4_DUMMY(canvas_item_add_particles, RID, RID, RID, RID)
	BIND2_DUMMY(canvas_item_add_set_transform, RID, const Transform2D &)
	BIND2_DUMMY(canvas_item_add_clip_ignore, RID, bool)
	BIND2_DUMMY(canvas_item_set_sort_children_by_y, RID, bool)
	BIND2_DUMMY(canvas_item_set_z_index, RID, int)
	BIND2_DUMMY(canvas_item_set_z_as_relative_to_parent, RID, bool)
	BIND3_DUMMY(canvas_item_set_copy_to_backbuffer, RID, bool, const Rect2 &)
	BIND2_DUMMY(canvas_item_attach_skeleton, RID, RID)

	BIND1_DUMMY(canvas_item_clear, RID)
	BIND2_DUMMY(canvas_item_set_draw_index, RID, int)

	BIND2_DUMMY(canvas_item_set_material, RID, RID)

	BIND2_DUMMY(canvas_item_set_use_parent_material, RID, bool)

	BIND0R_DUMMY(RID, canvas_light_create)
	BIND2_DUMMY(canvas_light_attach_to_canvas, RID, RID)
	BIND2_DUMMY(canvas_light_set_enabled, RID, bool)
	BIND2_DUMMY(canvas_light_set_scale, RID, float)
	BIND2_DUMMY(canvas_light_set_transform, RID, const Transform2D &)
	BIND2_DUMMY(canvas_light_set_texture, RID, RID)
	BIND2_DUMMY(canvas_light_set_texture_offset, RID, const Vector2 &)
	BIND2_DUMMY(canvas_light_set_color, RID, const Color &)
	BIND2_DUMMY(canvas_light_set_height, RID, float)
	BIND2_DUMMY(canvas_light_set_energy, RID, float)
	BIND3_DUMMY(canvas_light_set_z_range, RID, int, int)
	BIND3_DUMMY(canvas_light_set_layer_range, RID, int, int)
	BIND2_DUMMY(canvas_light_set_item_cull_mask, RID, int)
	BIND2_DUMMY(canvas_light_set_item_shadow_cull_mask, RID, int)

	BIND2_DUMMY(canvas_light_set_mode, RID, CanvasLightMode)

	BIND2_DUMMY(canvas_light_set_shadow_enabled, RID, bool)
	BIND2_DUMMY(canvas_light_set_shadow_buffer_size, RID, int)
	BIND2_DUMMY(canvas_light_set_shadow_gradient_length, RID, float)
	BIND2_DUMMY(canvas_light_set_shadow_filter, RID, CanvasLightShadowFilter)
	BIND2_DUMMY(canvas_light_set_shadow_color, RID, const Color &)
	BIND2_DUMMY(canvas_light_set_shadow_smooth, RID, float)

	BIND0R_DUMMY(RID, canvas_light_occluder_create)
	BIND2_DUMMY(canvas_light_occluder_attach_to_canvas, RID, RID)
	BIND2_DUMMY(canvas_light_occluder_set_enabled, RID, bool)
	BIND2_DUMMY(canvas_light_occluder_set_polygon, RID, RID)
	BIND2_DUMMY(canvas_light_occluder_set_transform, RID, const Transform2D &)
	BIND2_DUMMY(canvas_light_occluder_set_light_mask, RID, int)

	BIND0R_DUMMY(RID, canvas_occluder_polygon_create)
	BIND3_DUMMY(canvas_occluder_polygon_set_shape, RID, const PoolVector<Vector2> &, bool)
	BIND2_DUMMY(canvas_occluder_polygon_set_shape_as_lines, RID, const PoolVector<Vector2> &)

	BIND2_DUMMY(canvas_occluder_polygon_set_cull_mode, RID, CanvasOccluderPolygonCullMode)

	/* CUSTOM TITLE BAR */

	void custom_title_bar_set_image(RID p_texture) override {}

	/* BLACK BARS */

	void black_bars_set_margins(int p_left, int p_top, int p_right, int p_bottom) override {}
	void black_bars_set_images(RID p_left, RID p_top, RID p_right, RID p_bottom) override {}

	/* FREE */

	void free(RID p_rid) override {
		if (VSG_DUMMY::storage->free(p_rid))
			return;
	}

	/* EVENT QUEUING */

	void request_frame_drawn_callback(Object *p_where, const StringName &p_method, const Variant &p_userdata) override {}

	void draw(bool p_swap_buffers = true, double frame_step = 0.0) override {
		get_singleton()->emit_signal("frame_pre_draw");

		changes = 0;

		get_singleton()->emit_signal("frame_post_draw");
	}

	void sync() override {}

	bool has_changed() const override {
		return changes > 0;
	}

	void init() override {}
	void finish() override {}

	/* STATUS INFORMATION */

	uint64_t get_render_info(RenderInfo p_info) override { return 0; }
	String get_video_adapter_name() const override { return String(); }
	String get_video_adapter_vendor() const override { return String(); }

	RID get_test_cube() override { return RID(); }

	/* TESTING */

	void set_boot_image(const Ref<Image> &p_image, const Color &p_color, bool p_scale, bool p_use_filter = true) override {}

	void set_default_clear_color(const Color &p_color) override {}
	void set_shader_time_scale(float p_scale) override {}

	bool has_feature(Features p_feature) const override { return false; }

	bool has_os_feature(const String &p_feature) const override { return false; }
	void set_debug_generate_wireframes(bool p_generate) override {}

	void call_set_use_vsync(bool p_enable) override {}

	bool is_low_end() const override { return false; }

	VisualServerDummy() {
		VSG_DUMMY::canvas = memnew(VisualServerCanvasDummy);
		VSG_DUMMY::viewport = memnew(VisualServerViewportDummy);
		VSG_DUMMY::scene = memnew(VisualServerSceneDummy);
		VSG_DUMMY::rasterizer = RasterizerDummy::create();
		VSG_DUMMY::storage = VSG_DUMMY::rasterizer->get_storage();
		VSG_DUMMY::canvas_render = VSG_DUMMY::rasterizer->get_canvas();
		VSG_DUMMY::scene_render = VSG_DUMMY::rasterizer->get_scene();
	}

	~VisualServerDummy() {
		memdelete(VSG_DUMMY::canvas);
		memdelete(VSG_DUMMY::viewport);
		memdelete(VSG_DUMMY::rasterizer);
		memdelete(VSG_DUMMY::scene);
	}

#undef DISPLAY_CHANGED_DUMMY

#undef BIND0R_DUMMY
#undef BIND1RC_DUMMY
#undef BIND2RC_DUMMY
#undef BIND3RC_DUMMY
#undef BIND4RC_DUMMY

#undef BIND1_DUMMY
#undef BIND2_DUMMY
#undef BIND3_DUMMY
#undef BIND4_DUMMY
#undef BIND5_DUMMY
#undef BIND6_DUMMY
#undef BIND7_DUMMY
#undef BIND8_DUMMY
#undef BIND9_DUMMY
#undef BIND10_DUMMY
};

int VisualServerDummy::changes = 0;

#endif
