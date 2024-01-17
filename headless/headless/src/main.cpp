#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>
#include <thread>
#include <string>

#include <CLI11.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include <json.hpp>

#include <headless/filter_pipeline.h>
#include <headless/camera_handler.h>
#include <headless/plc_handler.h>
#include <headless/frame.h>

struct configuration
{
	struct cam
	{
		std::string ip = "";
		uint16_t port = 0;
	} cam;
	struct plc
	{
		std::string ip = "";
		int rack = 0;
		int slot = 0;
		int db_number = 0;
		int db_size_bytes = 0;
		int db_offset = 0;
	} plc;

	void print()
	{
		std::cout
			<< "cam:\n"
			<< "\tip:\t\t" << cam.ip << "\n"
			<< "\tport:\t\t" << cam.port << "\n"
			<< "plc:\n"
			<< "\tip:\t\t" << plc.ip << "\n"
			<< "\tslot:\t\t" << plc.slot << "\n"
			<< "\track:\t\t" << plc.rack << "\n"
			<< "\tdb_num:\t\t" << plc.db_number << "\n"
			<< "\tdb_size_bytes:\t" << plc.db_size_bytes << "\n"
			<< "\tdb_offset:\t" << plc.db_offset << "\n";
	}
};

configuration config;
filter::filter_pipeline pipeline;
volatile std::atomic_bool done = false;

void setup_loggers();
const int parse_args(int argc, char** argv);	
const bool parse_config(const std::string& path);
const bool parse_filters(const std::string& path);
void signal_handler(int signum);

int main(int argc, char** argv)
{
	setup_loggers();
	// register signal_handler as the ctrl+c callback
	signal(SIGINT, signal_handler);

	int ret;
	ret = parse_args(argc, argv);
	if (ret != 0)
	{
		return ret;
	}

	plc::plc_handler plc;
	while (!done && 0 != plc.connect_to(config.plc.ip, config.plc.rack, config.plc.slot))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	}

	camera::camera_handler camera;
	while (!done && !camera.open(config.cam.ip, config.cam.port, 1000))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	}

	// loop indefinitely, filtering and sending frames to the plc
	while (!done)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		frame::Frame raw_frame;
		if (camera.get_next_frame(raw_frame))
		{
			cv::Mat raw_mat = frame::to_mat(raw_frame);
			if (pipeline.apply(raw_mat))
			{
				frame::Frame filtered_frame = frame::to_frame(raw_mat);

				std::vector<uint16_t> distance_map_16 = filtered_frame.data;
				std::vector<uint32_t> distance_map(distance_map_16.begin(), distance_map_16.begin() + (config.plc.db_size_bytes / sizeof(uint32_t)));

				ret = plc.write_udint(distance_map, config.plc.db_number, config.plc.db_offset);
				if (ret != 0)
				{
					spdlog::error("Failed to write frame #{} to PLC: {}", filtered_frame.number, CliErrorText(ret));
					plc.disconnect();
					std::this_thread::sleep_for(std::chrono::milliseconds(5000));
					plc.connect();
				}
			}
		}
	}

	plc.disconnect();

	return 0;
}

void setup_loggers()
{
	auto camera_logger = spdlog::stdout_color_mt("camera");
	auto plc_logger = spdlog::stdout_color_mt("plc");
	auto sickapi_logger = spdlog::stdout_color_mt("sickapi");
	auto filter_logger = spdlog::stdout_color_mt("filter");
	auto app_logger = spdlog::stdout_color_mt("app");
	spdlog::set_default_logger(app_logger);
	spdlog::set_level(spdlog::level::trace);
}

const int parse_args(int argc, char** argv)
{
	CLI::App app;

	std::vector<CLI::Option*> req_with_no_config;

	// configuration file
	app.set_config("--config, config", "", "Path to configuration file");

	// camera options
	req_with_no_config.push_back(app.add_option("--cam-ip", config.cam.ip, "Camera IP address"));
	app.add_option("--cam-port", config.cam.port, "Camera control port");

	// plc options
	req_with_no_config.push_back(app.add_option("--plc-ip", config.plc.ip, "PLC IP address"));
	req_with_no_config.push_back(app.add_option("--plc-db", config.plc.db_number, "PLC DB number"));
	req_with_no_config.push_back(app.add_option("--plc-db-size", config.plc.db_size_bytes, "PLC DB size in bytes"));
	app.add_option("--plc-db-offset", config.plc.db_offset, "PLC DB start offset");
	app.add_option("--plc-rack", config.plc.rack, "PLC rack number");
	app.add_option("--plc-slot", config.plc.slot, "PLC slot number");

	// filtering options
	app.add_option("--filters", "Path to filter file")->each([](const std::string& file) { parse_filters(file); })->expected(1);

	try
	{
		app.parse(argc, argv);

		if (app.count("config") > 0)
		{
			const std::string config_path = app["config"]->as<std::string>();
			return parse_config(config_path) ? 0 : 1;
		}
		else
		{
			std::vector<CLI::Option*> missing;
			for (const auto& option : req_with_no_config)
			{
				if (app.count(option->get_name()) < 1)
				{
					missing.push_back(option);
				}
			}
			if (!missing.empty())
			{
				std::cerr << "Missing required option(s): ";
				int i;
				for (i = 0; i < missing.size() - 1; ++i)
				{
					std::cerr << missing[i]->get_name() << ", ";
				}
				std::cerr << missing[i]->get_name() << "\n";
				std::cerr << "\n";

				std::cerr << app.help();

				return -1;
			}
		}
	}
	catch (const CLI::ParseError& e)
	{
		int ret = app.exit(e);
		return ret == 0 ? 1 : ret;
	}
	catch (...)
	{
		std::cerr << "Unkown exception parsing arguments\n";
		return -1;
	}

	return 0;
}

const bool parse_config(const std::string& path)
{
	try
	{
		nlohmann::json json = nlohmann::json::parse(std::ifstream(path));
		nlohmann::json cam = json["configuration"]["camera"];
		nlohmann::json plc = json["configuration"]["plc"];

		if (cam.is_null())
			throw std::exception{"Missing camera configuration"};

		if (plc.is_null())
			throw std::exception{"Missing PLC configuration"};

		config.cam.ip = cam.value("ip", config.cam.ip);
		config.cam.port = cam.value("port", config.cam.port);

		config.plc.ip = plc.value("ip", config.plc.ip);
		config.plc.db_number = plc.value("db_number", config.plc.db_number);
		config.plc.db_size_bytes = plc.value("db_size_bytes", config.plc.db_size_bytes);
		config.plc.db_offset = plc.value("db_offset", config.plc.db_offset);
		config.plc.rack = plc.value("rack", config.plc.rack);
		config.plc.slot = plc.value("slot", config.plc.slot);
		
	}
	catch (const nlohmann::detail::exception& e)
	{
		std::cerr << "Exception while parsing configuration file: " << e.what() << "\n";
		return false;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception while parsing configuration file: " << e.what() << "\n";
		return false;
	}
	catch (...)
	{
		std::cerr << "Unknown exception while parsing configuration file\n";
		return false;
	}

	return true;
}

const bool parse_filters(const std::string& path)
{
	try
	{
		nlohmann::json filters = nlohmann::json::parse(std::ifstream(path));
		pipeline.load_json(filters);
	}
	catch (const nlohmann::detail::exception& e)
	{
		std::cerr << "Exception while parsing filter file: " << e.what() << "\n";
		return false;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception while parsing filter file: " << e.what() << "\n";
		return false;
	}
	catch (...)
	{
		std::cerr << "Unknown exception while parsing filter file\n";
		return false;
	}

	return true;
}

void signal_handler(int signum)
{
	if (signum == SIGINT)
	{
		spdlog::info("Quitting...");
		done = true;
	}
}
