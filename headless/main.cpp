#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <argparse/argparse.hpp>

#include <spdlog/spdlog.h>

#include <Framegrabber.h>
#include <VisionaryControl.h>
#include <VisionaryTMiniData.h>

#include <snap7.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

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
};

configuration config;

const bool parse_args(int argc, char** argv);
const bool parse_file(const char* file);
const bool save_config(const char* file);

int main(int argc, char** argv)
{
	spdlog::set_level(spdlog::level::trace);

	// assume second argument is a file path
	if (argc == 2)
	{
		if (!parse_file(argv[1]))
		{
			return -1;
		}
	}

	// if we don't have 2 arguments, parse options
	else
	{
		if (!parse_args(argc, argv))
		{
			return -1;
		}
	}

	int ret;

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
		if (std::cin.peek() == 'q')
		{
			done = true;
			spdlog::info("Stopping...");
		}

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

			ret = plc.DBWrite(config.plc.db_num, config.plc.db_offset, std::min(distance_map.size(), (size_t)1296), buffer.data());
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

const bool parse_args(int argc, char** argv)
{
	argparse::ArgumentParser program("Program description.");

	program.add_argument("--camera-ip")
		.required()
		.help("IP address of the camera");

	program.add_argument("--plc-ip")
		.required()
		.help("IP address of the PLC");

	program.add_argument("-cp", "--camera-port")
		.default_value(uint16_t(2114))
		.action([](const std::string& value) { return static_cast<uint16_t>(std::stoi(value)); })
		.help("Camera port number");

	program.add_argument("-r", "--plc-rack")
		.default_value(0)
		.action([](const std::string& value) { return std::stoi(value); })
		.help("PLC rack number");

	program.add_argument("-s", "--plc-slot")
		.default_value(0)
		.action([](const std::string& value) { return std::stoi(value); })
		.help("PLC slot number");

	program.add_argument("-db", "--plc-db-number")
		.default_value(0)
		.action([](const std::string& value) { return std::stoi(value); })
		.help("PLC DB number");

	program.add_argument("-do", "--plc-db-offset")
		.default_value(0)
		.action([](const std::string& value) { return std::stoi(value); })
		.help("PLC DB starting offset");

	int verbosity = 0;
	program.add_argument("-V")
		.action([&](const auto&) { ++verbosity; })
		.append()
		.default_value(false)
		.implicit_value(true)
		.nargs(0)
		.help("Logging verbosity (-V: info, -VV: debug, -VVV: trace) [default: critical]");

	try
	{
		program.parse_args(argc, argv);

		config.cam.ip = program.get<std::string>("--camera-ip");
		config.cam.port = program.get<uint16_t>("--camera-port");
		config.plc.ip = program.get<std::string>("--plc-ip");
		config.plc.rack = program.get<int>("--plc-rack");
		config.plc.slot = program.get<int>("--plc-slot");
		config.plc.db_num = program.get<int>("--plc-db-number");
		config.plc.db_offset = program.get<int>("--plc-db-offset");

		if (verbosity == 0)
			spdlog::set_level(spdlog::level::critical);
		else if (verbosity == 1)
			spdlog::set_level(spdlog::level::info);
		else if (verbosity == 2)
			spdlog::set_level(spdlog::level::debug);
		else if (verbosity > 2)
			spdlog::set_level(spdlog::level::trace);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << "\n\n";
		std::cout << program;
		return false;
	}

	return true;
}

const bool parse_file(const char* file)
{
	using namespace boost::property_tree;
	try
	{
		ptree pt;

		xml_parser::read_xml(file, pt);

		config.cam.ip = pt.get<std::string>("config.camera.ip");
		config.cam.port = pt.get<uint16_t>("config.camera.port", 2114);
		config.plc.ip = pt.get<std::string>("config.plc.ip");
		config.plc.db_num = pt.get<int>("config.plc.db_number", 0);
		config.plc.db_offset = pt.get<int>("config.plc.db_offset", 0);
		config.plc.rack = pt.get<int>("config.plc.rack", 0);
		config.plc.slot = pt.get<int>("config.plc.slot", 0);
	}
	catch (const std::exception& e)
	{
		spdlog::critical("Failed to parse configuration file: {}", e.what());
		return false;
	}
	catch (...)
	{
		spdlog::critical("Failed to parse configuration file");
		return false;
	}

	return true;
}

const bool save_config(const char* file) 
{
	using namespace boost::property_tree;
	try 
	{
		ptree pt;

		pt.put("config.camera.ip", config.cam.ip);
		pt.put("config.camera.port", config.cam.port);
		pt.put("config.plc.ip", config.plc.ip);
		pt.put("config.plc.db_number", config.plc.db_num);
		pt.put("config.plc.db_offset", config.plc.db_offset);
		pt.put("config.plc.rack", config.plc.rack);
		pt.put("config.plc.slot", config.plc.slot);

		write_xml(file, pt);
	}
	catch (const std::exception& e) 
	{
		spdlog::critical("Failed to save configuration file: {}", e.what());
		return false;
	}
	catch (...) 
	{
		spdlog::critical("Failed to save configuration file");
		return false;
	}

	return true;
}