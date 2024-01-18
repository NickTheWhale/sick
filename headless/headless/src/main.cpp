#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>
#include <thread>
#include <string>

#include "CLI11.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/daily_file_sink.h"

#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include "json.hpp"
#include "nlohmann/json-schema.hpp"

#include "common/filter_pipeline.h"
#include "common/camera_handler.h"
#include "common/plc_handler.h"
#include "common/frame.h"

#include "opencv2/core/utils/logger.hpp"

/**
 * @brief JSON Schema used to validate configuration file. Generated using https://transform.tools/json-to-json-schema.
 */
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
const bool parse_and_validate_config(const std::string& path, nlohmann::json& config);
const bool parse_filters(const std::string& path, filter::filter_pipeline& pipeline);
void signal_handler(int signum);

int main(int argc, char** argv)
{
	setup_loggers();
	// register 'signal_handler' to catch ctrl+c signal for shutdown
	signal(SIGINT, signal_handler);

	// set program argument options
	CLI::App app;

	std::string config_path = "";
	app.add_option("config", config_path, "Path to configuration file")->required(true);

	std::string filter_path = "";
	app.add_option("--filters", filter_path, "Path to filter file")->expected(1);

	// parse options
	CLI11_PARSE(app, argc, argv);

	// check if a valid configuration file was given
	nlohmann::json configuration_root;
	if (!parse_and_validate_config(config_path, configuration_root))
	{
		return EXIT_FAILURE;
	}
	nlohmann::json config = configuration_root["configuration"];

	// if a filter file was given, check if its valid
	filter::filter_pipeline pipeline;
	if (app.count("--filters") > 0 && !parse_filters(filter_path, pipeline))
	{
		return EXIT_FAILURE;
	}

	// log parsed configuration and filters
	spdlog::get("app")->info("Using configuration:\n{}", config.dump(2));
	spdlog::get("app")->info("Using filters:\n{}", pipeline.to_json().dump(2));

	// connect to plc. if unsuccessful, keep trying with a 5 second timeout
	const std::string& plc_ip = config["plc"]["ip"].get<std::string>();
	const int plc_rack = config["plc"]["rack"].get<int>();
	const int plc_slot = config["plc"]["slot"].get<int>();
	plc::plc_handler plc;
	while (!done && 0 != plc.connect_to(plc_ip, plc_rack, plc_slot))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	}
	
	// connect to camera. if unsuccessful, keep trying with a 5 second timeout
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
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			frame::Frame raw_frame;
			// get the next frame (blocking with timeout)
			if (camera.get_next_frame(raw_frame, 5000/*ms*/))
			{
				cv::Mat raw_mat = frame::to_mat(raw_frame);
				// apply filters
				if (pipeline.apply(raw_mat))
				{
					cv::Mat filtered_mat;
					// resize to desired frame dimensions from configuration file
					cv::resize(raw_mat, filtered_mat, cv::Size(frame_width, frame_height), 0.0, 0.0, cv::InterpolationFlags::INTER_AREA);
					const frame::Frame filtered_frame = frame::to_frame(filtered_mat);

					const std::vector<uint16_t> distance_map_16 = filtered_frame.data;
					// convert to uint32_t (UDint in TIA Portal world)
					const std::vector<uint32_t> distance_map(distance_map_16.begin(), distance_map_16.end());

					// write frame to plc
					const int ret = plc.write_udint(distance_map, db_number, db_offset_bytes);
					// if writing fails, assume the plc connection was lost and try to reconnect
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
		catch (const spdlog::spdlog_ex& e)
		{
			std::cerr << "Logging exception: " << e.what() << ". Ignoring\n";
		}
		catch (const std::exception& e)
		{
			spdlog::error("Exception in main loop: {}", e.what());
			return EXIT_FAILURE;
		}
		catch (...)
		{
			spdlog::error("Unkown exception in main loop");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

/**
 * @brief Creates logger objects and sets logging levels.
 * 
 */
void setup_loggers()
{
	std::vector<spdlog::sink_ptr> sinks;
	sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink_mt>("logs/daily_logger", 2, 30));

	static constexpr char const* log_names[] = {
		"camera",
		"plc",
		"sickapi",
		"filter",
		"app"
	};

	for (const char* log_name : log_names)
	{
		spdlog::register_logger(std::make_shared<spdlog::logger>((log_name), sinks.begin(), sinks.end()));
	}

	spdlog::set_default_logger(spdlog::get(log_names[0]));
	spdlog::set_level(spdlog::level::trace);
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);
}

/**
 * @brief Reads JSON configuration from file and validates against a schema.
 * 
 * @param path Path to JSON configuration file
 * @param config Output configuration JSON representation
 * @return True if parsed & validated, false otherwise
 */
const bool parse_and_validate_config(const std::string& path, nlohmann::json& config)
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

/**
 * @brief Reads JSON filter file.
 * 
 * @param path Path to JSON file containing filter definitions
 * @param pipeline Output filter pipeline constructed from parsed filters
 * @return True if successful, false otherwise
 */
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

/**
 * @brief Handles ctrl+c and other signals.
 * 
 * Logs shutdown message and sets 'done' flag to true
 * 
 * @param signum Signal number
 */
void signal_handler(int signum)
{
	if (signum == SIGINT)
	{
		spdlog::info("Quitting...");
		done = true;
	}
}
