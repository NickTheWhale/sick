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
#include <nlohmann/json-schema.hpp>

#include <headless/filter_pipeline.h>
#include <headless/camera_handler.h>
#include <headless/plc_handler.h>
#include <headless/frame.h>

#include <opencv2/core/utils/logger.hpp>

static const nlohmann::json config_schema = R"(
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "complete configuration",
  "type": "object",
  "properties": {
    "configuration": {
      "type": "object",
      "properties": {
        "camera": {
          "type": "object",
          "properties": {
            "ip": {
              "type": "string"
            },
            "port": {
              "type": "number"
            },
            "frame": {
              "type": "object",
              "properties": {
                "width": {
                  "type": "number"
                },
                "height": {
                  "type": "number"
                }
              },
              "required": [
                "width",
                "height"
              ]
            }
          },
          "required": [
            "ip",
            "port",
            "frame"
          ]
        },
        "plc": {
          "type": "object",
          "properties": {
            "ip": {
              "type": "string"
            },
            "rack": {
              "type": "number"
            },
            "slot": {
              "type": "number"
            },
            "db_number": {
              "type": "number"
            },
            "db_offset_bytes": {
              "type": "number"
            }
          },
          "required": [
            "ip",
            "rack",
            "slot",
            "db_number",
            "db_offset_bytes"
          ]
        }
      },
      "required": [
        "camera",
        "plc"
      ]
    }
  },
  "required": [
    "configuration"
  ]
}
)"_json;

volatile std::atomic_bool done = false;

void setup_loggers();
const bool parse_config(const std::string& path, nlohmann::json& config);
const bool parse_filters(const std::string& path, filter::filter_pipeline& pipeline);
void signal_handler(int signum);

int main(int argc, char** argv)
{
	setup_loggers();
	// register signal_handler as the ctrl+c callback
	signal(SIGINT, signal_handler);

	CLI::App app;

	std::string config_path = "";
	app.add_option("config", config_path, "Path to configuration file")->required(true);

	std::string filter_path = "";
	app.add_option("--filters", filter_path, "Path to filter file")->expected(1);

	CLI11_PARSE(app, argc, argv);

	nlohmann::json configuration_root;
	if (!parse_config(config_path, configuration_root))
	{
		return EXIT_FAILURE;
	}
	nlohmann::json config = configuration_root["configuration"];

	filter::filter_pipeline pipeline;
	if (app.count("--filters") > 0 && !parse_filters(filter_path, pipeline))
	{
		return EXIT_FAILURE;
	}

	spdlog::get("app")->info("Using configuration:\n{}", config.dump(2));
	spdlog::get("app")->info("Using filters:\n{}", pipeline.to_json().dump(2));

	const std::string& plc_ip = config["plc"]["ip"].get<std::string>();
	const int plc_rack = config["plc"]["rack"].get<int>();
	const int plc_slot = config["plc"]["slot"].get<int>();
	plc::plc_handler plc;
	while (!done && 0 != plc.connect_to(plc_ip, plc_rack, plc_slot))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	}

	const std::string& cam_ip = config["camera"]["ip"].get<std::string>();
	const uint16_t cam_port = config["camera"]["port"].get<uint16_t>();
	camera::camera_handler camera;
	while (!done && !camera.open(cam_ip, cam_port, 1000))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	}

	// loop indefinitely, filtering and sending frames to the plc
	const int db_number = config["plc"]["db_number"].get<int>();
	const int db_offset_bytes = config["plc"]["db_offset_bytes"].get<int>();
	const int frame_width = config["camera"]["frame"]["width"].get<int>();
	const int frame_height = config["camera"]["frame"]["height"].get<int>();
	while (!done)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		frame::Frame raw_frame;
		if (camera.get_next_frame(raw_frame))
		{
			cv::Mat raw_mat = frame::to_mat(raw_frame);
			if (pipeline.apply(raw_mat))
			{
				cv::Mat filtered_mat;
				cv::resize(raw_mat, filtered_mat, cv::Size(frame_width, frame_height), 0.0, 0.0, cv::InterpolationFlags::INTER_AREA);
				frame::Frame filtered_frame = frame::to_frame(filtered_mat);

				std::vector<uint16_t> distance_map_16 = filtered_frame.data;
				std::vector<uint32_t> distance_map(distance_map_16.begin(), distance_map_16.end());

				const int ret = plc.write_udint(distance_map, db_number, db_offset_bytes);
				if (ret != 0)
				{
					spdlog::error("Failed to write frame #{} to PLC: {}", filtered_frame.number, CliErrorText(ret));
					plc.disconnect();
					std::this_thread::sleep_for(std::chrono::milliseconds(5000));
					plc.connect();
				}
			}
			else
			{
				spdlog::get("filter")->error("Failed to apply filters on frame #{}", raw_frame.number);
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
		}
	}

	plc.disconnect();

	return EXIT_SUCCESS;
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
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);
}

const bool parse_config(const std::string& path, nlohmann::json& config)
{
	try
	{
		nlohmann::json json = nlohmann::json::parse(std::ifstream(path));		
		nlohmann::json_schema::json_validator validator;
		validator.set_root_schema(config_schema);

		validator.validate(json);

		config = json;
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

const bool parse_filters(const std::string& path, filter::filter_pipeline& pipeline)
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
