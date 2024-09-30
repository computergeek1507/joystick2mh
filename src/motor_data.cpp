#include "motor_data.h"

int MotorData::ConvertPostoCmd(float position) const
{
	/*

	int channel_coarse{ 0 };
	int channel_fine{ 0 };
	int min_value{ 0 };
	int max_value{ 65535 };
	int min_limit{ -180 };
	int max_limit{ 180 };
	float range_of_motion{180.0f};
	int orient_zero { 0 };
	int orient_home{ 0 };
	float slew_limit{ 0.0F };
	bool reverse{false};
	bool upside_down{false};
	int rev{ 1 };
	*/
	int rev{ 1 };
	if (reverse) {
		rev = -1;
	}
	else {
		rev = 1;
	}
	float limited_pos = position;
	if (limited_pos > max_limit) {
		limited_pos = max_limit;
	}
	else if (limited_pos < min_limit) {
		limited_pos = min_limit;
	}

	//if (upside_down) {
	//	limited_pos = -1.0f * limited_pos;
	//}

	float goto_home = (float)max_value * (float)orient_home / range_of_motion;
	float amount_to_move = (float)max_value * limited_pos / range_of_motion * rev;
	float cmd = goto_home + amount_to_move;
	float full_spin = (float)max_value * 360.0 / range_of_motion;

	if (cmd < 0) {
		if (cmd + full_spin < max_value) {
			cmd += full_spin;
		}
		else {
			cmd = 0; // tbd....figure out which limit is closer to desired target
		}
	}
	else if (cmd > max_value) {
		if (cmd - full_spin >= 0.0f) {
			cmd -= full_spin;
		}
		else {
			cmd = max_value; // tbd....figure out which limit is closer to desired target
		}
	}
	return cmd;
}