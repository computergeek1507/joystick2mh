#pragma once

#include <cstdint>
const int min_value{ 0 };
const int max_value{ 65535 };

struct MotorData {
	uint32_t channel_coarse{ 0u };
	uint32_t channel_fine{ 0u };
	int min_limit{ -180 };
	int max_limit{ 180 };
	double range_of_motion{ 180.0 };
	int orient_zero{ 0 };
	int orient_home{ 0 };
	bool reverse{ false };
	int ConvertPostoCmd(float position) const;
};