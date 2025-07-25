/*************************************************************************/
/*  main.cpp                                                             */
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

#include "main.h"

#include "core/crypto/crypto.h"
#include "core/input_map.h"
#include "core/io/file_access_network.h"
#include "core/io/file_access_pack.h"
#include "core/io/file_access_zip.h"
#include "core/io/image_loader.h"
#include "core/io/ip.h"
#include "core/io/resource_loader.h"
#include "core/message_queue.h"
#include "core/os/dir_access.h"
#include "core/os/os.h"
#include "core/project_settings.h"
#include "core/register_core_types.h"
#include "core/script_debugger_local.h"
#include "core/script_language.h"
#include "core/translation.h"
#include "core/version.h"
#include "core/version_hash.gen.h"
#include "drivers/register_driver_types.h"
#include "main/app_icon.gen.h"
#include "main/input_default.h"
#include "main/main_timer_sync.h"
#include "main/performance.h"
#include "main/splash.gen.h"
#include "main/splash_editor.gen.h"
#include "modules/register_module_types.h"
#include "platform/register_platform_apis.h"
#include "scene/debugger/script_debugger_remote.h"
#include "scene/main/scene_tree.h"
#include "scene/main/viewport.h"
#include "scene/register_scene_types.h"
#include "scene/resources/packed_scene.h"
#include "servers/arvr_server.h"
#include "servers/audio_server.h"
#include "servers/camera_server.h"
#include "servers/physics_2d_server.h"
#include "servers/physics_server.h"
#include "servers/register_server_types.h"
#include "tests/runtime/test_main.h"

#ifdef TOOLS_ENABLED
#include "editor/editor.h"
#include "editor/editor_node.h"
#endif

/* Static members */

// Singletons

// Initialized in setup()
static Engine *engine = NULL;
static ProjectSettings *globals = NULL;
static InputMap *input_map = NULL;
static TranslationServer *translation_server = NULL;
static Performance *performance = NULL;
static PackedData *packed_data = NULL;
#ifdef MINIZIP_ENABLED
static ZipArchive *zip_packed_data = NULL;
#endif
static FileAccessNetworkClient *file_access_network_client = NULL;
static ScriptDebugger *script_debugger = NULL;
static MessageQueue *message_queue = NULL;

// Initialized in setup2()
static AudioServer *audio_server = NULL;
static CameraServer *camera_server = NULL;
static ARVRServer *arvr_server = NULL;
static PhysicsServer *physics_server = NULL;
static Physics2DServer *physics_2d_server = NULL;
// We error out if setup2() doesn't turn this true
static bool _start_success = false;

// Drivers

static int video_driver_idx = -1;
static int audio_driver_idx = -1;

// Engine config/tools

static bool editor = false;
static bool project_manager = false;
static String locale;
static bool show_help = false;
static bool auto_quit = false;
static OS::ProcessID allow_focus_steal_pid = 0;
#ifdef TOOLS_ENABLED
static bool auto_build_solutions = false;
#endif

// Display

static OS::VideoMode video_mode;
static int init_screen = -1;
static bool init_fullscreen = false;
static bool init_maximized = false;
static bool init_windowed = false;
static bool init_always_on_top = false;
static bool init_use_custom_pos = false;
static Vector2 init_custom_pos;
static bool force_lowdpi = false;

// Debug

static bool use_debug_profiler = false;
#ifdef DEBUG_ENABLED
static bool debug_collisions = false;
static bool debug_navigation = false;
#endif
static int frame_delay = 0;
static bool disable_render_loop = false;
static int fixed_fps = -1;
static bool print_fps = false;

/* Helper methods */

// FIXME: This is also not 100% accurate, `project_manager` is only true when it was requested,
// but not if e.g. we fail to load and project and fallback to the manager.
bool Main::is_project_manager() {
	return project_manager;
}

static String unescape_cmdline(const String &p_str) {
	return p_str.replace("%20", " ");
}

static String get_full_version_string() {
	String hash = String(VERSION_HASH);
	if (hash.length() != 0)
		hash = "." + hash.left(9);
	return String(VERSION_FULL_BUILD) + hash;
}

// FIXME: Could maybe be moved to PhysicsServerManager and Physics2DServerManager directly
// to have less code in main.cpp.
void initialize_physics() {
	// This must be defined BEFORE the 3d physics server is created
	GLOBAL_DEF("physics/3d/godot_physics/use_bvh", true);

	/// 3D Physics Server
	physics_server = PhysicsServerManager::new_server(ProjectSettings::get_singleton()->get(PhysicsServerManager::setting_property_name));
	if (!physics_server) {
		// Physics server not found, Use the default physics
		physics_server = PhysicsServerManager::new_default_server();
	}
	ERR_FAIL_COND(!physics_server);
	physics_server->init();

	/// 2D Physics server
	physics_2d_server = Physics2DServerManager::new_server(ProjectSettings::get_singleton()->get(Physics2DServerManager::setting_property_name));
	if (!physics_2d_server) {
		// Physics server not found, Use the default physics
		physics_2d_server = Physics2DServerManager::new_default_server();
	}
	ERR_FAIL_COND(!physics_2d_server);
	physics_2d_server->init();
}

void finalize_physics() {
	physics_server->finish();
	memdelete(physics_server);

	physics_2d_server->finish();
	memdelete(physics_2d_server);
}

//#define DEBUG_INIT
#ifdef DEBUG_INIT
#define MAIN_PRINT(m_txt) print_line(m_txt)
#else
#define MAIN_PRINT(m_txt)
#endif

void Main::print_help(const char *p_binary) {
	print_line(String(VERSION_NAME) + " v" + get_full_version_string() + " - " + String(VERSION_WEBSITE));
	OS::get_singleton()->print("Free and open source software under the terms of the MIT license.\n");
	OS::get_singleton()->print("(c) 2007-2014 Juan Linietsky, Ariel Manzur.\n");
	OS::get_singleton()->print("(c) 2014-present Godot Engine contributors.\n");
	OS::get_singleton()->print("\n");
	OS::get_singleton()->print("Usage: %s [options] [path to scene or 'project.godot' file]\n", p_binary);
	OS::get_singleton()->print("\n");

	OS::get_singleton()->print("General options:\n");
	OS::get_singleton()->print("  -h, --help                       Display this help message.\n");
	OS::get_singleton()->print("  --version                        Display the version string.\n");
	OS::get_singleton()->print("  -v, --verbose                    Use verbose stdout mode.\n");
	OS::get_singleton()->print("  --quiet                          Quiet mode, silences stdout messages. Errors are still displayed.\n");
	OS::get_singleton()->print("\n");

	OS::get_singleton()->print("Run options:\n");
#ifdef TOOLS_ENABLED
	OS::get_singleton()->print("  -e, --editor                     Start the editor instead of running the scene.\n");
	OS::get_singleton()->print("  -p, --project-manager            Start the project manager, even if a project is auto-detected.\n");
#endif
	OS::get_singleton()->print("  -q, --quit                       Quit after the first iteration.\n");
	OS::get_singleton()->print("  -l, --language <locale>          Use a specific locale (<locale> being a two-letter code).\n");
	OS::get_singleton()->print("  --path <directory>               Path to a project (<directory> must contain a 'project.godot' file).\n");
	OS::get_singleton()->print("  -u, --upwards                    Scan folders upwards for project.godot file.\n");
	OS::get_singleton()->print("  --main-pack <file>               Path to a pack (.pck) file to load.\n");
	OS::get_singleton()->print("  --render-thread <mode>           Render thread mode ('unsafe', 'safe', 'separate').\n");
	OS::get_singleton()->print("  --remote-fs <address>            Remote filesystem (<host/IP>[:<port>] address).\n");
	OS::get_singleton()->print("  --remote-fs-password <password>  Password for remote filesystem.\n");
	OS::get_singleton()->print("  --audio-driver <driver>          Audio driver (");
	for (int i = 0; i < OS::get_singleton()->get_audio_driver_count(); i++) {
		if (i != 0)
			OS::get_singleton()->print(", ");
		OS::get_singleton()->print("'%s'", OS::get_singleton()->get_audio_driver_name(i));
	}
	OS::get_singleton()->print(").\n");
	OS::get_singleton()->print("  --video-driver <driver>          Video driver (");
	for (int i = 0; i < OS::get_singleton()->get_video_driver_count(); i++) {
		if (i != 0)
			OS::get_singleton()->print(", ");
		OS::get_singleton()->print("'%s'", OS::get_singleton()->get_video_driver_name(i));
	}
	OS::get_singleton()->print(").\n");
	OS::get_singleton()->print("\n");

#ifndef SERVER_ENABLED
	OS::get_singleton()->print("Display options:\n");
	OS::get_singleton()->print("  -f, --fullscreen                 Request fullscreen mode.\n");
	OS::get_singleton()->print("  -m, --maximized                  Request a maximized window.\n");
	OS::get_singleton()->print("  -w, --windowed                   Request windowed mode.\n");
	OS::get_singleton()->print("  -t, --always-on-top              Request an always-on-top window.\n");
	OS::get_singleton()->print("  --resolution <W>x<H>             Request window resolution.\n");
	OS::get_singleton()->print("  --position <X>,<Y>               Request window position.\n");
	OS::get_singleton()->print("  --low-dpi                        Force low-DPI mode (macOS and Windows only).\n");
	OS::get_singleton()->print("  --no-window                      Run with invisible window. Useful together with --script.\n");
	OS::get_singleton()->print("  --enable-vsync-via-compositor    When vsync is enabled, vsync via the OS' window compositor (Windows only).\n");
	OS::get_singleton()->print("  --disable-vsync-via-compositor   Disable vsync via the OS' window compositor (Windows only).\n");
	OS::get_singleton()->print("  --tablet-driver                  Tablet input driver (");
	for (int i = 0; i < OS::get_singleton()->get_tablet_driver_count(); i++) {
		if (i != 0)
			OS::get_singleton()->print(", ");
		OS::get_singleton()->print("'%s'", OS::get_singleton()->get_tablet_driver_name(i).utf8().get_data());
	}
	OS::get_singleton()->print(") (Windows only).\n");
	OS::get_singleton()->print("\n");
#endif

	OS::get_singleton()->print("Debug options:\n");
	OS::get_singleton()->print("  -d, --debug                      Debug (local stdout debugger).\n");
	OS::get_singleton()->print("  -b, --breakpoints                Breakpoint list as source::line comma-separated pairs, no spaces (use %%20 instead).\n");
	OS::get_singleton()->print("  --profiling                      Enable profiling in the script debugger.\n");
	OS::get_singleton()->print("  --remote-debug <address>         Remote debug (<host/IP>:<port> address).\n");
#if defined(DEBUG_ENABLED) && !defined(SERVER_ENABLED)
	OS::get_singleton()->print("  --debug-collisions               Show collision shapes when running the scene.\n");
	OS::get_singleton()->print("  --debug-navigation               Show navigation polygons when running the scene.\n");
#endif
	OS::get_singleton()->print("  --frame-delay <ms>               Simulate high CPU load (delay each frame by <ms> milliseconds).\n");
	OS::get_singleton()->print("  --time-scale <scale>             Force time scale (higher values are faster, 1.0 is normal speed).\n");
	OS::get_singleton()->print("  --disable-render-loop            Disable render loop so rendering only occurs when called explicitly from script.\n");
	OS::get_singleton()->print("  --disable-crash-handler          Disable crash handler when supported by the platform code.\n");
	OS::get_singleton()->print("  --fixed-fps <fps>                Force a fixed number of frames per second. This setting disables real-time synchronization.\n");
	OS::get_singleton()->print("  --print-fps                      Print the frames per second to the stdout.\n");
	OS::get_singleton()->print("\n");

	OS::get_singleton()->print("Standalone tools:\n");
	OS::get_singleton()->print("  -s, --script <script>            Run a script.\n");
	OS::get_singleton()->print("  --check-only                     Only parse for errors and quit (use with --script).\n");
#ifdef TOOLS_ENABLED
	OS::get_singleton()->print("  --export <preset> <path>         Export the project using the given preset and matching release template. The preset name should match one defined in export_presets.cfg.\n");
	OS::get_singleton()->print("                                   <path> should be absolute or relative to the project directory, and include the filename for the binary (e.g. 'builds/game.exe'). The target directory should exist.\n");
	OS::get_singleton()->print("  --export-debug <preset> <path>   Same as --export, but using the debug template.\n");
	OS::get_singleton()->print("  --export-pack <preset> <path>    Same as --export, but only export the game pack for the given preset. The <path> extension determines whether it will be in PCK or ZIP format.\n");
	OS::get_singleton()->print("  --doctool <path>                 Dump the engine API reference to the given <path> in XML format, merging if existing files are found.\n");
	OS::get_singleton()->print("  --no-docbase                     Disallow dumping the base types (used with --doctool).\n");
	OS::get_singleton()->print("  --build-solutions                Build the scripting solutions. Implies --editor and requires a valid project to edit.\n");
#ifdef DEBUG_METHODS_ENABLED
	OS::get_singleton()->print("  --gdnative-generate-json-api     Generate JSON dump of the Godot API for GDNative bindings.\n");
#endif
	OS::get_singleton()->print("  --test <test>                    Run a test (");
	const char **test_names = tests_get_names();
	const char *comma = "";
	while (*test_names) {
		OS::get_singleton()->print("%s'%s'", comma, *test_names);
		test_names++;
		comma = ", ";
	}
	OS::get_singleton()->print(").\n");
#endif
}

/* Engine initialization
 *
 * Consists of several methods that are called by each platform's specific main(argc, argv).
 * To fully understand engine init, one should therefore start from the platform's main and
 * see how it calls into the Main class' methods.
 *
 * The initialization is typically done in 3 steps (with the setup2 step triggered either
 * automatically by setup, or manually in the platform's main).
 *
 * - setup(execpath, argc, argv, p_second_phase) is the main entry point for all platforms,
 *   responsible for the initialization of all low level singletons and core types, and parsing
 *   command line arguments to configure things accordingly.
 *   If p_second_phase is true, it will chain into setup2() (default behaviour). This is
 *   disabled on some platforms (Android, iOS, UWP) which trigger the second step in their
 *   own time.
 *
 * - setup2(p_main_tid_override) registers high level servers and singletons, displays the
 *   boot splash, then registers higher level types (scene, editor, etc.).
 *
 * - start() is the last step and that's where command line tools can run, or the main loop
 *   can be created eventually and the project settings put into action. That's also where
 *   the editor node is created, if relevant.
 *   start() does it own argument parsing for a subset of the command line arguments described
 *   in help, it's a bit messy and should be globalized with the setup() parsing somehow.
 */

Error Main::setup(const char *execpath, int argc, char *argv[], bool p_second_phase) {
#if defined(DEBUG_ENABLED) && !defined(NO_THREADS)
	check_lockless_atomics();
#endif

	RID_OwnerBase::init_rid();

	OS::get_singleton()->initialize_core();

	engine = memnew(Engine);

	MAIN_PRINT("Main: Initialize CORE");

	register_core_types();
	register_core_driver_types();

	MAIN_PRINT("Main: Initialize Globals");

	globals = memnew(ProjectSettings);
	input_map = memnew(InputMap);

	register_core_settings(); //here globals is present

	translation_server = memnew(TranslationServer);
	performance = memnew(Performance);
	ClassDB::register_class<Performance>();
	engine->add_singleton(Engine::Singleton("Performance", performance));

	GLOBAL_DEF("debug/settings/crash_handler/message", String("Please include this when reporting the bug on https://github.com/godotengine/godot/issues"));

	MAIN_PRINT("Main: Parse CMDLine");

	/* argument parsing and main creation */
	List<String> args;
	List<String> main_args;

	for (int i = 0; i < argc; i++) {
		args.push_back(String::utf8(argv[i]));
	}

	List<String>::Element *I = args.front();

	I = args.front();

	while (I) {
		I->get() = unescape_cmdline(I->get().strip_edges());
		I = I->next();
	}

	I = args.front();

	String video_driver = "";
	String audio_driver = "";
	String tablet_driver = "";
	String project_path = ".";
	bool upwards = false;
	String debug_mode;
	String debug_host;
	bool skip_breakpoints = false;
	String main_pack;
	bool quiet_stdout = false;
	int rtm = -1;

	String remotefs;
	String remotefs_pass;

	Vector<String> breakpoints;
	bool use_custom_res = true;
	bool force_res = false;
	bool saw_vsync_via_compositor_override = false;
#ifdef TOOLS_ENABLED
	bool found_project = false;
#endif

	packed_data = PackedData::get_singleton();
	if (!packed_data)
		packed_data = memnew(PackedData);

#ifdef MINIZIP_ENABLED

	//XXX: always get_singleton() == 0x0
	zip_packed_data = ZipArchive::get_singleton();
	//TODO: remove this temporary fix
	if (!zip_packed_data) {
		zip_packed_data = memnew(ZipArchive);
	}

	packed_data->add_pack_source(zip_packed_data);
#endif

	I = args.front();
	while (I) {
#ifdef OSX_ENABLED
		// Ignore the process serial number argument passed by macOS Gatekeeper.
		// Otherwise, Godot would try to open a non-existent project on the first start and abort.
		if (I->get().begins_with("-psn_")) {
			I = I->next();
			continue;
		}
#endif

		List<String>::Element *N = I->next();

		if (I->get() == "-h" || I->get() == "--help" || I->get() == "/?") { // display help

			show_help = true;
			goto error;

		} else if (I->get() == "--version") {
			print_line(get_full_version_string());
			goto error;

		} else if (I->get() == "-v" || I->get() == "--verbose") { // verbose output

			OS::get_singleton()->_verbose_stdout = true;
		} else if (I->get() == "--quiet") { // quieter output

			quiet_stdout = true;

		} else if (I->get() == "--audio-driver") { // audio driver

			if (I->next()) {
				audio_driver = I->next()->get();

				bool found = false;
				for (int i = 0; i < OS::get_singleton()->get_audio_driver_count(); i++) {
					if (audio_driver == OS::get_singleton()->get_audio_driver_name(i)) {
						found = true;
					}
				}

				if (!found) {
					OS::get_singleton()->print("Unknown audio driver '%s', aborting.\nValid options are ", audio_driver.utf8().get_data());

					for (int i = 0; i < OS::get_singleton()->get_audio_driver_count(); i++) {
						if (i == OS::get_singleton()->get_audio_driver_count() - 1) {
							OS::get_singleton()->print(" and ");
						} else if (i != 0) {
							OS::get_singleton()->print(", ");
						}

						OS::get_singleton()->print("'%s'", OS::get_singleton()->get_audio_driver_name(i));
					}

					OS::get_singleton()->print(".\n");

					goto error;
				}

				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing audio driver argument, aborting.\n");
				goto error;
			}

		} else if (I->get() == "--video-driver") { // force video driver

			if (I->next()) {
				video_driver = I->next()->get();

				bool found = false;
				for (int i = 0; i < OS::get_singleton()->get_video_driver_count(); i++) {
					if (video_driver == OS::get_singleton()->get_video_driver_name(i)) {
						found = true;
					}
				}

				if (!found) {
					OS::get_singleton()->print("Unknown video driver '%s', aborting.\nValid options are ", video_driver.utf8().get_data());

					for (int i = 0; i < OS::get_singleton()->get_video_driver_count(); i++) {
						if (i == OS::get_singleton()->get_video_driver_count() - 1) {
							OS::get_singleton()->print(" and ");
						} else if (i != 0) {
							OS::get_singleton()->print(", ");
						}

						OS::get_singleton()->print("'%s'", OS::get_singleton()->get_video_driver_name(i));
					}

					OS::get_singleton()->print(".\n");

					goto error;
				}

				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing video driver argument, aborting.\n");
				goto error;
			}
#ifndef SERVER_ENABLED
		} else if (I->get() == "-f" || I->get() == "--fullscreen") { // force fullscreen

			init_fullscreen = true;
		} else if (I->get() == "-m" || I->get() == "--maximized") { // force maximized window

			init_maximized = true;
			video_mode.maximized = true;

		} else if (I->get() == "-w" || I->get() == "--windowed") { // force windowed window

			init_windowed = true;
		} else if (I->get() == "-t" || I->get() == "--always-on-top") { // force always-on-top window

			init_always_on_top = true;
		} else if (I->get() == "--resolution") { // force resolution

			if (I->next()) {
				String vm = I->next()->get();

				if (vm.find("x") == -1) { // invalid parameter format

					OS::get_singleton()->print("Invalid resolution '%s', it should be e.g. '1280x720'.\n", vm.utf8().get_data());
					goto error;
				}

				int w = vm.get_slice("x", 0).to_int();
				int h = vm.get_slice("x", 1).to_int();

				if (w <= 0 || h <= 0) {
					OS::get_singleton()->print("Invalid resolution '%s', width and height must be above 0.\n", vm.utf8().get_data());
					goto error;
				}

				video_mode.width = w;
				video_mode.height = h;
				force_res = true;

				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing resolution argument, aborting.\n");
				goto error;
			}

		} else if (I->get() == "--position") { // set window position

			if (I->next()) {
				String vm = I->next()->get();

				if (vm.find(",") == -1) { // invalid parameter format

					OS::get_singleton()->print("Invalid position '%s', it should be e.g. '80,128'.\n", vm.utf8().get_data());
					goto error;
				}

				int x = vm.get_slice(",", 0).to_int();
				int y = vm.get_slice(",", 1).to_int();

				init_custom_pos = Point2(x, y);
				init_use_custom_pos = true;

				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing position argument, aborting.\n");
				goto error;
			}

		} else if (I->get() == "--low-dpi") { // force low DPI

			force_lowdpi = true;
		} else if (I->get() == "--no-window") { // run with an invisible window

			OS::get_singleton()->set_no_window_mode(true);
		} else if (I->get() == "--tablet-driver") {
			if (I->next()) {
				tablet_driver = I->next()->get();
				bool found = false;
				for (int i = 0; i < OS::get_singleton()->get_tablet_driver_count(); i++) {
					if (tablet_driver == OS::get_singleton()->get_tablet_driver_name(i)) {
						found = true;
					}
				}

				if (!found) {
					OS::get_singleton()->print("Unknown tablet driver '%s', aborting.\n", tablet_driver.utf8().get_data());
					goto error;
				}

				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing tablet driver argument, aborting.\n");
				goto error;
			}
		} else if (I->get() == "--enable-vsync-via-compositor") {
			video_mode.vsync_via_compositor = true;
			saw_vsync_via_compositor_override = true;
		} else if (I->get() == "--disable-vsync-via-compositor") {
			video_mode.vsync_via_compositor = false;
			saw_vsync_via_compositor_override = true;
#endif
		} else if (I->get() == "--profiling") { // enable profiling

			use_debug_profiler = true;

		} else if (I->get() == "-l" || I->get() == "--language") { // language

			if (I->next()) {
				locale = I->next()->get();
				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing language argument, aborting.\n");
				goto error;
			}

		} else if (I->get() == "--remote-fs") { // remote filesystem

			if (I->next()) {
				remotefs = I->next()->get();
				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing remote filesystem address, aborting.\n");
				goto error;
			}
		} else if (I->get() == "--remote-fs-password") { // remote filesystem password

			if (I->next()) {
				remotefs_pass = I->next()->get();
				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing remote filesystem password, aborting.\n");
				goto error;
			}
		} else if (I->get() == "--render-thread") { // render thread mode

			if (I->next()) {
				if (I->next()->get() == "safe")
					rtm = OS::RENDER_THREAD_SAFE;
				else if (I->next()->get() == "unsafe")
					rtm = OS::RENDER_THREAD_UNSAFE;
				else if (I->next()->get() == "separate")
					rtm = OS::RENDER_SEPARATE_THREAD;

				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing render thread mode argument, aborting.\n");
				goto error;
			}
#ifdef TOOLS_ENABLED
		} else if (I->get() == "-e" || I->get() == "--editor") { // starts editor

			editor = true;
		} else if (I->get() == "-p" || I->get() == "--project-manager") { // starts project manager

			project_manager = true;
		} else if (I->get() == "--build-solutions") { // Build the scripting solution

			auto_build_solutions = true;
			editor = true;
#ifdef DEBUG_METHODS_ENABLED
		} else if (I->get() == "--gdnative-generate-json-api") {
			// Register as an editor instance to use the GLES2 fallback automatically on hardware that doesn't support the GLES3 backend
			editor = true;

			// We still pass it to the main arguments since the argument handling itself is not done in this function
			main_args.push_back(I->get());
#endif
		} else if (I->get() == "--export" || I->get() == "--export-debug" || I->get() == "--export-pack") { // Export project

			editor = true;
			main_args.push_back(I->get());
#endif
		} else if (I->get() == "--path") { // set path of project to start or edit

			if (I->next()) {
				String p = I->next()->get();
				if (OS::get_singleton()->set_cwd(p) == OK) {
					//nothing
				} else {
					project_path = I->next()->get(); //use project_path instead
				}
				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing relative or absolute path, aborting.\n");
				goto error;
			}
		} else if (I->get() == "-u" || I->get() == "--upwards") { // scan folders upwards
			upwards = true;
		} else if (I->get() == "-q" || I->get() == "--quit") { // Auto quit at the end of the first main loop iteration
			auto_quit = true;
		} else if (I->get().ends_with("project.godot")) {
			String path;
			String file = I->get();
			int sep = MAX(file.find_last("/"), file.find_last("\\"));
			if (sep == -1)
				path = ".";
			else {
				path = file.substr(0, sep);
			}
			if (OS::get_singleton()->set_cwd(path) == OK) {
				// path already specified, don't override
			} else {
				project_path = path;
			}
#ifdef TOOLS_ENABLED
			editor = true;
#endif
		} else if (I->get() == "-b" || I->get() == "--breakpoints") { // add breakpoints

			if (I->next()) {
				String bplist = I->next()->get();
				breakpoints = bplist.split(",");
				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing list of breakpoints, aborting.\n");
				goto error;
			}

		} else if (I->get() == "--frame-delay") { // force frame delay

			if (I->next()) {
				frame_delay = I->next()->get().to_int();
				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing frame delay argument, aborting.\n");
				goto error;
			}

		} else if (I->get() == "--time-scale") { // force time scale

			if (I->next()) {
				Engine::get_singleton()->set_time_scale(I->next()->get().to_double());
				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing time scale argument, aborting.\n");
				goto error;
			}

		} else if (I->get() == "--main-pack") {
			if (I->next()) {
				main_pack = I->next()->get();
				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing path to main pack file, aborting.\n");
				goto error;
			};

		} else if (I->get() == "-d" || I->get() == "--debug") {
			debug_mode = "local";
			OS::get_singleton()->_debug_stdout = true;
#if defined(DEBUG_ENABLED) && !defined(SERVER_ENABLED)
		} else if (I->get() == "--debug-collisions") {
			debug_collisions = true;
		} else if (I->get() == "--debug-navigation") {
			debug_navigation = true;
#endif
		} else if (I->get() == "--remote-debug") {
			if (I->next()) {
				debug_mode = "remote";
				debug_host = I->next()->get();
				if (debug_host.find(":") == -1) { // wrong address
					OS::get_singleton()->print("Invalid debug host address, it should be of the form <host/IP>:<port>.\n");
					goto error;
				}
				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing remote debug host address, aborting.\n");
				goto error;
			}
		} else if (I->get() == "--allow_focus_steal_pid") { // not exposed to user
			if (I->next()) {
				allow_focus_steal_pid = I->next()->get().to_int64();
				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing editor PID argument, aborting.\n");
				goto error;
			}
		} else if (I->get() == "--disable-render-loop") {
			disable_render_loop = true;
		} else if (I->get() == "--fixed-fps") {
			if (I->next()) {
				fixed_fps = I->next()->get().to_int();
				N = I->next()->next();
			} else {
				OS::get_singleton()->print("Missing fixed-fps argument, aborting.\n");
				goto error;
			}
		} else if (I->get() == "--print-fps") {
			print_fps = true;
		} else if (I->get() == "--disable-crash-handler") {
			OS::get_singleton()->disable_crash_handler();
		} else if (I->get() == "--skip-breakpoints") {
			skip_breakpoints = true;
		} else {
			main_args.push_back(I->get());
		}

		I = N;
	}

#ifdef TOOLS_ENABLED
	if (editor && project_manager) {
		OS::get_singleton()->print("Error: Command line arguments implied opening both editor and project manager, which is not possible. Aborting.\n");
		goto error;
	}
#endif

	// Network file system needs to be configured before globals, since globals are based on the
	// 'project.godot' file which will only be available through the network if this is enabled
	FileAccessNetwork::configure();
	if (remotefs != "") {
		file_access_network_client = memnew(FileAccessNetworkClient);
		int port;
		if (remotefs.find(":") != -1) {
			port = remotefs.get_slicec(':', 1).to_int();
			remotefs = remotefs.get_slicec(':', 0);
		} else {
			port = 6010;
		}

		Error err = file_access_network_client->connect(remotefs, port, remotefs_pass);
		if (err) {
			OS::get_singleton()->printerr("Could not connect to remotefs: %s:%i.\n", remotefs.utf8().get_data(), port);
			goto error;
		}

		FileAccess::make_default<FileAccessNetwork>(FileAccess::ACCESS_RESOURCES);
	}

	if (globals->setup(project_path, main_pack, upwards) == OK) {
#ifdef TOOLS_ENABLED
		found_project = true;
#endif
	} else {
#ifdef TOOLS_ENABLED
		editor = false;
#else
		const String error_msg = "Error: Couldn't load project data at path \"" + project_path + "\". Is the .pck file missing?\nIf you've renamed the executable, the associated .pck file should also be renamed to match the executable's name (without the extension).\n";
		OS::get_singleton()->print("%s", error_msg.ascii().get_data());
		OS::get_singleton()->alert(error_msg);

		goto error;
#endif
	}

	// Initialize user data dir.
	OS::get_singleton()->ensure_user_data_dir();

	GLOBAL_DEF("memory/limits/multithreaded_server/rid_pool_prealloc", 60);
	ProjectSettings::get_singleton()->set_custom_property_info("memory/limits/multithreaded_server/rid_pool_prealloc", PropertyInfo(Variant::INT, "memory/limits/multithreaded_server/rid_pool_prealloc", PROPERTY_HINT_RANGE, "0,500,1")); // No negative and limit to 500 due to crashes
	GLOBAL_DEF("network/limits/debugger_stdout/max_chars_per_second", 2048);
	ProjectSettings::get_singleton()->set_custom_property_info("network/limits/debugger_stdout/max_chars_per_second", PropertyInfo(Variant::INT, "network/limits/debugger_stdout/max_chars_per_second", PROPERTY_HINT_RANGE, "0, 4096, 1, or_greater"));
	GLOBAL_DEF("network/limits/debugger_stdout/max_messages_per_frame", 10);
	ProjectSettings::get_singleton()->set_custom_property_info("network/limits/debugger_stdout/max_messages_per_frame", PropertyInfo(Variant::INT, "network/limits/debugger_stdout/max_messages_per_frame", PROPERTY_HINT_RANGE, "0, 20, 1, or_greater"));
	GLOBAL_DEF("network/limits/debugger_stdout/max_errors_per_second", 100);
	ProjectSettings::get_singleton()->set_custom_property_info("network/limits/debugger_stdout/max_errors_per_second", PropertyInfo(Variant::INT, "network/limits/debugger_stdout/max_errors_per_second", PROPERTY_HINT_RANGE, "0, 200, 1, or_greater"));
	GLOBAL_DEF("network/limits/debugger_stdout/max_warnings_per_second", 100);
	ProjectSettings::get_singleton()->set_custom_property_info("network/limits/debugger_stdout/max_warnings_per_second", PropertyInfo(Variant::INT, "network/limits/debugger_stdout/max_warnings_per_second", PROPERTY_HINT_RANGE, "0, 200, 1, or_greater"));

	if (debug_mode == "remote") {
		ScriptDebuggerRemote *sdr = memnew(ScriptDebuggerRemote);
		uint16_t debug_port = 6007;
		if (debug_host.find(":") != -1) {
			int sep_pos = debug_host.find_last(":");
			debug_port = debug_host.substr(sep_pos + 1, debug_host.length()).to_int();
			debug_host = debug_host.substr(0, sep_pos);
		}
		Error derr = sdr->connect_to_host(debug_host, debug_port);

		sdr->set_skip_breakpoints(skip_breakpoints);

		if (derr != OK) {
			memdelete(sdr);
		} else {
			script_debugger = sdr;
			sdr->set_allow_focus_steal_pid(allow_focus_steal_pid);
		}
	} else if (debug_mode == "local") {
		script_debugger = memnew(ScriptDebuggerLocal);
		OS::get_singleton()->initialize_debugging();
	}
	if (script_debugger) {
		//there is a debugger, parse breakpoints

		for (int i = 0; i < breakpoints.size(); i++) {
			String bp = breakpoints[i];
			int sp = bp.find_last(":");
			ERR_CONTINUE_MSG(sp == -1, "Invalid breakpoint: '" + bp + "', expected file:line format.");

			script_debugger->insert_breakpoint(bp.substr(sp + 1, bp.length()).to_int(), bp.substr(0, sp));
		}
	}

#ifdef TOOLS_ENABLED
	if (editor) {
		packed_data->set_disabled(true);
		globals->set_disable_feature_overrides(true);
	}

#endif

#ifdef TOOLS_ENABLED
	if (editor) {
		if (!init_windowed) {
			init_maximized = true;
			video_mode.maximized = true;
		}
	}

	if (!project_manager && !editor) {
		// Determine if the project manager should be requested
		project_manager = main_args.size() == 0 && !found_project;
	}
#endif

	// Only flush stdout in debug builds by default, as spamming `print()` will
	// decrease performance if this is enabled.
	GLOBAL_DEF_RST("application/run/flush_stdout_on_print", false);
	GLOBAL_DEF_RST("application/run/flush_stdout_on_print.debug", true);

	GLOBAL_DEF("logging/file_logging/enable_file_logging", false);
	// Only file logging by default on desktop platforms as logs can't be
	// accessed easily on mobile/Web platforms (if at all).
	// This also prevents logs from being created for the editor instance, as feature tags
	// are disabled while in the editor (even if they should logically apply).
	GLOBAL_DEF("logging/file_logging/enable_file_logging.pc", true);
	GLOBAL_DEF("logging/file_logging/log_path", "user://logs/godot.log");
	GLOBAL_DEF("logging/file_logging/max_log_files", 5);
	ProjectSettings::get_singleton()->set_custom_property_info("logging/file_logging/max_log_files", PropertyInfo(Variant::INT, "logging/file_logging/max_log_files", PROPERTY_HINT_RANGE, "0,20,1,or_greater")); //no negative numbers
	if (!project_manager && !editor && FileAccess::get_create_func(FileAccess::ACCESS_USERDATA) && GLOBAL_GET("logging/file_logging/enable_file_logging")) {
		// Don't create logs for the project manager as they would be written to
		// the current working directory, which is inconvenient.
		String base_path = GLOBAL_GET("logging/file_logging/log_path");
		int max_files = GLOBAL_GET("logging/file_logging/max_log_files");
		OS::get_singleton()->add_logger(memnew(RotatedFileLogger(base_path, max_files)));
	}

	if (main_args.size() == 0 && String(GLOBAL_DEF("application/run/main_scene", "")) == "") {
#ifdef TOOLS_ENABLED
		if (!editor && !project_manager) {
#endif
			OS::get_singleton()->print("Error: Can't run project: no main scene defined.\n");
			goto error;
#ifdef TOOLS_ENABLED
		}
#endif
	}

	if (editor || project_manager) {
		Engine::get_singleton()->set_editor_hint(true);
		use_custom_res = false;
		input_map->load_default(); //keys for editor
	} else {
		input_map->load_from_globals(); //keys for game
	}

	if (bool(ProjectSettings::get_singleton()->get("application/run/disable_stdout"))) {
		quiet_stdout = true;
	}
	if (bool(ProjectSettings::get_singleton()->get("application/run/disable_stderr"))) {
		_print_error_enabled = false;
	};

	if (quiet_stdout)
		_print_line_enabled = false;

	Logger::set_flush_stdout_on_print(ProjectSettings::get_singleton()->get("application/run/flush_stdout_on_print"));

	OS::get_singleton()->set_cmdline(execpath, main_args);

	GLOBAL_DEF("rendering/quality/driver/driver_name", "GLES3");
	ProjectSettings::get_singleton()->set_custom_property_info("rendering/quality/driver/driver_name", PropertyInfo(Variant::STRING, "rendering/quality/driver/driver_name", PROPERTY_HINT_ENUM, "GLES2,GLES3"));
	if (video_driver == "") {
		video_driver = GLOBAL_GET("rendering/quality/driver/driver_name");
	}

	GLOBAL_DEF("rendering/quality/driver/fallback_to_gles2", false);

	// Assigning here, to be sure that it appears in docs
	GLOBAL_DEF("rendering/2d/options/use_nvidia_rect_flicker_workaround", false);

	GLOBAL_DEF("display/window/size/width", 1024);
	ProjectSettings::get_singleton()->set_custom_property_info("display/window/size/width", PropertyInfo(Variant::INT, "display/window/size/width", PROPERTY_HINT_RANGE, "0,7680,or_greater")); // 8K resolution
	GLOBAL_DEF("display/window/size/height", 600);
	ProjectSettings::get_singleton()->set_custom_property_info("display/window/size/height", PropertyInfo(Variant::INT, "display/window/size/height", PROPERTY_HINT_RANGE, "0,4320,or_greater")); // 8K resolution
	GLOBAL_DEF("display/window/size/resizable", true);
	GLOBAL_DEF("display/window/size/fullscreen", false);
	GLOBAL_DEF("display/window/size/always_on_top", false);
	GLOBAL_DEF("display/window/size/keep_aspect_ratio", false);
	GLOBAL_DEF("display/window/size/test_width", 0);
	ProjectSettings::get_singleton()->set_custom_property_info("display/window/size/test_width", PropertyInfo(Variant::INT, "display/window/size/test_width", PROPERTY_HINT_RANGE, "0,7680,or_greater")); // 8K resolution
	GLOBAL_DEF("display/window/size/test_height", 0);
	ProjectSettings::get_singleton()->set_custom_property_info("display/window/size/test_height", PropertyInfo(Variant::INT, "display/window/size/test_height", PROPERTY_HINT_RANGE, "0,4320,or_greater")); // 8K resolution

	GLOBAL_DEF("display/window/borderless/enabled", false);
	GLOBAL_DEF("display/window/borderless/resizable", false);
	GLOBAL_DEF("display/window/borderless/shadow", false);
	GLOBAL_DEF("display/window/custom_title_bar/enabled", false);
	GLOBAL_DEF("display/window/custom_title_bar/height", 32);

	if (use_custom_res) {
		if (!force_res) {
			video_mode.width = GLOBAL_GET("display/window/size/width");
			video_mode.height = GLOBAL_GET("display/window/size/height");

			if (globals->has_setting("display/window/size/test_width") && globals->has_setting("display/window/size/test_height")) {
				int tw = globals->get("display/window/size/test_width");
				if (tw > 0) {
					video_mode.width = tw;
				}
				int th = globals->get("display/window/size/test_height");
				if (th > 0) {
					video_mode.height = th;
				}
			}
		}

		video_mode.resizable = GLOBAL_GET("display/window/size/resizable");
		video_mode.fullscreen = GLOBAL_GET("display/window/size/fullscreen");
		video_mode.always_on_top = GLOBAL_GET("display/window/size/always_on_top");
		video_mode.keep_aspect_ratio = GLOBAL_GET("display/window/size/keep_aspect_ratio");
		video_mode.borderless = GLOBAL_GET("display/window/borderless/enabled");
		video_mode.borderless_resizable = GLOBAL_GET("display/window/borderless/resizable");
		video_mode.borderless_shadow = GLOBAL_GET("display/window/borderless/shadow");
		video_mode.custom_title_bar_enabled = GLOBAL_GET("display/window/custom_title_bar/enabled");
		video_mode.custom_title_bar_height = GLOBAL_GET("display/window/custom_title_bar/height");
	}

	if (!force_lowdpi) {
		OS::get_singleton()->_allow_hidpi = GLOBAL_DEF("display/window/dpi/allow_hidpi", true);
	}

	video_mode.use_vsync = GLOBAL_DEF_RST("display/window/vsync/use_vsync", true);
	OS::get_singleton()->_use_vsync = video_mode.use_vsync;

	if (!saw_vsync_via_compositor_override) {
		// If one of the command line options to enable/disable vsync via the
		// window compositor ("--enable-vsync-via-compositor" or
		// "--disable-vsync-via-compositor") was present then it overrides the
		// project setting.
		video_mode.vsync_via_compositor = GLOBAL_DEF("display/window/vsync/vsync_via_compositor", false);
	}

	OS::get_singleton()->_vsync_via_compositor = video_mode.vsync_via_compositor;

	if (tablet_driver == "") { // specified in project.godot
		tablet_driver = GLOBAL_DEF_RST("display/window/tablet_driver", OS::get_singleton()->get_tablet_driver_name(0));
	}

	for (int i = 0; i < OS::get_singleton()->get_tablet_driver_count(); i++) {
		if (tablet_driver == OS::get_singleton()->get_tablet_driver_name(i)) {
			OS::get_singleton()->set_current_tablet_driver(OS::get_singleton()->get_tablet_driver_name(i));
			break;
		}
	}

	if (tablet_driver == "") {
		OS::get_singleton()->set_current_tablet_driver(OS::get_singleton()->get_tablet_driver_name(0));
	}

	OS::get_singleton()->_allow_layered = GLOBAL_DEF("display/window/per_pixel_transparency/allowed", false);
	video_mode.layered = GLOBAL_DEF("display/window/per_pixel_transparency/enabled", false);

	GLOBAL_DEF("rendering/quality/intended_usage/framebuffer_allocation", 2);
	GLOBAL_DEF("rendering/quality/intended_usage/framebuffer_allocation.mobile", 3);

	if (editor || project_manager) {
		OS::get_singleton()->_allow_layered = false;
	}

	Engine::get_singleton()->_gpu_pixel_snap = GLOBAL_DEF_ALIAS("rendering/2d/snapping/use_gpu_pixel_snap", "rendering/quality/2d/use_pixel_snap", false);

	OS::get_singleton()->_keep_screen_on = GLOBAL_DEF("display/window/energy_saving/keep_screen_on", true);
	if (rtm == -1) {
		rtm = GLOBAL_DEF("rendering/threads/thread_model", OS::RENDER_THREAD_SAFE);
	}

	if (rtm >= 0 && rtm < 3) {
#ifdef NO_THREADS
		rtm = OS::RENDER_THREAD_UNSAFE; // No threads available on this platform.
#else
		if (editor) {
			rtm = OS::RENDER_THREAD_SAFE;
		}
#endif
		OS::get_singleton()->_render_thread_mode = OS::RenderThreadMode(rtm);
	}

	/* Determine audio and video drivers */

	for (int i = 0; i < OS::get_singleton()->get_video_driver_count(); i++) {
		if (video_driver == OS::get_singleton()->get_video_driver_name(i)) {
			video_driver_idx = i;
			break;
		}
	}

	if (video_driver_idx < 0) {
		video_driver_idx = 0;
	}

	if (audio_driver == "") { // specified in project.godot
		audio_driver = GLOBAL_DEF_RST("audio/driver", OS::get_singleton()->get_audio_driver_name(0));
	}

	for (int i = 0; i < OS::get_singleton()->get_audio_driver_count(); i++) {
		if (audio_driver == OS::get_singleton()->get_audio_driver_name(i)) {
			audio_driver_idx = i;
			break;
		}
	}

	if (audio_driver_idx < 0) {
		audio_driver_idx = 0;
	}

	{
		String orientation = GLOBAL_DEF("display/window/handheld/orientation", "landscape");

		if (orientation == "portrait")
			OS::get_singleton()->set_screen_orientation(OS::SCREEN_PORTRAIT);
		else if (orientation == "reverse_landscape")
			OS::get_singleton()->set_screen_orientation(OS::SCREEN_REVERSE_LANDSCAPE);
		else if (orientation == "reverse_portrait")
			OS::get_singleton()->set_screen_orientation(OS::SCREEN_REVERSE_PORTRAIT);
		else if (orientation == "sensor_landscape")
			OS::get_singleton()->set_screen_orientation(OS::SCREEN_SENSOR_LANDSCAPE);
		else if (orientation == "sensor_portrait")
			OS::get_singleton()->set_screen_orientation(OS::SCREEN_SENSOR_PORTRAIT);
		else if (orientation == "sensor")
			OS::get_singleton()->set_screen_orientation(OS::SCREEN_SENSOR);
		else
			OS::get_singleton()->set_screen_orientation(OS::SCREEN_LANDSCAPE);
	}

	Engine::get_singleton()->set_iterations_per_second(GLOBAL_DEF("physics/common/physics_fps", 60));
	ProjectSettings::get_singleton()->set_custom_property_info("physics/common/physics_fps", PropertyInfo(Variant::INT, "physics/common/physics_fps", PROPERTY_HINT_RANGE, "1,1000,1"));
	Engine::get_singleton()->set_physics_jitter_fix(GLOBAL_DEF("physics/common/physics_jitter_fix", 0.5));
	Engine::get_singleton()->set_target_fps(GLOBAL_DEF("debug/settings/fps/force_fps", 0));
	ProjectSettings::get_singleton()->set_custom_property_info("debug/settings/fps/force_fps", PropertyInfo(Variant::INT, "debug/settings/fps/force_fps", PROPERTY_HINT_RANGE, "0,1000,1"));
	GLOBAL_DEF("physics/common/enable_pause_aware_picking", false);

	GLOBAL_DEF("debug/settings/stdout/print_fps", false);
	GLOBAL_DEF("debug/settings/stdout/verbose_stdout", false);

	if (!OS::get_singleton()->_verbose_stdout) { // Not manually overridden.
		OS::get_singleton()->_verbose_stdout = GLOBAL_GET("debug/settings/stdout/verbose_stdout");
	}

	if (frame_delay == 0) {
		frame_delay = GLOBAL_DEF("application/run/frame_delay_msec", 0);
		ProjectSettings::get_singleton()->set_custom_property_info("application/run/frame_delay_msec", PropertyInfo(Variant::INT, "application/run/frame_delay_msec", PROPERTY_HINT_RANGE, "0,100,1,or_greater")); // No negative numbers
	}

	OS::get_singleton()->set_low_processor_usage_mode(GLOBAL_DEF("application/run/low_processor_mode", false));
	OS::get_singleton()->set_low_processor_usage_mode_sleep_usec(GLOBAL_DEF("application/run/low_processor_mode_sleep_usec", 6900)); // Roughly 144 FPS
	ProjectSettings::get_singleton()->set_custom_property_info("application/run/low_processor_mode_sleep_usec", PropertyInfo(Variant::INT, "application/run/low_processor_mode_sleep_usec", PROPERTY_HINT_RANGE, "0,33200,1,or_greater")); // No negative numbers

	GLOBAL_DEF("display/window/ios/hide_home_indicator", true);
	GLOBAL_DEF("input_devices/pointing/ios/touch_delay", 0.150);

	Engine::get_singleton()->set_frame_delay(frame_delay);

	message_queue = memnew(MessageQueue);

	if (p_second_phase)
		return setup2();

	return OK;

error:

	video_driver = "";
	audio_driver = "";
	tablet_driver = "";
	project_path = "";

	args.clear();
	main_args.clear();

	if (show_help)
		print_help(execpath);

	if (performance)
		memdelete(performance);
	if (input_map)
		memdelete(input_map);
	if (translation_server)
		memdelete(translation_server);
	if (globals)
		memdelete(globals);
	if (engine)
		memdelete(engine);
	if (script_debugger)
		memdelete(script_debugger);
	if (packed_data)
		memdelete(packed_data);
	if (file_access_network_client)
		memdelete(file_access_network_client);

	unregister_core_driver_types();
	unregister_core_types();

	OS::get_singleton()->_cmdline.clear();

	if (message_queue)
		memdelete(message_queue);
	OS::get_singleton()->finalize_core();
	locale = String();

	return ERR_INVALID_PARAMETER;
}

Error Main::setup2(Thread::ID p_main_tid_override) {
	// Print engine name and version
	print_line(String(VERSION_NAME) + " v" + get_full_version_string() + " - " + String(VERSION_WEBSITE));

#if !defined(NO_THREADS)
	if (p_main_tid_override) {
		Thread::main_thread_id = p_main_tid_override;
	}
#endif

	Error err = OS::get_singleton()->initialize(video_mode, video_driver_idx, audio_driver_idx);
	if (err != OK) {
		return err;
	}

	print_line(" "); //add a blank line for readability

	if (init_use_custom_pos) {
		OS::get_singleton()->set_window_position(init_custom_pos);
	}

	// right moment to create and initialize the audio server

	audio_server = memnew(AudioServer);
	audio_server->init();

	// also init our arvr_server from here
	arvr_server = memnew(ARVRServer);

	// and finally setup this property under visual_server
	VisualServer::get_singleton()->set_render_loop_enabled(!disable_render_loop);

	register_core_singletons();

	MAIN_PRINT("Main: Setup Logo");

#if defined(JAVASCRIPT_ENABLED) || defined(ANDROID_ENABLED)
	bool show_logo = false;
#else
	bool show_logo = true;
#endif

	if (init_screen != -1) {
		OS::get_singleton()->set_current_screen(init_screen);
	}
	if (init_windowed) {
		//do none..
	} else if (init_maximized) {
		OS::get_singleton()->set_window_maximized(true);
	} else if (init_fullscreen) {
		OS::get_singleton()->set_window_fullscreen(true);
	}
	if (init_always_on_top) {
		OS::get_singleton()->set_window_always_on_top(true);
	}

	register_server_types();

	MAIN_PRINT("Main: Load Boot Image");

	Color clear = GLOBAL_DEF("rendering/environment/default_clear_color", Color(0.3, 0.3, 0.3));
	VisualServer::get_singleton()->set_default_clear_color(clear);

	if (show_logo) { //boot logo!
		String boot_logo_path = GLOBAL_DEF("application/boot_splash/image", String());
		bool boot_logo_scale = GLOBAL_DEF("application/boot_splash/fullsize", true);
		bool boot_logo_filter = GLOBAL_DEF("application/boot_splash/use_filter", true);
		ProjectSettings::get_singleton()->set_custom_property_info("application/boot_splash/image", PropertyInfo(Variant::STRING, "application/boot_splash/image", PROPERTY_HINT_FILE, "*.png"));

		Ref<Image> boot_logo;

		boot_logo_path = boot_logo_path.strip_edges();

		if (boot_logo_path != String()) {
			boot_logo.instance();
			Error load_err = ImageLoader::load_image(boot_logo_path, boot_logo);
			if (load_err)
				ERR_PRINTS("Non-existing or invalid boot splash at '" + boot_logo_path + "'. Loading default splash.");
		}

#if defined(TOOLS_ENABLED) && !defined(NO_EDITOR_SPLASH)
		const Color boot_bg_color =
				GLOBAL_DEF("application/boot_splash/bg_color",
						(editor || project_manager) ? boot_splash_editor_bg_color : boot_splash_bg_color);
#else
		const Color boot_bg_color = GLOBAL_DEF("application/boot_splash/bg_color", boot_splash_bg_color);
#endif
		if (boot_logo.is_valid()) {
			OS::get_singleton()->_msec_splash = OS::get_singleton()->get_ticks_msec();
			VisualServer::get_singleton()->set_boot_image(boot_logo, boot_bg_color, boot_logo_scale, boot_logo_filter);

		} else {
#ifndef NO_DEFAULT_BOOT_LOGO
			MAIN_PRINT("Main: Create bootsplash");
#if defined(TOOLS_ENABLED) && !defined(NO_EDITOR_SPLASH)
			Ref<Image> splash = (editor || project_manager) ? memnew(Image(boot_splash_editor_png)) : memnew(Image(boot_splash_png));
#else
			Ref<Image> splash = memnew(Image(boot_splash_png));
#endif

			MAIN_PRINT("Main: ClearColor");
			VisualServer::get_singleton()->set_default_clear_color(boot_bg_color);
			MAIN_PRINT("Main: Image");
			VisualServer::get_singleton()->set_boot_image(splash, boot_bg_color, false);
#endif
		}

#ifdef TOOLS_ENABLED
		Ref<Image> icon = memnew(Image(app_icon_png));
		OS::get_singleton()->set_icon(icon);
#endif
	}

	MAIN_PRINT("Main: DCC");
	VisualServer::get_singleton()->set_default_clear_color(GLOBAL_DEF("rendering/environment/default_clear_color", Color(0.3, 0.3, 0.3)));

	GLOBAL_DEF("application/config/icon", String());
	ProjectSettings::get_singleton()->set_custom_property_info("application/config/icon",
			PropertyInfo(Variant::STRING, "application/config/icon",
					PROPERTY_HINT_FILE, "*.png,*.webp,*.svg,*.svgz"));

	GLOBAL_DEF("application/config/macos_native_icon", String());
	ProjectSettings::get_singleton()->set_custom_property_info("application/config/macos_native_icon", PropertyInfo(Variant::STRING, "application/config/macos_native_icon", PROPERTY_HINT_FILE, "*.icns"));

	GLOBAL_DEF("application/config/windows_native_icon", String());
	ProjectSettings::get_singleton()->set_custom_property_info("application/config/windows_native_icon", PropertyInfo(Variant::STRING, "application/config/windows_native_icon", PROPERTY_HINT_FILE, "*.ico"));

	InputDefault *id = Object::cast_to<InputDefault>(Input::get_singleton());
	if (id) {
		if (bool(GLOBAL_DEF("input_devices/pointing/emulate_touch_from_mouse", false)) && !(editor || project_manager)) {
			if (!OS::get_singleton()->has_touchscreen_ui_hint()) {
				//only if no touchscreen ui hint, set emulation
				id->set_emulate_touch_from_mouse(true);
			}
		}

		id->set_emulate_mouse_from_touch(bool(GLOBAL_DEF("input_devices/pointing/emulate_mouse_from_touch", true)));
	}

	MAIN_PRINT("Main: Load Translations and Remaps");

	translation_server->setup(); //register translations, load them, etc.
	if (locale != "") {
		translation_server->set_locale(locale);
	}
	translation_server->load_translations();
	ResourceLoader::load_translation_remaps(); //load remaps for resources

	ResourceLoader::load_path_remaps();

	MAIN_PRINT("Main: Load Scene Types");

	register_scene_types();

#ifdef TOOLS_ENABLED
	ClassDB::set_current_api(ClassDB::API_EDITOR);
	EditorNode::register_editor_types();

	ClassDB::set_current_api(ClassDB::API_CORE);

#endif

	MAIN_PRINT("Main: Load Modules, Physics, Drivers, Scripts");

	register_platform_apis();
	register_module_types();

	GLOBAL_DEF("display/mouse_cursor/custom_image", String());
	GLOBAL_DEF("display/mouse_cursor/custom_image_hotspot", Vector2());
	GLOBAL_DEF("display/mouse_cursor/tooltip_position_offset", Point2(10, 10));
	ProjectSettings::get_singleton()->set_custom_property_info("display/mouse_cursor/custom_image", PropertyInfo(Variant::STRING, "display/mouse_cursor/custom_image", PROPERTY_HINT_FILE, "*.png,*.webp"));

	if (String(ProjectSettings::get_singleton()->get("display/mouse_cursor/custom_image")) != String()) {
		Ref<Texture> cursor = ResourceLoader::load(ProjectSettings::get_singleton()->get("display/mouse_cursor/custom_image"));
		if (cursor.is_valid()) {
			Vector2 hotspot = ProjectSettings::get_singleton()->get("display/mouse_cursor/custom_image_hotspot");
			Input::get_singleton()->set_custom_mouse_cursor(cursor, Input::CURSOR_ARROW, hotspot);
		}
	}

	camera_server = CameraServer::create();

	initialize_physics();
	register_server_singletons();

	register_driver_types();

	// This loads global classes, so it must happen before custom loaders and savers are registered
	ScriptServer::init_languages();

	audio_server->load_default_bus_layout();

	if (use_debug_profiler && script_debugger) {
		script_debugger->profiling_start();
	}
	_start_success = true;
	locale = String();

	ClassDB::set_current_api(ClassDB::API_NONE); //no more api is registered at this point

	print_verbose("CORE API HASH: " + uitos(ClassDB::get_api_hash(ClassDB::API_CORE)));
	print_verbose("EDITOR API HASH: " + uitos(ClassDB::get_api_hash(ClassDB::API_EDITOR)));
	MAIN_PRINT("Main: Done");

	return OK;
}

// everything the main loop needs to know about frame timings
static MainTimerSync main_timer_sync;

bool Main::start() {
	ERR_FAIL_COND_V(!_start_success, false);

	bool hasicon = false;
	String game_path;
	String script;
	String test;
	bool check_only = false;

	main_timer_sync.init(OS::get_singleton()->get_ticks_usec());

	List<String> args = OS::get_singleton()->get_cmdline_args();
	for (int i = 0; i < args.size(); i++) {
		//parameters that do not have an argument to the right
		if (args[i] == "--check-only") {
			check_only = true;
			//parameters that have an argument to the right
		} else if (i < (args.size() - 1)) {
			bool parsed_pair = true;
			if (args[i] == "-s" || args[i] == "--script") {
				script = args[i + 1];
			} else if (args[i] == "--test") {
				test = args[i + 1];
			} else {
				// The parameter does not match anything known, don't skip the next argument
				parsed_pair = false;
			}
			if (parsed_pair) {
				i++;
			}
		} else if (args[i].length() && args[i][0] != '-') {
			if (args[i].ends_with(".scn") ||
					args[i].ends_with(".tscn") ||
					args[i].ends_with(".escn") ||
					args[i].ends_with(".res") ||
					args[i].ends_with(".tres")) {
				game_path = args[i];
			}
		}
	}

	String main_loop_type;

	if (script == "" && game_path == "" && String(GLOBAL_DEF("application/run/main_scene", "")) != "") {
		game_path = GLOBAL_DEF("application/run/main_scene", "");
	}

	MainLoop *main_loop = NULL;
#ifdef TOOLS_ENABLED
	if (editor) {
		main_loop = editor_main("editor", args);
	};
#endif

	if (test != "") {
#ifdef TOOLS_ENABLED
		main_loop = test_main(test, args);

		if (!main_loop)
			return false;
#endif

	} else if (script != "") {
		Ref<Script> script_res = ResourceLoader::load(script);
		ERR_FAIL_COND_V_MSG(script_res.is_null(), false, "Can't load script: " + script);

		if (check_only) {
			if (!script_res->is_valid()) {
				OS::get_singleton()->set_exit_code(1);
			}
			return false;
		}

		if (script_res->can_instance()) {
			StringName instance_type = script_res->get_instance_base_type();
			Object *obj = ClassDB::instance(instance_type);
			MainLoop *script_loop = Object::cast_to<MainLoop>(obj);
			if (!script_loop) {
				if (obj)
					memdelete(obj);
				ERR_FAIL_V_MSG(false, vformat("Can't load the script \"%s\" as it doesn't inherit from SceneTree or MainLoop.", script));
			}

			script_loop->set_init_script(script_res);
			main_loop = script_loop;
		} else {
			return false;
		}

	} else {
		main_loop_type = GLOBAL_DEF("application/run/main_loop_type", "");
	}

#ifdef TOOLS_ENABLED
	if (project_manager || (script == "" && test == "" && game_path == "" && !editor)) {
		main_loop = editor_main("project_manager", args);
		OS::get_singleton()->set_context(OS::CONTEXT_PROJECTMAN);
		project_manager = true;
	}
#endif

	if (!main_loop && main_loop_type == "")
		main_loop_type = "SceneTree";

	if (!main_loop) {
		if (!ClassDB::class_exists(main_loop_type)) {
			OS::get_singleton()->alert("Error: MainLoop type doesn't exist: " + main_loop_type);
			return false;
		} else {
			Object *ml = ClassDB::instance(main_loop_type);
			ERR_FAIL_COND_V_MSG(!ml, false, "Can't instance MainLoop type.");

			main_loop = Object::cast_to<MainLoop>(ml);
			if (!main_loop) {
				memdelete(ml);
				ERR_FAIL_V_MSG(false, "Invalid MainLoop type.");
			}
		}
	}

	if (main_loop->is_class("SceneTree")) {
		SceneTree *sml = Object::cast_to<SceneTree>(main_loop);

#ifdef DEBUG_ENABLED
		if (debug_collisions) {
			sml->set_debug_collisions_hint(true);
		}
		if (debug_navigation) {
			sml->set_debug_navigation_hint(true);
		}
#endif

		ResourceLoader::add_custom_loaders();
		ResourceSaver::add_custom_savers();

		if (!project_manager && !editor) { // game
			if (game_path != "" || script != "") {
				if (script_debugger && script_debugger->is_remote()) {
					ScriptDebuggerRemote *remote_debugger = static_cast<ScriptDebuggerRemote *>(script_debugger);

					remote_debugger->set_scene_tree(sml);

					Array msg;
					int64_t handle = (int64_t)OS::get_singleton()->get_native_handle(OS::HandleType::WINDOW_HANDLE);
					msg.push_back(handle);
					remote_debugger->send_message("set_remote_window_handle", msg);
				}

				//autoload
				List<PropertyInfo> props;
				ProjectSettings::get_singleton()->get_property_list(&props);

				//first pass, add the constants so they exist before any script is loaded
				for (List<PropertyInfo>::Element *E = props.front(); E; E = E->next()) {
					String s = E->get().name;
					if (!s.begins_with("autoload/"))
						continue;
					String name = s.get_slicec('/', 1);
					String path = ProjectSettings::get_singleton()->get(s);
					bool global_var = false;
					if (path.begins_with("*")) {
						global_var = true;
					}

					if (global_var) {
						for (int i = 0; i < ScriptServer::get_language_count(); i++) {
							ScriptServer::get_language(i)->add_global_constant(name, Variant());
						}
					}
				}

				//second pass, load into global constants
				List<Node *> to_add;
				for (List<PropertyInfo>::Element *E = props.front(); E; E = E->next()) {
					String s = E->get().name;
					if (!s.begins_with("autoload/"))
						continue;
					String name = s.get_slicec('/', 1);
					String path = ProjectSettings::get_singleton()->get(s);
					bool global_var = false;
					if (path.begins_with("*")) {
						global_var = true;
						path = path.substr(1, path.length() - 1);
					}

					RES res = ResourceLoader::load(path);
					ERR_CONTINUE_MSG(res.is_null(), "Can't autoload: " + path);
					Node *n = NULL;
					if (res->is_class("PackedScene")) {
						Ref<PackedScene> ps = res;
						n = ps->instance();
					} else if (res->is_class("Script")) {
						Ref<Script> script_res = res;
						StringName ibt = script_res->get_instance_base_type();
						bool valid_type = ClassDB::is_parent_class(ibt, "Node");
						ERR_CONTINUE_MSG(!valid_type, "Script does not inherit a Node: " + path);

						Object *obj = ClassDB::instance(ibt);

						ERR_CONTINUE_MSG(obj == NULL, "Cannot instance script for autoload, expected 'Node' inheritance, got: " + String(ibt));

						n = Object::cast_to<Node>(obj);
						n->set_script(script_res.get_ref_ptr());
					}

					ERR_CONTINUE_MSG(!n, "Path in autoload not a node or script: " + path);
					n->set_name(name);

					//defer so references are all valid on _ready()
					to_add.push_back(n);

					if (global_var) {
						for (int i = 0; i < ScriptServer::get_language_count(); i++) {
							ScriptServer::get_language(i)->add_global_constant(name, n);
						}
					}
				}

				for (List<Node *>::Element *E = to_add.front(); E; E = E->next()) {
					sml->get_root()->add_child(E->get());
				}
			}
		}

#ifdef TOOLS_ENABLED
		if (Editor::may_export_preset()) {
			game_path = "";
		}
#endif

		if (!editor && !project_manager) {
			//standard helpers that can be changed from main config

			String stretch_mode = GLOBAL_DEF("display/window/stretch/mode", "disabled");
			String stretch_aspect = GLOBAL_DEF("display/window/stretch/aspect", "ignore");
			Size2i stretch_size = Size2(GLOBAL_DEF("display/window/size/width", 0), GLOBAL_DEF("display/window/size/height", 0));
			real_t stretch_shrink = GLOBAL_DEF("display/window/stretch/shrink", 1.0);

			SceneTree::StretchMode sml_sm = SceneTree::STRETCH_MODE_DISABLED;
			if (stretch_mode == "2d")
				sml_sm = SceneTree::STRETCH_MODE_2D;
			else if (stretch_mode == "viewport")
				sml_sm = SceneTree::STRETCH_MODE_VIEWPORT;

			SceneTree::StretchAspect sml_aspect = SceneTree::STRETCH_ASPECT_IGNORE;
			if (stretch_aspect == "keep")
				sml_aspect = SceneTree::STRETCH_ASPECT_KEEP;
			else if (stretch_aspect == "keep_width")
				sml_aspect = SceneTree::STRETCH_ASPECT_KEEP_WIDTH;
			else if (stretch_aspect == "keep_height")
				sml_aspect = SceneTree::STRETCH_ASPECT_KEEP_HEIGHT;
			else if (stretch_aspect == "expand")
				sml_aspect = SceneTree::STRETCH_ASPECT_EXPAND;

			sml->set_screen_stretch(sml_sm, sml_aspect, stretch_size, stretch_shrink);

			sml->set_auto_accept_quit(GLOBAL_DEF("application/config/auto_accept_quit", true));
			sml->set_quit_on_go_back(GLOBAL_DEF("application/config/quit_on_go_back", true));
			String appname = ProjectSettings::get_singleton()->get("application/config/name");
			appname = TranslationServer::get_singleton()->translate(appname);
#ifdef DEBUG_ENABLED
			// Append a suffix to the window title to denote that the project is running
			// from a debug build (including the editor). Since this results in lower performance,
			// this should be clearly presented to the user.
			OS::get_singleton()->set_window_title(vformat("%s (DEBUG)", appname));
#else
			OS::get_singleton()->set_window_title(appname);
#endif

			int shadow_atlas_size = GLOBAL_GET("rendering/quality/shadow_atlas/size");
			int shadow_atlas_q0_subdiv = GLOBAL_GET("rendering/quality/shadow_atlas/quadrant_0_subdiv");
			int shadow_atlas_q1_subdiv = GLOBAL_GET("rendering/quality/shadow_atlas/quadrant_1_subdiv");
			int shadow_atlas_q2_subdiv = GLOBAL_GET("rendering/quality/shadow_atlas/quadrant_2_subdiv");
			int shadow_atlas_q3_subdiv = GLOBAL_GET("rendering/quality/shadow_atlas/quadrant_3_subdiv");

			sml->get_root()->set_shadow_atlas_size(shadow_atlas_size);
			sml->get_root()->set_shadow_atlas_quadrant_subdiv(0, Viewport::ShadowAtlasQuadrantSubdiv(shadow_atlas_q0_subdiv));
			sml->get_root()->set_shadow_atlas_quadrant_subdiv(1, Viewport::ShadowAtlasQuadrantSubdiv(shadow_atlas_q1_subdiv));
			sml->get_root()->set_shadow_atlas_quadrant_subdiv(2, Viewport::ShadowAtlasQuadrantSubdiv(shadow_atlas_q2_subdiv));
			sml->get_root()->set_shadow_atlas_quadrant_subdiv(3, Viewport::ShadowAtlasQuadrantSubdiv(shadow_atlas_q3_subdiv));
			Viewport::Usage usage = Viewport::Usage(int(GLOBAL_GET("rendering/quality/intended_usage/framebuffer_allocation")));
			sml->get_root()->set_usage(usage);

			bool snap_controls = GLOBAL_DEF("gui/common/snap_controls_to_pixels", true);
			sml->get_root()->set_snap_controls_to_pixels(snap_controls);

			bool font_oversampling = GLOBAL_DEF("rendering/quality/dynamic_fonts/use_oversampling", true);
			sml->set_use_font_oversampling(font_oversampling);

		} else {
			GLOBAL_DEF("display/window/stretch/mode", "disabled");
			ProjectSettings::get_singleton()->set_custom_property_info("display/window/stretch/mode", PropertyInfo(Variant::STRING, "display/window/stretch/mode", PROPERTY_HINT_ENUM, "disabled,2d,viewport"));
			GLOBAL_DEF("display/window/stretch/aspect", "ignore");
			ProjectSettings::get_singleton()->set_custom_property_info("display/window/stretch/aspect", PropertyInfo(Variant::STRING, "display/window/stretch/aspect", PROPERTY_HINT_ENUM, "ignore,keep,keep_width,keep_height,expand"));
			GLOBAL_DEF("display/window/stretch/shrink", 1.0);
			ProjectSettings::get_singleton()->set_custom_property_info("display/window/stretch/shrink", PropertyInfo(Variant::REAL, "display/window/stretch/shrink", PROPERTY_HINT_RANGE, "1.0,8.0,0.1"));
			sml->set_auto_accept_quit(GLOBAL_DEF("application/config/auto_accept_quit", true));
			sml->set_quit_on_go_back(GLOBAL_DEF("application/config/quit_on_go_back", true));
			GLOBAL_DEF("gui/common/snap_controls_to_pixels", true);
			GLOBAL_DEF("rendering/quality/dynamic_fonts/use_oversampling", true);
		}

		String local_game_path;
		if (game_path != "" && !project_manager) {
			local_game_path = game_path.replace("\\", "/");

			if (!local_game_path.begins_with("res://")) {
				bool absolute = (local_game_path.size() > 1) && (local_game_path[0] == '/' || local_game_path[1] == ':');

				if (!absolute) {
					if (ProjectSettings::get_singleton()->is_using_datapack()) {
						local_game_path = "res://" + local_game_path;

					} else {
						int sep = local_game_path.find_last("/");

						if (sep == -1) {
							DirAccess *da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
							local_game_path = da->get_current_dir().plus_file(local_game_path);
							memdelete(da);
						} else {
							DirAccess *da = DirAccess::open(local_game_path.substr(0, sep));
							if (da) {
								local_game_path = da->get_current_dir().plus_file(local_game_path.substr(sep + 1, local_game_path.length()));
								memdelete(da);
							}
						}
					}
				}
			}

			local_game_path = ProjectSettings::get_singleton()->localize_path(local_game_path);

#ifdef TOOLS_ENABLED
			if (editor) {
				Editor::load_scene(local_game_path);
				OS::get_singleton()->set_context(OS::CONTEXT_EDITOR);
			}
#endif
			if (!editor) {
				OS::get_singleton()->set_context(OS::CONTEXT_ENGINE);
			}
		}

		if (!project_manager && !editor) { // game

			// Load SSL Certificates from Project Settings (or builtin).
			Crypto::load_default_certificates(GLOBAL_DEF("network/ssl/certificates", ""));

			if (game_path != "") {
				Node *scene = NULL;
				Ref<PackedScene> scenedata = ResourceLoader::load(local_game_path);
				if (scenedata.is_valid())
					scene = scenedata->instance();

				ERR_FAIL_COND_V_MSG(!scene, false, "Failed loading scene: " + local_game_path);
				sml->add_current_scene(scene);

#ifdef OSX_ENABLED
				String mac_iconpath = GLOBAL_DEF("application/config/macos_native_icon", "Variant()");
				if (mac_iconpath != "") {
					OS::get_singleton()->set_native_icon(mac_iconpath);
					hasicon = true;
				}
#endif

#ifdef WINDOWS_ENABLED
				String win_iconpath = GLOBAL_DEF("application/config/windows_native_icon", "Variant()");
				if (win_iconpath != "") {
					OS::get_singleton()->set_native_icon(win_iconpath);
					hasicon = true;
				}
#endif

				String iconpath = GLOBAL_DEF("application/config/icon", "Variant()");
				if ((iconpath != "") && (!hasicon)) {
					Ref<Image> icon;
					icon.instance();
					if (ImageLoader::load_image(iconpath, icon) == OK) {
						OS::get_singleton()->set_icon(icon);
						hasicon = true;
					}
				}
			}
		}
	}

	if (!hasicon) {
		Ref<Image> icon = memnew(Image(app_icon_png));
		OS::get_singleton()->set_icon(icon);
	}

	OS::get_singleton()->set_main_loop(main_loop);

	return true;
}

/* Main iteration
 *
 * This is the iteration of the engine's game loop, advancing the state of physics,
 * rendering and audio.
 * It's called directly by the platform's OS::run method, where the loop is created
 * and monitored.
 *
 * The OS implementation can impact its draw step with the Main::force_redraw() method.
 */

uint64_t Main::last_ticks = 0;
uint32_t Main::frames = 0;
uint32_t Main::frame = 0;
bool Main::force_redraw_requested = false;
int Main::iterating = 0;
bool Main::is_iterating() {
	return iterating > 0;
}

// For performance metrics.
static uint64_t physics_process_max = 0;
static uint64_t idle_process_max = 0;

bool Main::iteration() {
	//for now do not error on this
	//ERR_FAIL_COND_V(iterating, false);

	iterating++;

	uint64_t ticks = OS::get_singleton()->get_ticks_usec();
	Engine::get_singleton()->_frame_ticks = ticks;
	main_timer_sync.set_cpu_ticks_usec(ticks);
	main_timer_sync.set_fixed_fps(fixed_fps);

	uint64_t ticks_elapsed = ticks - last_ticks;

	int physics_fps = Engine::get_singleton()->get_iterations_per_second();
	float frame_slice = 1.0 / physics_fps;

	float time_scale = Engine::get_singleton()->get_time_scale();

	MainFrameTime advance = main_timer_sync.advance(frame_slice, physics_fps);
	double step = advance.idle_step;
	double scaled_step = step * time_scale;

	Engine::get_singleton()->_frame_step = step;
	Engine::get_singleton()->_physics_interpolation_fraction = advance.interpolation_fraction;

	uint64_t physics_process_ticks = 0;
	uint64_t idle_process_ticks = 0;

	frame += ticks_elapsed;

	last_ticks = ticks;

	static const int max_physics_steps = 8;
	if (fixed_fps == -1 && advance.physics_steps > max_physics_steps) {
		step -= (advance.physics_steps - max_physics_steps) * frame_slice;
		advance.physics_steps = max_physics_steps;
	}

	bool exit = false;

	Engine::get_singleton()->_in_physics = true;

	for (int iters = 0; iters < advance.physics_steps; ++iters) {
		uint64_t physics_begin = OS::get_singleton()->get_ticks_usec();

		PhysicsServer::get_singleton()->flush_queries();

		Physics2DServer::get_singleton()->sync();
		Physics2DServer::get_singleton()->flush_queries();

		if (OS::get_singleton()->get_main_loop()->iteration(frame_slice * time_scale)) {
			exit = true;
			break;
		}

		message_queue->flush();

		PhysicsServer::get_singleton()->step(frame_slice * time_scale);

		Physics2DServer::get_singleton()->end_sync();
		Physics2DServer::get_singleton()->step(frame_slice * time_scale);

		message_queue->flush();

		physics_process_ticks = MAX(physics_process_ticks, OS::get_singleton()->get_ticks_usec() - physics_begin); // keep the largest one for reference
		physics_process_max = MAX(OS::get_singleton()->get_ticks_usec() - physics_begin, physics_process_max);
		Engine::get_singleton()->_physics_frames++;
	}

	Engine::get_singleton()->_in_physics = false;

	uint64_t idle_begin = OS::get_singleton()->get_ticks_usec();

	if (OS::get_singleton()->get_main_loop()->idle(step * time_scale)) {
		exit = true;
	}
	message_queue->flush();

	VisualServer::get_singleton()->sync(); //sync if still drawing from previous frames.

	if (OS::get_singleton()->can_draw() && VisualServer::get_singleton()->is_render_loop_enabled()) {
		if ((!force_redraw_requested) && OS::get_singleton()->is_in_low_processor_usage_mode()) {
			if (VisualServer::get_singleton()->has_changed()) {
				VisualServer::get_singleton()->draw(true, scaled_step); // flush visual commands
				Engine::get_singleton()->frames_drawn++;
			}
		} else {
			VisualServer::get_singleton()->draw(true, scaled_step); // flush visual commands
			Engine::get_singleton()->frames_drawn++;
			force_redraw_requested = false;
		}
	}

	idle_process_ticks = OS::get_singleton()->get_ticks_usec() - idle_begin;
	idle_process_max = MAX(idle_process_ticks, idle_process_max);
	uint64_t frame_time = OS::get_singleton()->get_ticks_usec() - ticks;

	for (int i = 0; i < ScriptServer::get_language_count(); i++) {
		ScriptServer::get_language(i)->frame();
	}

	AudioServer::get_singleton()->update();

	if (script_debugger) {
		if (script_debugger->is_profiling()) {
			script_debugger->profiling_set_frame_times(USEC_TO_SEC(frame_time), USEC_TO_SEC(idle_process_ticks), USEC_TO_SEC(physics_process_ticks), frame_slice);
		}
		script_debugger->idle_poll();
	}

	frames++;
	Engine::get_singleton()->_idle_frames++;

	if (frame > 1000000) {
		if (editor || project_manager) {
			if (print_fps) {
				print_line(vformat("Editor FPS: %d (%s mspf)", frames, rtos(1000.0 / frames).pad_decimals(1)));
			}
		} else if (GLOBAL_GET("debug/settings/stdout/print_fps") || print_fps) {
			print_line(vformat("Project FPS: %d (%s mspf)", frames, rtos(1000.0 / frames).pad_decimals(1)));
		}

		Engine::get_singleton()->_fps = frames;
		performance->set_process_time(USEC_TO_SEC(idle_process_max));
		performance->set_physics_process_time(USEC_TO_SEC(physics_process_max));
		idle_process_max = 0;
		physics_process_max = 0;

		frame %= 1000000;
		frames = 0;
	}

	iterating--;

	if (fixed_fps != -1)
		return exit;

	OS::get_singleton()->add_frame_delay(OS::get_singleton()->can_draw());

#ifdef TOOLS_ENABLED
	if (auto_build_solutions) {
		auto_build_solutions = false;
		// Only relevant when running the editor.
		if (!editor) {
			ERR_FAIL_V_MSG(true, "Command line option --build-solutions was passed, but no project is being edited. Aborting.");
		}
		if (!EditorNode::get_singleton()->call_build()) {
			ERR_FAIL_V_MSG(true, "Command line option --build-solutions was passed, but the build callback failed. Aborting.");
		}
	}
#endif

	return exit || auto_quit;
}

void Main::force_redraw() {
	force_redraw_requested = true;
}

/* Engine deinitialization
 *
 * Responsible for freeing all the memory allocated by previous setup steps,
 * so that the engine closes cleanly without leaking memory or crashing.
 * The order matters as some of those steps are linked with each other.
 */
void Main::cleanup(bool p_force) {
	if (!p_force) {
		ERR_FAIL_COND(!_start_success);
	}

	if (script_debugger) {
		// Flush any remaining messages
		script_debugger->idle_poll();
	}

	ResourceLoader::remove_custom_loaders();
	ResourceSaver::remove_custom_savers();

	// Flush before uninitializing the scene, but delete the MessageQueue as late as possible.
	message_queue->flush();

	if (script_debugger) {
		if (use_debug_profiler) {
			script_debugger->profiling_end();
		}

		memdelete(script_debugger);
	}

	OS::get_singleton()->delete_main_loop();

	OS::get_singleton()->_cmdline.clear();
	OS::get_singleton()->_execpath = "";
	OS::get_singleton()->_local_clipboard = "";

	ResourceLoader::clear_translation_remaps();
	ResourceLoader::clear_path_remaps();

	ScriptServer::finish_languages();

	// Sync pending commands that may have been queued from a different thread during ScriptServer finalization
	VisualServer::get_singleton()->sync();

#ifdef TOOLS_ENABLED
	EditorNode::unregister_editor_types();
#endif

	if (arvr_server) {
		// cleanup now before we pull the rug from underneath...
		memdelete(arvr_server);
	}

	ImageLoader::cleanup();

	unregister_driver_types();
	unregister_module_types();
	unregister_platform_apis();
	unregister_scene_types();
	unregister_server_types();

	if (audio_server) {
		audio_server->finish();
		memdelete(audio_server);
	}

	if (camera_server) {
		memdelete(camera_server);
	}

	OS::get_singleton()->finalize();
	finalize_physics();

	if (packed_data)
		memdelete(packed_data);
	if (file_access_network_client)
		memdelete(file_access_network_client);
	if (performance)
		memdelete(performance);
	if (input_map)
		memdelete(input_map);
	if (translation_server)
		memdelete(translation_server);
	if (globals)
		memdelete(globals);
	if (engine)
		memdelete(engine);

	if (OS::get_singleton()->is_restart_on_exit_set()) {
		//attempt to restart with arguments
		String exec = OS::get_singleton()->get_executable_path();
		List<String> args = OS::get_singleton()->get_restart_on_exit_arguments();
		OS::ProcessID pid = 0;
		OS::get_singleton()->execute(exec, args, false, &pid);
		OS::get_singleton()->set_restart_on_exit(false, List<String>()); //clear list (uses memory)
	}

	// Now should be safe to delete MessageQueue (famous last words).
	message_queue->flush();
	memdelete(message_queue);

	unregister_core_driver_types();
	unregister_core_types();

	OS::get_singleton()->finalize_core();
}
