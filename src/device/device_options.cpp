/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/command_line.h"
#include "core/filesystem/path.h"
#include "device/device_options.h"
#include <stdlib.h>

namespace crown
{
static void help(const char* msg = NULL)
{
	printf(
		"The Flexible Game Engine\n"
		"Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.\n"
		"License: https://github.com/dbartolini/crown/blob/master/LICENSE\n"
		"\n"
		"Usage:\n"
		"  crown [options]\n"
		"\n"
		"Options:\n"
		"  -h --help                       Display this help.\n"
		"  -v --version                    Display engine version.\n"
		"  --source-dir <path>             Use <path> as the source directory for resource compilation.\n"
		"  --data-dir <path>               Use <path> as the destination directory for compiled resources.\n"
		"  --map-source-dir <name> <path>  Mount <path>/<name> at <source-dir>/<name>.\n"
		"  --boot-dir <prefix>             Use <prefix>/boot.config to boot the engine.\n"
		"  --compile                       Do a full compile of the resources.\n"
		"  --platform <platform>           Compile resources for the given <platform>.\n"
		"      linux\n"
		"      windows\n"
		"      android\n"
		"  --continue                      Run the engine after resource compilation.\n"
		"  --console-port <port>           Set port of the console.\n"
		"  --wait-console                  Wait for a console connection before starting up.\n"
		"  --parent-window <handle>        Set the parent window <handle> of the main window.\n"
		"  --server                        Run the engine in server mode.\n"
		"\n"
		"Complete documentation available at https://dbartolini.github.io/crown/html\n"
	);

	if (msg)
		printf("Error: %s\n", msg);
}

DeviceOptions::DeviceOptions(Allocator& a, int argc, const char** argv)
	: _argc(argc)
	, _argv(argv)
	, _source_dir(a)
	, _map_source_dir_name(NULL)
	, _map_source_dir_prefix(a)
	, _data_dir(a)
	, _boot_dir(NULL)
	, _platform(NULL)
	, _lua_string(a)
	, _wait_console(false)
	, _do_compile(false)
	, _do_continue(false)
	, _server(false)
	, _parent_window(0)
	, _console_port(CROWN_DEFAULT_CONSOLE_PORT)
	, _window_x(0)
	, _window_y(0)
	, _window_width(CROWN_DEFAULT_WINDOW_WIDTH)
	, _window_height(CROWN_DEFAULT_WINDOW_HEIGHT)
{
}

int DeviceOptions::parse()
{
	CommandLine cl(_argc, _argv);

	if (cl.has_option("help", 'h'))
	{
		help();
		return EXIT_FAILURE;
	}

	if (cl.has_option("version", 'v'))
	{
		printf(CROWN_VERSION);
		return EXIT_FAILURE;
	}

	path::reduce(_source_dir, cl.get_parameter(0, "source-dir"));
	path::reduce(_data_dir, cl.get_parameter(0, "data-dir"));

	_map_source_dir_name = cl.get_parameter(0, "map-source-dir");
	if (_map_source_dir_name)
	{
		path::reduce(_map_source_dir_prefix, cl.get_parameter(1, "map-source-dir"));
		if (_map_source_dir_prefix.empty())
		{
			help("Mapped source directory must be specified.");
			return EXIT_FAILURE;
		}
	}

	_do_compile = cl.has_option("compile");
	if (_do_compile)
	{
		_platform = cl.get_parameter(0, "platform");

		// Compile for platform the executable is built for.
		if (!_platform)
			_platform = CROWN_PLATFORM_NAME;

		if (true
			&& strcmp(_platform, "android") != 0
			&& strcmp(_platform, "linux") != 0
			&& strcmp(_platform, "windows") != 0
			)
		{
			help("Cannot compile for the given platform.");
			return EXIT_FAILURE;
		}

		if (_source_dir.empty())
		{
			help("Source dir must be specified.");
			return EXIT_FAILURE;
		}

		if (_data_dir.empty())
		{
			_data_dir += _source_dir;
			_data_dir += '_';
			_data_dir += _platform;
		}
	}

	_server = cl.has_option("server");
	if (_server)
	{
		if (_source_dir.empty())
		{
			help("Source dir must be specified.");
			return EXIT_FAILURE;
		}
	}

	if (!_data_dir.empty())
	{
		if (!path::is_absolute(_data_dir.c_str()))
		{
			help("Data dir must be absolute.");
			return EXIT_FAILURE;
		}
	}

	if (!_source_dir.empty())
	{
		if (!path::is_absolute(_source_dir.c_str()))
		{
			help("Source dir must be absolute.");
			return EXIT_FAILURE;
		}
	}

	if (!_map_source_dir_prefix.empty())
	{
		if (!path::is_absolute(_map_source_dir_prefix.c_str()))
		{
			help("Mapped source dir must be absolute.");
			return EXIT_FAILURE;
		}
	}

	_do_continue = cl.has_option("continue");

	_boot_dir = cl.get_parameter(0, "boot-dir");
	if (_boot_dir)
	{
		if (!path::is_relative(_boot_dir))
		{
			help("Boot dir must be relative.");
			return EXIT_FAILURE;
		}
	}

	_wait_console = cl.has_option("wait-console");

	const char* parent = cl.get_parameter(0, "parent-window");
	if (parent)
	{
		if (sscanf(parent, "%u", &_parent_window) != 1)
		{
			help("Parent window is invalid.");
			return EXIT_FAILURE;
		}
	}

	const char* port = cl.get_parameter(0, "console-port");
	if (port)
	{
		if (sscanf(port, "%hu", &_console_port) != 1)
		{
			help("Console port is invalid.");
			return EXIT_FAILURE;
		}
	}

	const char* ls = cl.get_parameter(0, "lua-string");
	if (ls)
		_lua_string = ls;

	return EXIT_SUCCESS;
}

} // namespace crown
