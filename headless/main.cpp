#include <chrono>
#include <iostream>
#include <string>
#include <thread>	

#include <Framegrabber.h>
#include <VisionaryControl.h>
#include <VisionaryTMiniData.h>

#include <snap7.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

boost::property_tree::ptree g_config;
const std::string config_path = "configuration";

const std::string default_cam_ip = "127.0.0.1";
const std::string default_plc_ip = "127.0.0.1";

const void save_configuration(const std::string file, const boost::property_tree::ptree &config);
const bool load_configuration(const std::string file, boost::property_tree::ptree& config);

int main(int argc, char** argv)
{
	g_config.put("camera_ip_address", default_cam_ip);
	g_config.put("plc_ip_address", default_plc_ip);

#pragma region plc

	TS7Client plc;
	if (plc.ConnectTo("192.168.1.10", 0, 0) != 0)
	{
		std::cout << "Failed to connect plc\n";
		return -1;
	}

	std::vector<uint32_t> data;

	for (int i = 0; i < 1296; ++i)
	{
		data.push_back(i);
	}

	// prepare byte buffer
	std::vector<byte> buffer;
	buffer.resize(data.size() * sizeof(uint32_t));

	// fill byte buffer from uint32_t (UDint) buffer
	for (size_t i = 0; i < data.size(); ++i)
	{
		SetDWordAt(buffer.data(), i * sizeof(uint32_t), data[i]);
	}

	// write the buffer
	int ret = plc.DBWrite(2, 0, buffer.size(), buffer.data());
	if (ret != 0)
	{
		std::cout << "Failed to write to plc: " << CliErrorText(ret) << "\n";
	}

#pragma endregion


#pragma region camera
	
	const std::string cam_ip = "192.168.1.67";
	visionary::FrameGrabber<visionary::VisionaryTMiniData> grabber(cam_ip, htons(2114u), 5000);
	std::shared_ptr<visionary::VisionaryTMiniData> data_handler;
	visionary::VisionaryControl visionary_control;

	// open control channel
	if (!visionary_control.open(visionary::VisionaryControl::ProtocolType::COLA_2, cam_ip, 5000/*ms*/))
	{
		std::cout << "Failed to open control connection to camera.\n";
		return -1;
	}

	// read device id
	std::cout << "DeviceIdent: " << visionary_control.getDeviceIdent() << "\n";

	// logout
	if (!visionary_control.logout())
	{
		std::cout << "Failed to logout camera\n";
	}

	// logout should always work
	visionary_control.stopAcquisition();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// start continous acquisition
	if (!visionary_control.startAcquisition())
	{
		std::cout << "Failed to start acquisition\n";
		return -1;
	}

	while (true)
	{
		if (grabber.getNextFrame(data_handler))
		{
			//std::this_thread::sleep_for(std::chrono::milliseconds(10));
			std::cout << data_handler->getFrameNum() << "\n";

			std::vector<uint16_t> depth = data_handler->getDistanceMap();
			std::vector<uint32_t> data;

			for (int i = 0; i < 1296; ++i)
			{
				data.push_back(depth[i]);
			}

			// prepare byte buffer
			std::vector<byte> buffer;
			buffer.resize(data.size() * sizeof(uint32_t));

			// fill byte buffer from uint32_t (UDint) buffer
			for (size_t i = 0; i < data.size(); ++i)
			{
				SetDWordAt(buffer.data(), i * sizeof(uint32_t), data[i]);
			}

			// write the buffer
			int ret = plc.DBWrite(2, 0, buffer.size(), buffer.data());
			if (ret != 0)
			{
				std::cout << "Failed to write to plc: " << CliErrorText(ret) << "\n";
				plc.Disconnect();
				ret = plc.Connect();
				if (ret != 0)
				{
					std::cout << "Failed to reconnect PLC: " << CliErrorText(ret) << "\n";
				}
			}
		}
	}

	// close control
	visionary_control.close();

#pragma endregion

	return 0;
}

const void save_configuration(const std::string file, const boost::property_tree::ptree &config)
{
	try
	{
		boost::property_tree::ini_parser::write_ini(file, config);
	}
	catch (const boost::property_tree::ini_parser_error e)
	{
		std::cout << "Error saving configuration: " << e.what() << "\n";
	}
}

const bool load_configuration(const std::string file, boost::property_tree::ptree &config)
{
	try
	{
		boost::property_tree::ini_parser::read_ini(file, config);
	}
	catch (const boost::property_tree::ini_parser_error e)
	{
		std::cout << "Error reading configuration: " << e.what() << "\n";
	}
}
