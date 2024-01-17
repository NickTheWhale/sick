#include <headless/plc_handler.h>

#include <spdlog/spdlog.h>

plc::plc_handler::plc_handler()
{
}

plc::plc_handler::~plc_handler()
{
	plc.Disconnect();
}

const int plc::plc_handler::connect_to(const std::string& ip, const int rack, const int slot)
{
	const int ret = plc.ConnectTo(ip.c_str(), rack, slot);
	if (ret != 0)
		spdlog::get("plc")->error("Failed to connect to PLC with address '{}', rack '{}', and slot '{}': {}", ip, rack, slot, CliErrorText(ret));
	else
		spdlog::get("plc")->info("Connected to PLC");

	return ret;
}

const int plc::plc_handler::connect()
{
	const int ret = plc.Connect();
	if (ret != 0)
		spdlog::get("plc")->error("Failed to connect to PLC: {}", CliErrorText(ret));
	else
		spdlog::get("plc")->info("Connected to PLC");

	return ret;
}

const int plc::plc_handler::disconnect()
{
	const int ret = plc.Disconnect();
	if (ret != 0)
		spdlog::get("plc")->error("Failed to disconnect PLC: {}", CliErrorText(ret));
	else
		spdlog::get("plc")->info("Disconnected from PLC");

	return ret;
}

const int plc::plc_handler::write_udint(const std::vector<uint32_t>& data, const int db_number, const int db_offset)
{
	int ret;
	std::vector<byte> buffer;
	buffer.resize(data.size() * sizeof(uint32_t));

	for (int i = 0; i < data.size(); ++i)
	{
		SetDWordAt(buffer.data(), i * static_cast<int>(sizeof(uint32_t)), data[i]);
	}

	ret = plc.DBWrite(db_number, db_offset, static_cast<int>(buffer.size()), static_cast<void *>(buffer.data()));
	if (ret != 0)
	{
		spdlog::get("plc")->error("Failed to write to PLC: {}", CliErrorText(ret));
	}

	return ret;
}
