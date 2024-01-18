#pragma once

#include <string>
#include <vector>

#include "3pp/snap7/snap7.h"

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
		const int write_udint(const std::vector<uint32_t>& data, const int db_number, const int db_offset_bytes);

	private:
		TS7Client plc;
	};
}
