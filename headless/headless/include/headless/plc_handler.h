#pragma once

#include <string>
#include <vector>

#include <snap7.h>

namespace plc
{
	class plc_handler
	{
	public:
		plc_handler();
		~plc_handler();

		const int connect_to(const std::string& ip, const int rack, const int slot);
		const int connect();
		const int disconnect();
		const int write_udint(const std::vector<uint32_t>& data, const int db_number, const int db_offset);

	private:
		TS7Client plc;
	};
}
