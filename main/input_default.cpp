/*************************************************************************/
/*  input_default.cpp                                                    */
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

#include "input_default.h"

#include "core/input_map.h"
#include "core/os/os.h"
#include "main/default_controller_mappings.h"
#include "scene/resources/texture.h"
#include "servers/visual_server.h"

void InputDefault::SpeedTrack::update(const Vector2 &p_delta_p) {
	uint64_t tick = OS::get_singleton()->get_ticks_usec();
	uint32_t tdiff = tick - last_tick;
	float delta_t = tdiff / 1000000.0;
	last_tick = tick;

	accum += p_delta_p;
	accum_t += delta_t;

	if (accum_t > max_ref_frame * 10)
		accum_t = max_ref_frame * 10;

	while (accum_t >= min_ref_frame) {
		float slice_t = min_ref_frame / accum_t;
		Vector2 slice = accum * slice_t;
		accum = accum - slice;
		accum_t -= min_ref_frame;

		speed = (slice / min_ref_frame).linear_interpolate(speed, min_ref_frame / max_ref_frame);
	}
}

void InputDefault::SpeedTrack::reset() {
	last_tick = OS::get_singleton()->get_ticks_usec();
	speed = Vector2();
	accum_t = 0;
}

InputDefault::SpeedTrack::SpeedTrack() {
	min_ref_frame = 0.1;
	max_ref_frame = 0.3;
	reset();
}

bool InputDefault::is_key_pressed(int p_scancode) const {
	_THREAD_SAFE_METHOD_
	return keys_pressed.has(p_scancode);
}

bool InputDefault::is_mouse_button_pressed(int p_button) const {
	_THREAD_SAFE_METHOD_
	return (mouse_button_mask & (1 << (p_button - 1))) != 0;
}

static int _combine_device(int p_value, int p_device) {
	return p_value | (p_device << 20);
}

bool InputDefault::is_joy_button_pressed(int p_device, int p_button) const {
	_THREAD_SAFE_METHOD_
	return joy_buttons_pressed.has(_combine_device(p_button, p_device));
}

bool InputDefault::is_action_pressed(const StringName &p_action) const {
#ifdef DEBUG_ENABLED
	bool has_action = InputMap::get_singleton()->has_action(p_action);
	ERR_FAIL_COND_V_MSG(!has_action, false, "Request for nonexistent InputMap action '" + String(p_action) + "'.");
#endif
	return action_state.has(p_action) && action_state[p_action].pressed;
}

bool InputDefault::is_action_just_pressed(const StringName &p_action) const {
#ifdef DEBUG_ENABLED
	bool has_action = InputMap::get_singleton()->has_action(p_action);
	ERR_FAIL_COND_V_MSG(!has_action, false, "Request for nonexistent InputMap action '" + String(p_action) + "'.");
#endif
	const Map<StringName, Action>::Element *E = action_state.find(p_action);
	if (!E)
		return false;

	if (Engine::get_singleton()->is_in_physics_frame()) {
		return E->get().pressed && E->get().physics_frame == Engine::get_singleton()->get_physics_frames();
	} else {
		return E->get().pressed && E->get().idle_frame == Engine::get_singleton()->get_idle_frames();
	}
}

bool InputDefault::is_action_just_released(const StringName &p_action) const {
#ifdef DEBUG_ENABLED
	bool has_action = InputMap::get_singleton()->has_action(p_action);
	ERR_FAIL_COND_V_MSG(!has_action, false, "Request for nonexistent InputMap action '" + String(p_action) + "'.");
#endif
	const Map<StringName, Action>::Element *E = action_state.find(p_action);
	if (!E)
		return false;

	if (Engine::get_singleton()->is_in_physics_frame()) {
		return !E->get().pressed && E->get().physics_frame == Engine::get_singleton()->get_physics_frames();
	} else {
		return !E->get().pressed && E->get().idle_frame == Engine::get_singleton()->get_idle_frames();
	}
}

float InputDefault::get_action_strength(const StringName &p_action) const {
#ifdef DEBUG_ENABLED
	bool has_action = InputMap::get_singleton()->has_action(p_action);
	ERR_FAIL_COND_V_MSG(!has_action, false, "Request for nonexistent InputMap action '" + String(p_action) + "'.");
#endif
	const Map<StringName, Action>::Element *E = action_state.find(p_action);
	if (!E)
		return 0.0f;

	return E->get().strength;
}

float InputDefault::get_joy_axis(int p_device, int p_axis) const {
	_THREAD_SAFE_METHOD_
	int c = _combine_device(p_axis, p_device);
	if (_joy_axis.has(c)) {
		return _joy_axis[c];
	} else {
		return 0;
	}
}

String InputDefault::get_joy_name(int p_idx) {
	_THREAD_SAFE_METHOD_
	return joy_names[p_idx].name;
};

Vector2 InputDefault::get_joy_vibration_strength(int p_device) {
	if (joy_vibration.has(p_device)) {
		return Vector2(joy_vibration[p_device].weak_magnitude, joy_vibration[p_device].strong_magnitude);
	} else {
		return Vector2(0, 0);
	}
}

uint64_t InputDefault::get_joy_vibration_timestamp(int p_device) {
	if (joy_vibration.has(p_device)) {
		return joy_vibration[p_device].timestamp;
	} else {
		return 0;
	}
}

float InputDefault::get_joy_vibration_duration(int p_device) {
	if (joy_vibration.has(p_device)) {
		return joy_vibration[p_device].duration;
	} else {
		return 0.f;
	}
}

static String _hex_str(uint8_t p_byte) {
	static const char *dict = "0123456789abcdef";
	char ret[3];
	ret[2] = 0;

	ret[0] = dict[p_byte >> 4];
	ret[1] = dict[p_byte & 0xf];

	return ret;
};

void InputDefault::joy_connection_changed(int p_idx, bool p_connected, String p_name, String p_guid) {
	_THREAD_SAFE_METHOD_
	Joypad js;
	js.name = p_connected ? p_name : "";
	js.uid = p_connected ? p_guid : "";

	if (p_connected) {
		String uidname = p_guid;
		if (p_guid == "") {
			int uidlen = MIN(p_name.length(), 16);
			for (int i = 0; i < uidlen; i++) {
				uidname = uidname + _hex_str(p_name[i]);
			};
		};
		js.uid = uidname;
		js.connected = true;
		int mapping = fallback_mapping;
		for (int i = 0; i < map_db.size(); i++) {
			if (js.uid == map_db[i].uid) {
				mapping = i;
				js.name = map_db[i].name;
			};
		};
		js.mapping = mapping;
	} else {
		js.connected = false;
		for (int i = 0; i < JOY_BUTTON_MAX; i++) {
			if (i < JOY_AXIS_MAX)
				set_joy_axis(p_idx, i, 0.0f);

			int c = _combine_device(i, p_idx);
			joy_buttons_pressed.erase(c);
		};
	};
	joy_names[p_idx] = js;

	emit_signal("joy_connection_changed", p_idx, p_connected);
};

Vector3 InputDefault::get_gravity() const {
	_THREAD_SAFE_METHOD_
	return gravity;
}

Vector3 InputDefault::get_accelerometer() const {
	_THREAD_SAFE_METHOD_
	return accelerometer;
}

Vector3 InputDefault::get_magnetometer() const {
	_THREAD_SAFE_METHOD_
	return magnetometer;
}

Vector3 InputDefault::get_gyroscope() const {
	_THREAD_SAFE_METHOD_
	return gyroscope;
}

void InputDefault::parse_input_event(const Ref<InputEvent> &p_event) {
	_parse_input_event_impl(p_event, false);
}

void InputDefault::_parse_input_event_impl(const Ref<InputEvent> &p_event, bool p_is_emulated) {
	// Notes on mouse-touch emulation:
	// - Emulated mouse events are parsed, that is, re-routed to this method, so they make the same effects
	//   as true mouse events. The only difference is the situation is flagged as emulated so they are not
	//   emulated back to touch events in an endless loop.
	// - Emulated touch events are handed right to the main loop (i.e., the SceneTree) because they don't
	//   require additional handling by this class.

	_THREAD_SAFE_METHOD_

	Ref<InputEventKey> k = p_event;
	if (k.is_valid() && !k->is_echo() && k->get_scancode() != 0) {
		if (k->is_pressed())
			keys_pressed.insert(k->get_scancode());
		else
			keys_pressed.erase(k->get_scancode());
	}

	Ref<InputEventMouseButton> mb = p_event;

	if (mb.is_valid()) {
		if (mb->is_pressed()) {
			mouse_button_mask |= (1 << (mb->get_button_index() - 1));
		} else {
			mouse_button_mask &= ~(1 << (mb->get_button_index() - 1));
		}

		Point2 pos = mb->get_global_position();
		if (mouse_pos != pos) {
			set_mouse_position(pos);
		}

		if (main_loop && emulate_touch_from_mouse && !p_is_emulated && mb->get_button_index() == 1) {
			Ref<InputEventScreenTouch> touch_event;
			touch_event.instance();
			touch_event->set_pressed(mb->is_pressed());
			touch_event->set_position(mb->get_position());
			main_loop->input_event(touch_event);
		}
	}

	Ref<InputEventMouseMotion> mm = p_event;

	if (mm.is_valid()) {
		Point2 pos = mm->get_global_position();
		if (mouse_pos != pos) {
			set_mouse_position(pos);
		}

		if (main_loop && emulate_touch_from_mouse && !p_is_emulated && mm->get_button_mask() & 1) {
			Ref<InputEventScreenDrag> drag_event;
			drag_event.instance();

			drag_event->set_position(mm->get_position());
			drag_event->set_relative(mm->get_relative());
			drag_event->set_speed(mm->get_speed());

			main_loop->input_event(drag_event);
		}
	}

	Ref<InputEventScreenTouch> st = p_event;

	if (st.is_valid()) {
		if (st->is_pressed()) {
			SpeedTrack &track = touch_speed_track[st->get_index()];
			track.reset();
		} else {
			// Since a pointer index may not occur again (OSs may or may not reuse them),
			// imperatively remove it from the map to keep no fossil entries in it
			touch_speed_track.erase(st->get_index());
		}

		if (emulate_mouse_from_touch) {
			bool translate = false;
			if (st->is_pressed()) {
				if (mouse_from_touch_index == -1) {
					translate = true;
					mouse_from_touch_index = st->get_index();
				}
			} else {
				if (st->get_index() == mouse_from_touch_index) {
					translate = true;
					mouse_from_touch_index = -1;
				}
			}

			if (translate) {
				Ref<InputEventMouseButton> button_event;
				button_event.instance();

				button_event->set_device(InputEvent::DEVICE_ID_TOUCH_MOUSE);
				button_event->set_position(st->get_position());
				button_event->set_global_position(st->get_position());
				button_event->set_pressed(st->is_pressed());
				button_event->set_button_index(BUTTON_LEFT);
				if (st->is_pressed()) {
					button_event->set_button_mask(mouse_button_mask | (1 << (BUTTON_LEFT - 1)));
				} else {
					button_event->set_button_mask(mouse_button_mask & ~(1 << (BUTTON_LEFT - 1)));
				}

				_parse_input_event_impl(button_event, true);
			}
		}
	}

	Ref<InputEventScreenDrag> sd = p_event;

	if (sd.is_valid()) {
		SpeedTrack &track = touch_speed_track[sd->get_index()];
		track.update(sd->get_relative());
		sd->set_speed(track.speed);

		if (emulate_mouse_from_touch && sd->get_index() == mouse_from_touch_index) {
			Ref<InputEventMouseMotion> motion_event;
			motion_event.instance();

			motion_event->set_device(InputEvent::DEVICE_ID_TOUCH_MOUSE);
			motion_event->set_position(sd->get_position());
			motion_event->set_global_position(sd->get_position());
			motion_event->set_relative(sd->get_relative());
			motion_event->set_speed(sd->get_speed());
			motion_event->set_button_mask(mouse_button_mask);

			_parse_input_event_impl(motion_event, true);
		}
	}

	Ref<InputEventJoypadButton> jb = p_event;

	if (jb.is_valid()) {
		int c = _combine_device(jb->get_button_index(), jb->get_device());

		if (jb->is_pressed())
			joy_buttons_pressed.insert(c);
		else
			joy_buttons_pressed.erase(c);
	}

	Ref<InputEventJoypadMotion> jm = p_event;

	if (jm.is_valid()) {
		set_joy_axis(jm->get_device(), jm->get_axis(), jm->get_axis_value());
	}

	Ref<InputEventGesture> ge = p_event;

	if (ge.is_valid()) {
		if (main_loop) {
			main_loop->input_event(ge);
		}
	}

	for (const Map<StringName, InputMap::Action>::Element *E = InputMap::get_singleton()->get_action_map().front(); E; E = E->next()) {
		if (InputMap::get_singleton()->event_is_action(p_event, E->key())) {
			// Save the action's state
			if (!p_event->is_echo() && is_action_pressed(E->key()) != p_event->is_action_pressed(E->key())) {
				Action action;
				action.physics_frame = Engine::get_singleton()->get_physics_frames();
				action.idle_frame = Engine::get_singleton()->get_idle_frames();
				action.pressed = p_event->is_action_pressed(E->key());
				action.strength = 0.f;
				action_state[E->key()] = action;
			}
			action_state[E->key()].strength = p_event->get_action_strength(E->key());
		}
	}

	if (main_loop)
		main_loop->input_event(p_event);
}

void InputDefault::set_joy_axis(int p_device, int p_axis, float p_value) {
	_THREAD_SAFE_METHOD_
	int c = _combine_device(p_axis, p_device);
	_joy_axis[c] = p_value;
}

void InputDefault::start_joy_vibration(int p_device, float p_weak_magnitude, float p_strong_magnitude, float p_duration) {
	_THREAD_SAFE_METHOD_
	if (p_weak_magnitude < 0.f || p_weak_magnitude > 1.f || p_strong_magnitude < 0.f || p_strong_magnitude > 1.f) {
		return;
	}
	VibrationInfo vibration;
	vibration.weak_magnitude = p_weak_magnitude;
	vibration.strong_magnitude = p_strong_magnitude;
	vibration.duration = p_duration;
	vibration.timestamp = OS::get_singleton()->get_ticks_usec();
	joy_vibration[p_device] = vibration;
}

void InputDefault::stop_joy_vibration(int p_device) {
	_THREAD_SAFE_METHOD_
	VibrationInfo vibration;
	vibration.weak_magnitude = 0;
	vibration.strong_magnitude = 0;
	vibration.duration = 0;
	vibration.timestamp = OS::get_singleton()->get_ticks_usec();
	joy_vibration[p_device] = vibration;
}

void InputDefault::vibrate_handheld(int p_duration_ms) {
	OS::get_singleton()->vibrate_handheld(p_duration_ms);
}

void InputDefault::set_gravity(const Vector3 &p_gravity) {
	_THREAD_SAFE_METHOD_

	gravity = p_gravity;
}

void InputDefault::set_accelerometer(const Vector3 &p_accel) {
	_THREAD_SAFE_METHOD_

	accelerometer = p_accel;
}

void InputDefault::set_magnetometer(const Vector3 &p_magnetometer) {
	_THREAD_SAFE_METHOD_

	magnetometer = p_magnetometer;
}

void InputDefault::set_gyroscope(const Vector3 &p_gyroscope) {
	_THREAD_SAFE_METHOD_

	gyroscope = p_gyroscope;
}

void InputDefault::set_main_loop(MainLoop *p_main_loop) {
	main_loop = p_main_loop;
}

void InputDefault::set_mouse_position(const Point2 &p_posf) {
	mouse_speed_track.update(p_posf - mouse_pos);
	mouse_pos = p_posf;
}

Point2 InputDefault::get_mouse_position() const {
	return mouse_pos;
}
Point2 InputDefault::get_last_mouse_speed() const {
	return mouse_speed_track.speed;
}

int InputDefault::get_mouse_button_mask() const {
	return mouse_button_mask; // do not trust OS implementation, should remove it - OS::get_singleton()->get_mouse_button_state();
}

void InputDefault::warp_mouse_position(const Vector2 &p_to) {
	OS::get_singleton()->warp_mouse_position(p_to);
}

Point2i InputDefault::warp_mouse_motion(const Ref<InputEventMouseMotion> &p_motion, const Rect2 &p_rect) {
	// The relative distance reported for the next event after a warp is in the boundaries of the
	// size of the rect on that axis, but it may be greater, in which case there's not problem as fmod()
	// will warp it, but if the pointer has moved in the opposite direction between the pointer relocation
	// and the subsequent event, the reported relative distance will be less than the size of the rect
	// and thus fmod() will be disabled for handling the situation.
	// And due to this mouse warping mechanism being stateless, we need to apply some heuristics to
	// detect the warp: if the relative distance is greater than the half of the size of the relevant rect
	// (checked per each axis), it will be considered as the consequence of a former pointer warp.

	const Point2i rel_sgn(p_motion->get_relative().x >= 0.0f ? 1 : -1, p_motion->get_relative().y >= 0.0 ? 1 : -1);
	const Size2i warp_margin = p_rect.size * 0.5f;
	const Point2i rel_warped(
			Math::fmod(p_motion->get_relative().x + rel_sgn.x * warp_margin.x, p_rect.size.x) - rel_sgn.x * warp_margin.x,
			Math::fmod(p_motion->get_relative().y + rel_sgn.y * warp_margin.y, p_rect.size.y) - rel_sgn.y * warp_margin.y);

	const Point2i pos_local = p_motion->get_global_position() - p_rect.position;
	const Point2i pos_warped(Math::fposmod(pos_local.x, p_rect.size.x), Math::fposmod(pos_local.y, p_rect.size.y));
	if (pos_warped != pos_local) {
		OS::get_singleton()->warp_mouse_position(pos_warped + p_rect.position);
	}

	return rel_warped;
}

void InputDefault::iteration(float p_step) {
}

void InputDefault::action_press(const StringName &p_action, float p_strength) {
	Action action;

	action.physics_frame = Engine::get_singleton()->get_physics_frames();
	action.idle_frame = Engine::get_singleton()->get_idle_frames();
	action.pressed = true;
	action.strength = p_strength;

	action_state[p_action] = action;
}

void InputDefault::action_release(const StringName &p_action) {
	Action action;

	action.physics_frame = Engine::get_singleton()->get_physics_frames();
	action.idle_frame = Engine::get_singleton()->get_idle_frames();
	action.pressed = false;
	action.strength = 0.f;

	action_state[p_action] = action;
}

void InputDefault::set_emulate_touch_from_mouse(bool p_emulate) {
	emulate_touch_from_mouse = p_emulate;
}

bool InputDefault::is_emulating_touch_from_mouse() const {
	return emulate_touch_from_mouse;
}

// Calling this whenever the game window is focused helps unstucking the "touch mouse"
// if the OS or its abstraction class hasn't properly reported that touch pointers raised
void InputDefault::ensure_touch_mouse_raised() {
	if (mouse_from_touch_index != -1) {
		mouse_from_touch_index = -1;

		Ref<InputEventMouseButton> button_event;
		button_event.instance();

		button_event->set_device(InputEvent::DEVICE_ID_TOUCH_MOUSE);
		button_event->set_position(mouse_pos);
		button_event->set_global_position(mouse_pos);
		button_event->set_pressed(false);
		button_event->set_button_index(BUTTON_LEFT);
		button_event->set_button_mask(mouse_button_mask & ~(1 << (BUTTON_LEFT - 1)));

		_parse_input_event_impl(button_event, true);
	}
}

void InputDefault::set_emulate_mouse_from_touch(bool p_emulate) {
	emulate_mouse_from_touch = p_emulate;
}

bool InputDefault::is_emulating_mouse_from_touch() const {
	return emulate_mouse_from_touch;
}

Input::CursorShape InputDefault::get_default_cursor_shape() const {
	return default_shape;
}

void InputDefault::set_default_cursor_shape(CursorShape p_shape) {
	if (default_shape == p_shape)
		return;

	default_shape = p_shape;
	// The default shape is set in Viewport::_gui_input_event. To instantly
	// see the shape in the viewport we need to trigger a mouse motion event.
	Ref<InputEventMouseMotion> mm;
	mm.instance();
	mm->set_position(mouse_pos);
	mm->set_global_position(mouse_pos);
	parse_input_event(mm);
}

Input::CursorShape InputDefault::get_current_cursor_shape() const {
	return (Input::CursorShape)OS::get_singleton()->get_cursor_shape();
}

void InputDefault::set_custom_mouse_cursor(const RES &p_cursor, CursorShape p_shape, const Vector2 &p_hotspot) {
	if (Engine::get_singleton()->is_editor_hint())
		return;

	OS::get_singleton()->set_custom_mouse_cursor(p_cursor, (OS::CursorShape)p_shape, p_hotspot);
}

void InputDefault::accumulate_input_event(const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND(p_event.is_null());

	if (!use_accumulated_input) {
		parse_input_event(p_event);
		return;
	}
	if (!accumulated_events.empty() && accumulated_events.back()->get()->accumulate(p_event)) {
		return; //event was accumulated, exit
	}

	accumulated_events.push_back(p_event);
}
void InputDefault::flush_accumulated_events() {
	while (accumulated_events.front()) {
		parse_input_event(accumulated_events.front()->get());
		accumulated_events.pop_front();
	}
}

void InputDefault::set_use_accumulated_input(bool p_enable) {
	use_accumulated_input = p_enable;
}

void InputDefault::release_pressed_events() {
	flush_accumulated_events(); // this is needed to release actions strengths

	keys_pressed.clear();
	joy_buttons_pressed.clear();
	_joy_axis.clear();

	for (Map<StringName, InputDefault::Action>::Element *E = action_state.front(); E; E = E->next()) {
		if (E->get().pressed)
			action_release(E->key());
	}
}

InputDefault::InputDefault() {
	use_accumulated_input = true;
	mouse_button_mask = 0;
	emulate_touch_from_mouse = false;
	emulate_mouse_from_touch = false;
	mouse_from_touch_index = -1;
	main_loop = NULL;
	default_shape = CURSOR_ARROW;

	fallback_mapping = -1;

	// Parse default mappings.
	{
		int i = 0;
		while (DefaultControllerMappings::mappings[i]) {
			parse_mapping(DefaultControllerMappings::mappings[i++]);
		}
	}

	// If defined, parse SDL_GAMECONTROLLERCONFIG for possible new mappings/overrides.
	String env_mapping = OS::get_singleton()->get_environment("SDL_GAMECONTROLLERCONFIG");
	if (env_mapping != "") {
		Vector<String> entries = env_mapping.split("\n");
		for (int i = 0; i < entries.size(); i++) {
			if (entries[i] == "")
				continue;
			parse_mapping(entries[i]);
		}
	}
}

void InputDefault::joy_button(int p_device, int p_button, bool p_pressed) {
	_THREAD_SAFE_METHOD_;
	Joypad &joy = joy_names[p_device];
	//printf("got button %i, mapping is %i\n", p_button, joy.mapping);
	if (joy.last_buttons[p_button] == p_pressed) {
		return;
	}
	joy.last_buttons[p_button] = p_pressed;
	if (joy.mapping == -1) {
		_button_event(p_device, p_button, p_pressed);
		return;
	}

	JoyEvent map = _get_mapped_button_event(map_db[joy.mapping], p_button);

	if (map.type == TYPE_BUTTON) {
		//fake additional axis event for triggers
		if (map.index == JOY_L2 || map.index == JOY_R2) {
			float value = p_pressed ? 1.0f : 0.0f;
			int axis = map.index == JOY_L2 ? JOY_ANALOG_L2 : JOY_ANALOG_R2;
			_axis_event(p_device, axis, value);
		}
		_button_event(p_device, map.index, p_pressed);
		return;
	}

	if (map.type == TYPE_AXIS) {
		_axis_event(p_device, map.index, p_pressed ? map.value : 0.0);
	}
	// no event?
}

void InputDefault::joy_axis(int p_device, int p_axis, const JoyAxis &p_value) {
	_THREAD_SAFE_METHOD_;

	ERR_FAIL_INDEX(p_axis, JOY_AXIS_MAX);

	Joypad &joy = joy_names[p_device];

	if (joy.last_axis[p_axis] == p_value.value) {
		return;
	}

	//when changing direction quickly, insert fake event to release pending inputmap actions
	float last = joy.last_axis[p_axis];
	if (p_value.min == 0 && (last < 0.25 || last > 0.75) && (last - 0.5) * (p_value.value - 0.5) < 0) {
		JoyAxis jx;
		jx.min = p_value.min;
		jx.value = p_value.value < 0.5 ? 0.6 : 0.4;
		joy_axis(p_device, p_axis, jx);
	} else if (ABS(last) > 0.5 && last * p_value.value <= 0) {
		JoyAxis jx;
		jx.min = p_value.min;
		jx.value = last > 0 ? 0.1 : -0.1;
		joy_axis(p_device, p_axis, jx);
	}

	joy.last_axis[p_axis] = p_value.value;
	float val = p_value.min == 0 ? -1.0f + 2.0f * p_value.value : p_value.value;

	if (joy.mapping == -1) {
		_axis_event(p_device, p_axis, val);
		return;
	};

	JoyEvent map = _get_mapped_axis_event(map_db[joy.mapping], p_axis, val);

	if (map.type == TYPE_BUTTON) {
		//send axis event for triggers
		if (map.index == JOY_L2 || map.index == JOY_R2) {
			float value = p_value.min == 0 ? p_value.value : 0.5f + p_value.value / 2.0f;
			int axis = map.index == JOY_L2 ? JOY_ANALOG_L2 : JOY_ANALOG_R2;
			_axis_event(p_device, axis, value);
		}

		bool pressed = map.value > 0.5;
		if (pressed == joy_buttons_pressed.has(_combine_device(map.index, p_device))) {
			// Button already pressed or released; so ignore.
			return;
		}
		_button_event(p_device, map.index, pressed);

		// Ensure opposite D-Pad button is also released.
		switch (map.index) {
			case JOY_DPAD_UP:
				if (joy_buttons_pressed.has(_combine_device(JOY_DPAD_DOWN, p_device))) {
					_button_event(p_device, JOY_DPAD_DOWN, false);
				}
				break;
			case JOY_DPAD_DOWN:
				if (joy_buttons_pressed.has(_combine_device(JOY_DPAD_UP, p_device))) {
					_button_event(p_device, JOY_DPAD_UP, false);
				}
				break;
			case JOY_DPAD_LEFT:
				if (joy_buttons_pressed.has(_combine_device(JOY_DPAD_RIGHT, p_device))) {
					_button_event(p_device, JOY_DPAD_RIGHT, false);
				}
				break;
			case JOY_DPAD_RIGHT:
				if (joy_buttons_pressed.has(_combine_device(JOY_DPAD_LEFT, p_device))) {
					_button_event(p_device, JOY_DPAD_LEFT, false);
				}
				break;
			default:
				// Nothing to do.
				break;
		}
		return;
	}

	if (map.type == TYPE_AXIS) {
		_axis_event(p_device, map.index, val);
		return;
	}
	//printf("invalid mapping\n");
}

void InputDefault::joy_hat(int p_device, int p_val) {
	_THREAD_SAFE_METHOD_;
	const Joypad &joy = joy_names[p_device];

	JoyEvent map[HAT_MAX];

	map[HAT_UP].type = TYPE_BUTTON;
	map[HAT_UP].index = JOY_DPAD_UP;
	map[HAT_UP].value = 0;

	map[HAT_RIGHT].type = TYPE_BUTTON;
	map[HAT_RIGHT].index = JOY_DPAD_RIGHT;
	map[HAT_RIGHT].value = 0;

	map[HAT_DOWN].type = TYPE_BUTTON;
	map[HAT_DOWN].index = JOY_DPAD_DOWN;
	map[HAT_DOWN].value = 0;

	map[HAT_LEFT].type = TYPE_BUTTON;
	map[HAT_LEFT].index = JOY_DPAD_LEFT;
	map[HAT_LEFT].value = 0;

	if (joy.mapping != -1) {
		_get_mapped_hat_events(map_db[joy.mapping], 0, map);
	};

	int cur_val = joy_names[p_device].hat_current;

	for (int hat_direction = 0, hat_mask = 1; hat_direction < HAT_MAX; hat_direction++, hat_mask <<= 1) {
		if ((p_val & hat_mask) != (cur_val & hat_mask)) {
			if (map[hat_direction].type == TYPE_BUTTON) {
				_button_event(p_device, map[hat_direction].index, p_val & hat_mask);
			}
			if (map[hat_direction].type == TYPE_AXIS) {
				_axis_event(p_device, map[hat_direction].index, (p_val & hat_mask) ? map[hat_direction].value : 0.0);
			}
		}
	}

	joy_names[p_device].hat_current = p_val;
}

void InputDefault::_button_event(int p_device, int p_index, bool p_pressed) {
	Ref<InputEventJoypadButton> ievent;
	ievent.instance();
	ievent->set_device(p_device);
	ievent->set_button_index(p_index);
	ievent->set_pressed(p_pressed);

	parse_input_event(ievent);
}

void InputDefault::_axis_event(int p_device, int p_axis, float p_value) {
	Ref<InputEventJoypadMotion> ievent;
	ievent.instance();
	ievent->set_device(p_device);
	ievent->set_axis(p_axis);
	ievent->set_axis_value(p_value);

	parse_input_event(ievent);
};

InputDefault::JoyEvent InputDefault::_get_mapped_button_event(const JoyDeviceMapping &mapping, int p_button) {
	JoyEvent event;
	event.type = TYPE_MAX;

	for (int i = 0; i < mapping.bindings.size(); i++) {
		const JoyBinding binding = mapping.bindings[i];
		if (binding.inputType == TYPE_BUTTON && binding.input.button == p_button) {
			event.type = binding.outputType;
			switch (binding.outputType) {
				case TYPE_BUTTON:
					event.index = binding.output.button;
					return event;
				case TYPE_AXIS:
					event.index = binding.output.axis.axis;
					switch (binding.output.axis.range) {
						case POSITIVE_HALF_AXIS:
							event.value = 1;
							break;
						case NEGATIVE_HALF_AXIS:
							event.value = -1;
							break;
						case FULL_AXIS:
							// It doesn't make sense for a button to map to a full axis,
							// but keeping as a default for a trigger with a positive half-axis.
							event.value = 1;
							break;
					}
					return event;
				default:
					ERR_PRINT_ONCE("Joypad button mapping error.");
			}
		}
	}
	return event;
}

InputDefault::JoyEvent InputDefault::_get_mapped_axis_event(const JoyDeviceMapping &mapping, int p_axis, float p_value) {
	JoyEvent event;
	event.type = TYPE_MAX;

	for (int i = 0; i < mapping.bindings.size(); i++) {
		const JoyBinding binding = mapping.bindings[i];
		if (binding.inputType == TYPE_AXIS && binding.input.axis.axis == p_axis) {
			float value = p_value;
			if (binding.input.axis.invert)
				value = -value;
			if (binding.input.axis.range == FULL_AXIS ||
					(binding.input.axis.range == POSITIVE_HALF_AXIS && value > 0) ||
					(binding.input.axis.range == NEGATIVE_HALF_AXIS && value < 0)) {
				event.type = binding.outputType;
				float shifted_positive_value = 0;
				switch (binding.input.axis.range) {
					case POSITIVE_HALF_AXIS:
						shifted_positive_value = value;
						break;
					case NEGATIVE_HALF_AXIS:
						shifted_positive_value = value + 1;
						break;
					case FULL_AXIS:
						shifted_positive_value = (value + 1) / 2;
						break;
				}
				switch (binding.outputType) {
					case TYPE_BUTTON:
						event.index = binding.output.button;
						switch (binding.input.axis.range) {
							case POSITIVE_HALF_AXIS:
								event.value = shifted_positive_value;
								break;
							case NEGATIVE_HALF_AXIS:
								event.value = 1 - shifted_positive_value;
								break;
							case FULL_AXIS:
								// It doesn't make sense for a full axis to map to a button,
								// but keeping as a default for a trigger with a positive half-axis.
								event.value = (shifted_positive_value * 2) - 1;
								break;
						}
						return event;
					case TYPE_AXIS:
						event.index = binding.output.axis.axis;
						event.value = value;
						if (binding.output.axis.range != binding.input.axis.range) {
							switch (binding.output.axis.range) {
								case POSITIVE_HALF_AXIS:
									event.value = shifted_positive_value;
									break;
								case NEGATIVE_HALF_AXIS:
									event.value = shifted_positive_value - 1;
									break;
								case FULL_AXIS:
									event.value = (shifted_positive_value * 2) - 1;
									break;
							}
						}
						return event;
					default:
						ERR_PRINT_ONCE("Joypad axis mapping error.");
				}
			}
		}
	}
	return event;
}

void InputDefault::_get_mapped_hat_events(const JoyDeviceMapping &mapping, int p_hat, JoyEvent r_events[]) {
	for (int i = 0; i < mapping.bindings.size(); i++) {
		const JoyBinding binding = mapping.bindings[i];
		if (binding.inputType == TYPE_HAT && binding.input.hat.hat == p_hat) {
			int hat_direction;
			switch (binding.input.hat.hat_mask) {
				case HAT_MASK_UP:
					hat_direction = HAT_UP;
					break;
				case HAT_MASK_RIGHT:
					hat_direction = HAT_RIGHT;
					break;
				case HAT_MASK_DOWN:
					hat_direction = HAT_DOWN;
					break;
				case HAT_MASK_LEFT:
					hat_direction = HAT_LEFT;
					break;
				default:
					ERR_PRINT_ONCE("Joypad button mapping error.");
					continue;
			}

			r_events[hat_direction].type = binding.outputType;
			switch (binding.outputType) {
				case TYPE_BUTTON:
					r_events[hat_direction].index = binding.output.button;
					break;
				case TYPE_AXIS:
					r_events[hat_direction].index = binding.output.axis.axis;
					switch (binding.output.axis.range) {
						case POSITIVE_HALF_AXIS:
							r_events[hat_direction].value = 1;
							break;
						case NEGATIVE_HALF_AXIS:
							r_events[hat_direction].value = -1;
							break;
						case FULL_AXIS:
							// It doesn't make sense for a hat direction to map to a full axis,
							// but keeping as a default for a trigger with a positive half-axis.
							r_events[hat_direction].value = 1;
							break;
					}
					break;
				default:
					ERR_PRINT_ONCE("Joypad button mapping error.");
			}
		}
	}
}

// string names of the SDL buttons in the same order as input_event.h godot buttons
static const char *_joy_buttons[] = { "a", "b", "x", "y", "leftshoulder", "rightshoulder", "lefttrigger", "righttrigger", "leftstick", "rightstick", "back", "start", "dpup", "dpdown", "dpleft", "dpright", "guide", "misc1", "paddle1", "paddle2", "paddle3", "paddle4", "touchpad", nullptr };
static const char *_joy_axes[] = { "leftx", "lefty", "rightx", "righty", nullptr };

JoystickList InputDefault::_get_output_button(String output) {
	for (int i = 0; _joy_buttons[i]; i++) {
		if (output == _joy_buttons[i])
			return JoystickList(i);
	}
	return JoystickList::JOY_INVALID_OPTION;
}

JoystickList InputDefault::_get_output_axis(String output) {
	for (int i = 0; _joy_axes[i]; i++) {
		if (output == _joy_axes[i])
			return JoystickList(i);
	}
	return JoystickList::JOY_INVALID_OPTION;
}

void InputDefault::parse_mapping(String p_mapping) {
	_THREAD_SAFE_METHOD_;
	JoyDeviceMapping mapping;

	Vector<String> entry = p_mapping.split(",");
	if (entry.size() < 2) {
		return;
	}

	CharString uid;
	uid.resize(17);

	mapping.uid = entry[0];
	mapping.name = entry[1];

	int idx = 1;
	while (++idx < entry.size()) {
		if (entry[idx] == "")
			continue;

		String output = entry[idx].get_slice(":", 0).replace(" ", "");
		String input = entry[idx].get_slice(":", 1).replace(" ", "");
		ERR_CONTINUE_MSG(output.length() < 1 || input.length() < 2,
				vformat("Invalid device mapping entry \"%s\" in mapping:\n%s", entry[idx], p_mapping));

		if (output == "platform" || output == "hint")
			continue;

		JoyAxisRange output_range = FULL_AXIS;
		if (output[0] == '+' || output[0] == '-') {
			ERR_CONTINUE_MSG(output.length() < 2,
					vformat("Invalid output entry \"%s\" in mapping:\n%s", entry[idx], p_mapping));
			if (output[0] == '+')
				output_range = POSITIVE_HALF_AXIS;
			else if (output[0] == '-')
				output_range = NEGATIVE_HALF_AXIS;
			output = output.right(1);
		}

		JoyAxisRange input_range = FULL_AXIS;
		if (input[0] == '+') {
			input_range = POSITIVE_HALF_AXIS;
			input = input.right(1);
		} else if (input[0] == '-') {
			input_range = NEGATIVE_HALF_AXIS;
			input = input.right(1);
		}
		bool invert_axis = false;
		if (input[input.length() - 1] == '~') {
			invert_axis = true;
			input = input.left(input.length() - 1);
		}

		JoystickList output_button = _get_output_button(output);
		JoystickList output_axis = _get_output_axis(output);
		ERR_CONTINUE_MSG(output_button == JOY_INVALID_OPTION && output_axis == JOY_INVALID_OPTION,
				vformat("Unrecognised output string \"%s\" in mapping:\n%s", output, p_mapping));
		ERR_CONTINUE_MSG(output_button != JOY_INVALID_OPTION && output_axis != JOY_INVALID_OPTION,
				vformat("Output string \"%s\" matched both button and axis in mapping:\n%s", output, p_mapping));

		JoyBinding binding;
		if (output_button != JOY_INVALID_OPTION) {
			binding.outputType = TYPE_BUTTON;
			binding.output.button = output_button;
		} else if (output_axis != JOY_INVALID_OPTION) {
			binding.outputType = TYPE_AXIS;
			binding.output.axis.axis = output_axis;
			binding.output.axis.range = output_range;
		}

		switch (input[0]) {
			case 'b':
				binding.inputType = TYPE_BUTTON;
				binding.input.button = input.right(1).to_int();
				break;
			case 'a':
				binding.inputType = TYPE_AXIS;
				binding.input.axis.axis = input.right(1).to_int();
				binding.input.axis.range = input_range;
				binding.input.axis.invert = invert_axis;
				break;
			case 'h':
				ERR_CONTINUE_MSG(input.length() != 4 || input[2] != '.',
						vformat("Invalid had input \"%s\" in mapping:\n%s", input, p_mapping));
				binding.inputType = TYPE_HAT;
				binding.input.hat.hat = input.substr(1, 1).to_int();
				binding.input.hat.hat_mask = static_cast<HatMask>(input.right(3).to_int());
				break;
			default:
				ERR_CONTINUE_MSG(true, vformat("Unrecognized input string \"%s\" in mapping:\n%s", input, p_mapping));
		}

		mapping.bindings.push_back(binding);
	};

	map_db.push_back(mapping);
};

void InputDefault::add_joy_mapping(String p_mapping, bool p_update_existing) {
	parse_mapping(p_mapping);
	if (p_update_existing) {
		Vector<String> entry = p_mapping.split(",");
		String uid = entry[0];
		for (Map<int, Joypad>::Element *E = joy_names.front(); E; E = E->next()) {
			Joypad &joy = E->get();
			if (joy.uid == uid) {
				joy.mapping = map_db.size() - 1;
			}
		}
	}
}

void InputDefault::remove_joy_mapping(String p_guid) {
	for (int i = map_db.size() - 1; i >= 0; i--) {
		if (p_guid == map_db[i].uid) {
			map_db.remove(i);
		}
	}
	for (Map<int, Joypad>::Element *E = joy_names.front(); E; E = E->next()) {
		Joypad &joy = E->get();
		if (joy.uid == p_guid) {
			joy.mapping = -1;
		}
	}
}

void InputDefault::set_fallback_mapping(String p_guid) {
	for (int i = 0; i < map_db.size(); i++) {
		if (map_db[i].uid == p_guid) {
			fallback_mapping = i;
			return;
		}
	}
}

//Defaults to simple implementation for platforms with a fixed gamepad layout, like consoles.
bool InputDefault::is_joy_known(int p_device) {
	return OS::get_singleton()->is_joy_known(p_device);
}

String InputDefault::get_joy_guid(int p_device) const {
	return OS::get_singleton()->get_joy_guid(p_device);
}

//platforms that use the remapping system can override and call to these ones
bool InputDefault::is_joy_mapped(int p_device) {
	if (joy_names.has(p_device)) {
		int mapping = joy_names[p_device].mapping;
		if (mapping != -1 && mapping != fallback_mapping) {
			return true;
		}
	}
	return false;
}

String InputDefault::get_joy_guid_remapped(int p_device) const {
	ERR_FAIL_COND_V(!joy_names.has(p_device), "");
	return joy_names[p_device].uid;
}

Array InputDefault::get_connected_joypads() {
	Array ret;
	Map<int, Joypad>::Element *elem = joy_names.front();
	while (elem) {
		if (elem->get().connected) {
			ret.push_back(elem->key());
		}
		elem = elem->next();
	}
	return ret;
}

static const char *_buttons[JOY_BUTTON_MAX] = {
	"Face Button Bottom",
	"Face Button Right",
	"Face Button Left",
	"Face Button Top",
	"L",
	"R",
	"L2",
	"R2",
	"L3",
	"R3",
	"Select",
	"Start",
	"DPAD Up",
	"DPAD Down",
	"DPAD Left",
	"DPAD Right",
	"Guide",
	"Misc 1",
	"Paddle 1",
	"Paddle 2",
	"Paddle 3",
	"Paddle 4",
	"Touchpad",
};

static const char *_axes[JOY_AXIS_MAX] = {
	"Left Stick X",
	"Left Stick Y",
	"Right Stick X",
	"Right Stick Y",
	"",
	"",
	"L2",
	"R2",
	"",
	""
};

String InputDefault::get_joy_button_string(int p_button) {
	ERR_FAIL_INDEX_V(p_button, JOY_BUTTON_MAX, "");
	return _buttons[p_button];
}

int InputDefault::get_joy_button_index_from_string(String p_button) {
	for (int i = 0; i < JOY_BUTTON_MAX; i++) {
		if (p_button == _buttons[i]) {
			return i;
		}
	}
	ERR_FAIL_V(-1);
}

int InputDefault::get_unused_joy_id() {
	for (int i = 0; i < JOYPADS_MAX; i++) {
		if (!joy_names.has(i) || !joy_names[i].connected) {
			return i;
		}
	}
	return -1;
}

String InputDefault::get_joy_axis_string(int p_axis) {
	ERR_FAIL_INDEX_V(p_axis, JOY_AXIS_MAX, "");
	return _axes[p_axis];
}

int InputDefault::get_joy_axis_index_from_string(String p_axis) {
	for (int i = 0; i < JOY_AXIS_MAX; i++) {
		if (p_axis == _axes[i]) {
			return i;
		}
	}
	ERR_FAIL_V(-1);
}
