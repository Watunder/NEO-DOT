/*************************************************************************/
/*  audio_driver_pulseaudio.cpp                                          */
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

#include "audio_driver_pulseaudio.h"

#ifdef PULSEAUDIO_ENABLED

#include "core/os/os.h"
#include "core/project_settings.h"

#ifdef ALSAMIDI_ENABLED
#include "drivers/alsa/asound-so_wrap.h"
#endif

void AudioDriverPulseAudio::pa_state_cb(pa_context *c, void *userdata) {
	AudioDriverPulseAudio *ad = (AudioDriverPulseAudio *)userdata;

	switch (pa_context_get_state(c)) {
		case PA_CONTEXT_TERMINATED:
			print_verbose("PulseAudio: context terminated");
			ad->pa_ready = -1;
			break;
		case PA_CONTEXT_FAILED:
			print_verbose("PulseAudio: context failed");
			ad->pa_ready = -1;
			break;
		case PA_CONTEXT_READY:
			print_verbose("PulseAudio: context ready");
			ad->pa_ready = 1;
			break;
		default:
			print_verbose("PulseAudio: context other");
			// TODO: Check if we want to handle some of the other
			// PA context states like PA_CONTEXT_UNCONNECTED.
			break;
	}
}

void AudioDriverPulseAudio::pa_sink_info_cb(pa_context *c, const pa_sink_info *l, int eol, void *userdata) {
	AudioDriverPulseAudio *ad = (AudioDriverPulseAudio *)userdata;

	// If eol is set to a positive number, you're at the end of the list
	if (eol > 0) {
		return;
	}

	// If eol is set to a negative number there's an error.
	if (eol < 0) {
		ERR_PRINT("PulseAudio: sink info error: " + String(pa_strerror(pa_context_errno(c))));
		ad->pa_status--;
		return;
	}

	ad->pa_map = l->channel_map;
	ad->pa_status++;
}

void AudioDriverPulseAudio::pa_source_info_cb(pa_context *c, const pa_source_info *l, int eol, void *userdata) {
	AudioDriverPulseAudio *ad = (AudioDriverPulseAudio *)userdata;

	// If eol is set to a positive number, you're at the end of the list
	if (eol > 0) {
		return;
	}

	// If eol is set to a negative number there's an error.
	if (eol < 0) {
		ERR_PRINT("PulseAudio: sink info error: " + String(pa_strerror(pa_context_errno(c))));
		ad->pa_status--;
		return;
	}

	ad->pa_rec_map = l->channel_map;
	ad->pa_status++;
}

void AudioDriverPulseAudio::pa_server_info_cb(pa_context *c, const pa_server_info *i, void *userdata) {
	ERR_FAIL_COND_MSG(!i, "PulseAudio server info is null.");
	AudioDriverPulseAudio *ad = (AudioDriverPulseAudio *)userdata;

	ad->capture_default_device = i->default_source_name;
	ad->default_device = i->default_sink_name;
	ad->pa_status++;
}

Error AudioDriverPulseAudio::detect_channels(bool capture) {
	pa_channel_map_init_stereo(capture ? &pa_rec_map : &pa_map);

	String device = capture ? capture_device_name : device_name;
	if (device == "Default") {
		// Get the default output device name
		pa_status = 0;
		pa_operation *pa_op = pa_context_get_server_info(pa_ctx, &AudioDriverPulseAudio::pa_server_info_cb, (void *)this);
		if (pa_op) {
			while (pa_status == 0) {
				int ret = pa_mainloop_iterate(pa_ml, 1, NULL);
				if (ret < 0) {
					ERR_PRINT("pa_mainloop_iterate error");
				}
			}

			pa_operation_unref(pa_op);
		} else {
			ERR_PRINT("pa_context_get_server_info error: " + String(pa_strerror(pa_context_errno(pa_ctx))));
			return FAILED;
		}
	}

	char dev[1024];
	if (device == "Default") {
		strcpy(dev, capture ? capture_default_device.utf8().get_data() : default_device.utf8().get_data());
	} else {
		strcpy(dev, device.utf8().get_data());
	}
	print_verbose("PulseAudio: Detecting channels for device: " + String(dev));

	// Now using the device name get the amount of channels
	pa_status = 0;
	pa_operation *pa_op;
	if (capture) {
		pa_op = pa_context_get_source_info_by_name(pa_ctx, dev, &AudioDriverPulseAudio::pa_source_info_cb, (void *)this);
	} else {
		pa_op = pa_context_get_sink_info_by_name(pa_ctx, dev, &AudioDriverPulseAudio::pa_sink_info_cb, (void *)this);
	}

	if (pa_op) {
		while (pa_status == 0) {
			int ret = pa_mainloop_iterate(pa_ml, 1, NULL);
			if (ret < 0) {
				ERR_PRINT("pa_mainloop_iterate error");
			}
		}

		pa_operation_unref(pa_op);

		if (pa_status == -1) {
			return FAILED;
		}
	} else {
		if (capture) {
			ERR_PRINT("pa_context_get_source_info_by_name error");
		} else {
			ERR_PRINT("pa_context_get_sink_info_by_name error");
		}
	}

	return OK;
}

Error AudioDriverPulseAudio::init_device() {
	// If there is a specified device check that it is really present
	if (device_name != "Default") {
		Array list = get_device_list();
		if (list.find(device_name) == -1) {
			device_name = "Default";
			new_device = "Default";
		}
	}

	// Detect the amount of channels PulseAudio is using
	// Note: If using an even amount of channels (2, 4, etc) channels and pa_map.channels will be equal,
	// if not then pa_map.channels will have the real amount of channels PulseAudio is using and channels
	// will have the amount of channels Godot is using (in this case it's pa_map.channels + 1)
	Error err = detect_channels();
	if (err != OK) {
		// This most likely means there are no sinks.
		ERR_PRINT("PulseAudio: init device failed to detect number of channels");
		return err;
	}

	switch (pa_map.channels) {
		case 1: // Mono
		case 3: // Surround 2.1
		case 5: // Surround 5.0
		case 7: // Surround 7.0
			channels = pa_map.channels + 1;
			break;

		case 2: // Stereo
		case 4: // Surround 4.0
		case 6: // Surround 5.1
		case 8: // Surround 7.1
			channels = pa_map.channels;
			break;

		default:
			WARN_PRINTS("PulseAudio: Unsupported number of channels: " + itos(pa_map.channels));
			pa_channel_map_init_stereo(&pa_map);
			channels = 2;
			break;
	}

	int latency = GLOBAL_GET("audio/output_latency");
	buffer_frames = closest_power_of_2(latency * mix_rate / 1000);
	pa_buffer_size = buffer_frames * pa_map.channels;

	print_verbose("PulseAudio: detected " + itos(pa_map.channels) + " channels");
	print_verbose("PulseAudio: audio buffer frames: " + itos(buffer_frames) + " calculated latency: " + itos(buffer_frames * 1000 / mix_rate) + "ms");

	pa_sample_spec spec;
	spec.format = PA_SAMPLE_S16LE;
	spec.channels = pa_map.channels;
	spec.rate = mix_rate;
	pa_map.map[0] = PA_CHANNEL_POSITION_FRONT_LEFT;
	pa_map.map[1] = PA_CHANNEL_POSITION_FRONT_RIGHT;
	pa_map.map[2] = PA_CHANNEL_POSITION_FRONT_CENTER;
	pa_map.map[3] = PA_CHANNEL_POSITION_LFE;
	pa_map.map[4] = PA_CHANNEL_POSITION_REAR_LEFT;
	pa_map.map[5] = PA_CHANNEL_POSITION_REAR_RIGHT;
	pa_map.map[6] = PA_CHANNEL_POSITION_SIDE_LEFT;
	pa_map.map[7] = PA_CHANNEL_POSITION_SIDE_RIGHT;

	pa_str = pa_stream_new(pa_ctx, "Sound", &spec, &pa_map);
	if (pa_str == NULL) {
		ERR_PRINTS("PulseAudio: pa_stream_new error: " + String(pa_strerror(pa_context_errno(pa_ctx))));
		ERR_FAIL_V(ERR_CANT_OPEN);
	}

	pa_buffer_attr attr;
	// set to appropriate buffer length (in bytes) from global settings
	// Note: PulseAudio defaults to 4 fragments, which means that the actual
	// latency is tlength / fragments. It seems that the PulseAudio has no way
	// to get the fragments number so we're hardcoding this to the default of 4
	const int fragments = 4;
	attr.tlength = pa_buffer_size * sizeof(int16_t) * fragments;
	// set them to be automatically chosen
	attr.prebuf = (uint32_t)-1;
	attr.maxlength = (uint32_t)-1;
	attr.minreq = (uint32_t)-1;

	const char *dev = device_name == "Default" ? NULL : device_name.utf8().get_data();
	pa_stream_flags flags = pa_stream_flags(PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_ADJUST_LATENCY | PA_STREAM_AUTO_TIMING_UPDATE);
	int error_code = pa_stream_connect_playback(pa_str, dev, &attr, flags, NULL, NULL);
	ERR_FAIL_COND_V(error_code < 0, ERR_CANT_OPEN);

	samples_in.resize(buffer_frames * channels);
	samples_out.resize(pa_buffer_size);

	// Reset audio input to keep synchronisation.
	input_position = 0;
	input_size = 0;

	return OK;
}

Error AudioDriverPulseAudio::init() {
#ifdef DEBUG_ENABLED
	int dylibloader_verbose = 1;
#else
	int dylibloader_verbose = 0;
#endif
#ifdef ALSAMIDI_ENABLED
	// If using PulseAudio with ALSA MIDI, we need to initialize ALSA as well
	initialize_asound(dylibloader_verbose);
#endif
	if (initialize_pulse(dylibloader_verbose)) {
		return ERR_CANT_OPEN;
	}

	active = false;
	thread_exited = false;
	exit_thread = false;

	mix_rate = GLOBAL_GET("audio/mix_rate");

	pa_ml = pa_mainloop_new();
	ERR_FAIL_COND_V(pa_ml == NULL, ERR_CANT_OPEN);

	pa_ctx = pa_context_new(pa_mainloop_get_api(pa_ml), "Godot");
	ERR_FAIL_COND_V(pa_ctx == NULL, ERR_CANT_OPEN);

	pa_ready = 0;
	pa_context_set_state_callback(pa_ctx, pa_state_cb, (void *)this);

	int ret = pa_context_connect(pa_ctx, NULL, PA_CONTEXT_NOFLAGS, NULL);
	if (ret < 0) {
		if (pa_ctx) {
			pa_context_unref(pa_ctx);
			pa_ctx = NULL;
		}

		if (pa_ml) {
			pa_mainloop_free(pa_ml);
			pa_ml = NULL;
		}

		return ERR_CANT_OPEN;
	}

	while (pa_ready == 0) {
		ret = pa_mainloop_iterate(pa_ml, 1, NULL);
		if (ret < 0) {
			ERR_PRINT("pa_mainloop_iterate error");
		}
	}

	if (pa_ready < 0) {
		if (pa_ctx) {
			pa_context_disconnect(pa_ctx);
			pa_context_unref(pa_ctx);
			pa_ctx = NULL;
		}

		if (pa_ml) {
			pa_mainloop_free(pa_ml);
			pa_ml = NULL;
		}

		return ERR_CANT_OPEN;
	}

	init_device();
	thread.start(AudioDriverPulseAudio::thread_func, this);

	return OK;
}

float AudioDriverPulseAudio::get_latency() {
	if (latency == 0) { //only do this once since it's approximate anyway
		lock();

		pa_usec_t palat = 0;
		if (pa_stream_get_state(pa_str) == PA_STREAM_READY) {
			int negative = 0;

			if (pa_stream_get_latency(pa_str, &palat, &negative) >= 0) {
				if (negative) {
					palat = 0;
				}
			}
		}

		if (palat > 0) {
			latency = double(palat) / 1000000.0;
		}

		unlock();
	}

	return latency;
}

void AudioDriverPulseAudio::thread_func(void *p_udata) {
	AudioDriverPulseAudio *ad = (AudioDriverPulseAudio *)p_udata;
	unsigned int write_ofs = 0;
	size_t avail_bytes = 0;
	uint32_t default_device_msec = OS::get_singleton()->get_ticks_msec();

	while (!ad->exit_thread) {
		size_t read_bytes = 0;
		size_t written_bytes = 0;

		if (avail_bytes == 0) {
			ad->lock();
			ad->start_counting_ticks();

			if (!ad->active) {
				for (unsigned int i = 0; i < ad->pa_buffer_size; i++) {
					ad->samples_out.write[i] = 0;
				}
			} else {
				ad->audio_server_process(ad->buffer_frames, ad->samples_in.ptrw());

				if (ad->channels == ad->pa_map.channels) {
					for (unsigned int i = 0; i < ad->pa_buffer_size; i++) {
						ad->samples_out.write[i] = ad->samples_in[i] >> 16;
					}
				} else {
					// Uneven amount of channels
					unsigned int in_idx = 0;
					unsigned int out_idx = 0;

					for (unsigned int i = 0; i < ad->buffer_frames; i++) {
						for (int j = 0; j < ad->pa_map.channels - 1; j++) {
							ad->samples_out.write[out_idx++] = ad->samples_in[in_idx++] >> 16;
						}
						uint32_t l = ad->samples_in[in_idx++] >> 16;
						uint32_t r = ad->samples_in[in_idx++] >> 16;
						ad->samples_out.write[out_idx++] = (l + r) / 2;
					}
				}
			}

			avail_bytes = ad->pa_buffer_size * sizeof(int16_t);
			write_ofs = 0;
			ad->stop_counting_ticks();
			ad->unlock();
		}

		ad->lock();
		ad->start_counting_ticks();

		int ret;
		do {
			ret = pa_mainloop_iterate(ad->pa_ml, 0, NULL);
		} while (ret > 0);

		if (avail_bytes > 0 && pa_stream_get_state(ad->pa_str) == PA_STREAM_READY) {
			size_t bytes = pa_stream_writable_size(ad->pa_str);
			if (bytes > 0) {
				size_t bytes_to_write = MIN(bytes, avail_bytes);
				const void *ptr = ad->samples_out.ptr();
				ret = pa_stream_write(ad->pa_str, (char *)ptr + write_ofs, bytes_to_write, NULL, 0LL, PA_SEEK_RELATIVE);
				if (ret != 0) {
					ERR_PRINTS("PulseAudio: pa_stream_write error: " + String(pa_strerror(ret)));
				} else {
					avail_bytes -= bytes_to_write;
					write_ofs += bytes_to_write;
					written_bytes += bytes_to_write;
				}
			}
		}

		// User selected a new device, finish the current one so we'll init the new device
		if (ad->device_name != ad->new_device) {
			ad->device_name = ad->new_device;
			ad->finish_device();

			Error err = ad->init_device();
			if (err != OK) {
				ERR_PRINT("PulseAudio: init_device error");
				ad->device_name = "Default";
				ad->new_device = "Default";

				err = ad->init_device();
				if (err != OK) {
					ad->active = false;
					ad->exit_thread = true;
					break;
				}
			}

			avail_bytes = 0;
			write_ofs = 0;
		}

		// If we're using the default device check that the current device is still the default
		if (ad->device_name == "Default") {
			uint32_t msec = OS::get_singleton()->get_ticks_msec();
			if (msec > (default_device_msec + 1000)) {
				String old_default_device = ad->default_device;

				default_device_msec = msec;

				ad->pa_status = 0;
				pa_operation *pa_op = pa_context_get_server_info(ad->pa_ctx, &AudioDriverPulseAudio::pa_server_info_cb, (void *)ad);
				if (pa_op) {
					while (ad->pa_status == 0) {
						ret = pa_mainloop_iterate(ad->pa_ml, 1, NULL);
						if (ret < 0) {
							ERR_PRINT("pa_mainloop_iterate error");
						}
					}

					pa_operation_unref(pa_op);
				} else {
					ERR_PRINT("pa_context_get_server_info error: " + String(pa_strerror(pa_context_errno(ad->pa_ctx))));
				}

				if (old_default_device != ad->default_device) {
					ad->finish_device();

					Error err = ad->init_device();
					if (err != OK) {
						ERR_PRINT("PulseAudio: init_device error");
						ad->active = false;
						ad->exit_thread = true;
						break;
					}

					avail_bytes = 0;
					write_ofs = 0;
				}
			}
		}

		if (ad->pa_rec_str && pa_stream_get_state(ad->pa_rec_str) == PA_STREAM_READY) {
			size_t bytes = pa_stream_readable_size(ad->pa_rec_str);
			if (bytes > 0) {
				const void *ptr = NULL;
				size_t maxbytes = ad->input_buffer.size() * sizeof(int16_t);

				bytes = MIN(bytes, maxbytes);
				ret = pa_stream_peek(ad->pa_rec_str, &ptr, &bytes);
				if (ret != 0) {
					ERR_PRINT("pa_stream_peek error");
				} else {
					int16_t *srcptr = (int16_t *)ptr;
					for (size_t i = bytes >> 1; i > 0; i--) {
						int32_t sample = int32_t(*srcptr++) << 16;
						ad->input_buffer_write(sample);

						if (ad->pa_rec_map.channels == 1) {
							// In case input device is single channel convert it to Stereo
							ad->input_buffer_write(sample);
						}
					}

					read_bytes += bytes;
					ret = pa_stream_drop(ad->pa_rec_str);
					if (ret != 0) {
						ERR_PRINT("pa_stream_drop error");
					}
				}
			}

			// User selected a new device, finish the current one so we'll init the new device
			if (ad->capture_device_name != ad->capture_new_device) {
				ad->capture_device_name = ad->capture_new_device;
				ad->capture_finish_device();

				Error err = ad->capture_init_device();
				if (err != OK) {
					ERR_PRINT("PulseAudio: capture_init_device error");
					ad->capture_device_name = "Default";
					ad->capture_new_device = "Default";

					err = ad->capture_init_device();
					if (err != OK) {
						ad->active = false;
						ad->exit_thread = true;
						break;
					}
				}
			}
		}

		ad->stop_counting_ticks();
		ad->unlock();

		// Let the thread rest a while if we haven't read or write anything
		if (written_bytes == 0 && read_bytes == 0) {
			OS::get_singleton()->delay_usec(1000);
		}
	}

	ad->thread_exited = true;
}

void AudioDriverPulseAudio::start() {
	active = true;
}

int AudioDriverPulseAudio::get_mix_rate() const {
	return mix_rate;
}

AudioDriver::SpeakerMode AudioDriverPulseAudio::get_speaker_mode() const {
	return get_speaker_mode_by_total_channels(channels);
}

void AudioDriverPulseAudio::pa_sinklist_cb(pa_context *c, const pa_sink_info *l, int eol, void *userdata) {
	AudioDriverPulseAudio *ad = (AudioDriverPulseAudio *)userdata;

	// If eol is set to a positive number, you're at the end of the list
	if (eol > 0) {
		return;
	}

	ad->pa_devices.push_back(l->name);
	ad->pa_status++;
}

Array AudioDriverPulseAudio::get_device_list() {
	pa_devices.clear();
	pa_devices.push_back("Default");

	if (pa_ctx == NULL) {
		return pa_devices;
	}

	lock();

	// Get the device list
	pa_status = 0;
	pa_operation *pa_op = pa_context_get_sink_info_list(pa_ctx, pa_sinklist_cb, (void *)this);
	if (pa_op) {
		while (pa_status == 0) {
			int ret = pa_mainloop_iterate(pa_ml, 1, NULL);
			if (ret < 0) {
				ERR_PRINT("pa_mainloop_iterate error");
			}
		}

		pa_operation_unref(pa_op);
	} else {
		ERR_PRINT("pa_context_get_server_info error");
	}

	unlock();

	return pa_devices;
}

String AudioDriverPulseAudio::get_device() {
	return device_name;
}

void AudioDriverPulseAudio::set_device(String device) {
	lock();
	new_device = device;
	unlock();
}

void AudioDriverPulseAudio::lock() {
	mutex.lock();
}

void AudioDriverPulseAudio::unlock() {
	mutex.unlock();
}

void AudioDriverPulseAudio::finish_device() {
	if (pa_str) {
		pa_stream_disconnect(pa_str);
		pa_stream_unref(pa_str);
		pa_str = NULL;
	}
}

void AudioDriverPulseAudio::finish() {
	if (!thread.is_started())
		return;

	exit_thread = true;
	thread.wait_to_finish();

	finish_device();

	if (pa_ctx) {
		pa_context_disconnect(pa_ctx);
		pa_context_unref(pa_ctx);
		pa_ctx = NULL;
	}

	if (pa_ml) {
		pa_mainloop_free(pa_ml);
		pa_ml = NULL;
	}
}

Error AudioDriverPulseAudio::capture_init_device() {
	// If there is a specified device check that it is really present
	if (capture_device_name != "Default") {
		Array list = capture_get_device_list();
		if (list.find(capture_device_name) == -1) {
			capture_device_name = "Default";
			capture_new_device = "Default";
		}
	}

	detect_channels(true);
	switch (pa_rec_map.channels) {
		case 1: // Mono
		case 2: // Stereo
			break;

		default:
			WARN_PRINTS("PulseAudio: Unsupported number of input channels: " + itos(pa_rec_map.channels));
			pa_channel_map_init_stereo(&pa_rec_map);
			break;
	}

	pa_sample_spec spec;

	spec.format = PA_SAMPLE_S16LE;
	spec.channels = pa_rec_map.channels;
	spec.rate = mix_rate;

	int input_latency = 30;
	int input_buffer_frames = closest_power_of_2(input_latency * mix_rate / 1000);
	int input_buffer_size = input_buffer_frames * spec.channels;

	pa_buffer_attr attr;
	attr.fragsize = input_buffer_size * sizeof(int16_t);

	pa_rec_str = pa_stream_new(pa_ctx, "Record", &spec, &pa_rec_map);
	if (pa_rec_str == NULL) {
		ERR_PRINTS("PulseAudio: pa_stream_new error: " + String(pa_strerror(pa_context_errno(pa_ctx))));
		ERR_FAIL_V(ERR_CANT_OPEN);
	}

	const char *dev = capture_device_name == "Default" ? NULL : capture_device_name.utf8().get_data();
	pa_stream_flags flags = pa_stream_flags(PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_ADJUST_LATENCY | PA_STREAM_AUTO_TIMING_UPDATE);
	int error_code = pa_stream_connect_record(pa_rec_str, dev, &attr, flags);
	if (error_code < 0) {
		ERR_PRINTS("PulseAudio: pa_stream_connect_record error: " + String(pa_strerror(error_code)));
		ERR_FAIL_V(ERR_CANT_OPEN);
	}

	input_buffer_init(input_buffer_frames);

	print_verbose("PulseAudio: detected " + itos(pa_rec_map.channels) + " input channels");
	print_verbose("PulseAudio: input buffer frames: " + itos(input_buffer_frames) + " calculated latency: " + itos(input_buffer_frames * 1000 / mix_rate) + "ms");

	return OK;
}

void AudioDriverPulseAudio::capture_finish_device() {
	if (pa_rec_str) {
		int ret = pa_stream_disconnect(pa_rec_str);
		if (ret != 0) {
			ERR_PRINTS("PulseAudio: pa_stream_disconnect error: " + String(pa_strerror(ret)));
		}
		pa_stream_unref(pa_rec_str);
		pa_rec_str = NULL;
	}
}

Error AudioDriverPulseAudio::capture_start() {
	lock();
	Error err = capture_init_device();
	unlock();

	return err;
}

Error AudioDriverPulseAudio::capture_stop() {
	lock();
	capture_finish_device();
	unlock();

	return OK;
}

void AudioDriverPulseAudio::capture_set_device(const String &p_name) {
	lock();
	capture_new_device = p_name;
	unlock();
}

void AudioDriverPulseAudio::pa_sourcelist_cb(pa_context *c, const pa_source_info *l, int eol, void *userdata) {
	AudioDriverPulseAudio *ad = (AudioDriverPulseAudio *)userdata;

	// If eol is set to a positive number, you're at the end of the list
	if (eol > 0) {
		return;
	}

	if (l->monitor_of_sink == PA_INVALID_INDEX) {
		ad->pa_rec_devices.push_back(l->name);
	}

	ad->pa_status++;
}

Array AudioDriverPulseAudio::capture_get_device_list() {
	pa_rec_devices.clear();
	pa_rec_devices.push_back("Default");

	if (pa_ctx == NULL) {
		return pa_rec_devices;
	}

	lock();

	// Get the device list
	pa_status = 0;
	pa_operation *pa_op = pa_context_get_source_info_list(pa_ctx, pa_sourcelist_cb, (void *)this);
	if (pa_op) {
		while (pa_status == 0) {
			int ret = pa_mainloop_iterate(pa_ml, 1, NULL);
			if (ret < 0) {
				ERR_PRINT("pa_mainloop_iterate error");
			}
		}

		pa_operation_unref(pa_op);
	} else {
		ERR_PRINT("pa_context_get_server_info error");
	}

	unlock();

	return pa_rec_devices;
}

String AudioDriverPulseAudio::capture_get_device() {
	lock();
	String name = capture_device_name;
	unlock();

	return name;
}

AudioDriverPulseAudio::AudioDriverPulseAudio() :
		pa_ml(NULL),
		pa_ctx(NULL),
		pa_str(NULL),
		pa_rec_str(NULL),
		device_name("Default"),
		new_device("Default"),
		default_device(""),
		mix_rate(0),
		buffer_frames(0),
		pa_buffer_size(0),
		channels(0),
		pa_ready(0),
		pa_status(0),
		active(false),
		thread_exited(false),
		exit_thread(false),
		latency(0) {
	samples_in.clear();
	samples_out.clear();
}

AudioDriverPulseAudio::~AudioDriverPulseAudio() {
}

#endif
