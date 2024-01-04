#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>

#include <CLI11.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <Framegrabber.h>
#include <VisionaryControl.h>
#include <VisionaryTMiniData.h>

#include <snap7.h>

#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include <json.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/types_c.h>

#include <headless/filter_pipeline.h>

namespace v = visionary;

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
		int db_num = 0;
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
			<< "\tdb_num:\t\t" << plc.db_num << "\n"
			<< "\tdb_offset:\t" << plc.db_offset << "\n";
	}
};

configuration config;
filter_pipeline pipeline;

const int parse_args(int argc, char** argv);	
const bool parse_config(const std::string& file);
const bool parse_filters(const std::string& file);

int main(int argc, char** argv)
{
	spdlog::set_level(spdlog::level::trace);

	int ret;
	ret = parse_args(argc, argv);
	if (ret != 0)
	{
		return ret;
	}

	// connect to plc
	TS7Client plc;
	spdlog::trace("Connecting to PLC...");
	ret = plc.ConnectTo(config.plc.ip.c_str(), config.plc.rack, config.plc.slot);
	if (ret != 0)
	{
		spdlog::critical("Failed to connect to PLC at {}: {}", config.plc.ip, CliErrorText(ret));
		return -1;
	}
	spdlog::info("Connected to PLC at {}", config.plc.ip);

	// connect to camera
	spdlog::trace("Starting camera frame grabber...");
	v::FrameGrabber<v::VisionaryTMiniData> grabber(config.cam.ip, htons(config.cam.port), 5000);
	std::shared_ptr<v::VisionaryTMiniData> data_handler;
	v::VisionaryControl visionary_control;
	spdlog::trace("Frame grabber started");

	spdlog::trace("Opening camera control channel...");
	if (!visionary_control.open(v::VisionaryControl::ProtocolType::COLA_2, config.cam.ip, 5000))
	{
		spdlog::critical("Failed to connect to camera at {}", config.cam.ip);
		return -1;
	}
	spdlog::info("Connected to camera at {}", config.cam.ip);

	spdlog::trace("Stopping acquisition...");
	visionary_control.stopAcquisition();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	spdlog::trace("Starting camera frame acquisition...");
	if (!visionary_control.startAcquisition())
	{
		spdlog::critical("Failed to start camera acquisition");
		return -1;
	}
	spdlog::trace("Camera frame acquisition started");

	// loop indefinitely, filtering and sending frames to the plc
	bool done = false;
	while (!done)
	{
		if (grabber.getNextFrame(data_handler))
		{
			std::vector<uint16_t> distance_map_16 = data_handler->getDistanceMap();
			std::vector<uint32_t> distance_map(distance_map_16.begin(), distance_map_16.end());

			std::vector<byte> buffer;
			buffer.resize(distance_map.size() * sizeof(uint32_t));

			for (size_t i = 0; i < distance_map.size(); ++i)
			{
				SetDWordAt(buffer.data(), i * sizeof(uint32_t), distance_map[i]);
			}

			ret = plc.DBWrite(config.plc.db_num, config.plc.db_offset, (int)1296*4, buffer.data());
			if (ret != 0)
			{
				spdlog::error("Failed to write frame #{} to PLC: {}", data_handler->getFrameNum(), CliErrorText(ret));
				plc.Disconnect();
				ret = plc.Connect();
				if (ret != 0)
				{
					spdlog::error("Failed to reconnect PLC: {}", CliErrorText(ret));
				}
			}
		}
	}

	visionary_control.close();
	plc.Disconnect();

	return 0;
}

const int parse_args(int argc, char** argv)
{
	CLI::App app;

	std::vector<CLI::Option*> req_with_no_config;

	// configuration file
	app.set_config("--config, config", "", "Path to configuration file")->each([](const std::string& file) { parse_config(file); });

	// camera options
	req_with_no_config.push_back(app.add_option("--cam-ip", config.cam.ip, "Camera IP address"));
	app.add_option("--cam-port", config.cam.port, "Camera control port");

	// plc options
	req_with_no_config.push_back(app.add_option("--plc-ip", config.plc.ip, "PLC IP address"));
	app.add_option("--plc-db", config.plc.db_num, "PLC DB number");
	app.add_option("--plc-db-offset", config.plc.db_offset, "PLC DB start offset");
	app.add_option("--plc-rack", config.plc.rack, "PLC rack number");
	app.add_option("--plc-slot", config.plc.slot, "PLC slot number");

	// filtering options
	app.add_option("--filters", "Path to filter file")->each([](const std::string& file) { parse_filters(file); })->expected(1);

	try
	{
		app.parse(argc, argv);

		if (app.count("config") < 1)
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

const bool parse_config(const std::string& file)
{
	try
	{
		nlohmann::json json = nlohmann::json::parse(std::ifstream(file));
		nlohmann::json cam = json["configuration"]["camera"];
		nlohmann::json plc = json["configuration"]["plc"];

		config.cam.ip = cam["ip"].is_null() ? config.cam.ip : cam["ip"].get<std::string>();
		config.cam.port = cam["port"].is_null() ? config.cam.port : cam["port"].get<uint16_t>();

		config.plc.ip = plc["ip"].is_null() ? config.plc.ip : plc["ip"].get<std::string>();
		config.plc.db_num = plc["db_number"].is_null() ? config.plc.db_num : plc["db_number"].get<int>();
		config.plc.db_offset = plc["db_offset"].is_null() ? config.plc.db_offset : plc["db_offset"].get<int>();
		config.plc.rack = plc["rack"].is_null() ? config.plc.rack : plc["rack"].get<int>();
		config.plc.slot = plc["slot"].is_null() ? config.plc.slot : plc["slot"].get<int>();
		
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

const bool parse_filters(const std::string& file)
{
	try
	{
		nlohmann::json filters = nlohmann::json::parse(std::ifstream(file));
		pipeline.from_json(filters);
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
