#include "../features.h"

void c_movement::on_create_move(bool a1) {
	if (!globals::m_local->is_alive())
		return;

	if (a1) 
		return compute_buttons();
	


	bunny_hop();

	compute_buttons();
}

void c_movement::bunny_hop() {
	if (!cfg::get<bool>(FNV1A("bunnyhop")))
		return;

	if (!globals::m_local->is_alive())
		return;

	if (globals::m_local->get_flags().has(FL_IN_WATER))
		return;

	static bool jumped_last_tick = false;
	static bool should_fake_jump = true;

	if (!jumped_last_tick && should_fake_jump) {
		should_fake_jump = false;

		globals::m_cur_cmd->m_buttons.add(IN_JUMP);
	}
	else if (globals::m_cur_cmd->m_buttons.has(IN_JUMP)) {
		if (globals::m_local->get_flags().has(FL_ONGROUND)) {
			jumped_last_tick = true;
			should_fake_jump = true;
		}
		else {
			globals::m_cur_cmd->m_buttons.remove(IN_JUMP);
			jumped_last_tick = false;
		}
	}
	else {
		jumped_last_tick = false;
		should_fake_jump = false;
	}
}

void c_movement::auto_strafer(qangle_t& angle) {
	if (!cfg::get<bool>(FNV1A("directional_strafer")))
		return;

	if (globals::m_cur_cmd->m_buttons.has(IN_SPEED) && globals::m_local->get_velocity().length_2d() < 1.f)
		return;

	static float yaw = 0.f;

	static auto cl_sidespeed = interfaces::m_cvar_system->find_var(FNV1A("cl_sidespeed"));

	if (!globals::m_local->get_flags().has(FL_ONGROUND)) {
		bool back = globals::m_cur_cmd->m_buttons.has(IN_BACK);
		bool forward = globals::m_cur_cmd->m_buttons.has(IN_FORWARD);
		bool right = globals::m_cur_cmd->m_buttons.has(IN_MOVELEFT);
		bool left = globals::m_cur_cmd->m_buttons.has(IN_MOVERIGHT);

		if (back) {
			yaw = -180.f;
			if (right)
				yaw -= 45.f;
			else if (left)
				yaw += 45.f;
		}
		else if (right) {
			yaw = 90.f;
			if (back)
				yaw += 45.f;
			else if (forward)
				yaw -= 45.f;
		}
		else if (left) {
			yaw = -90.f;
			if (back)
				yaw -= 45.f;
			else if (forward)
				yaw += 45.f;
		}
		else 
			yaw = 0.f;

		angle.y += yaw;

		globals::m_cur_cmd->m_move.x = 0.f;
		globals::m_cur_cmd->m_move.y = 0.f;

		const auto delta = math::normalize_yaw(angle.y - math::rad_to_deg(math::atan2(globals::m_local->get_velocity().y, globals::m_local->get_velocity().x)));

		globals::m_cur_cmd->m_move.y = delta > 0.f ? -cl_sidespeed->get_float() : cl_sidespeed->get_float();

		angle.y = math::normalize_yaw(angle.y - delta);
	}
}

void c_movement::compute_buttons() {
	globals::m_cur_cmd->m_buttons.remove(IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT);

	if (globals::m_local->get_move_type() == MOVE_TYPE_LADDER) {
		if (std::fabsf(globals::m_cur_cmd->m_move.x) > 200.f) {
			globals::m_cur_cmd->m_buttons.add(globals::m_cur_cmd->m_move.x > 0.f ? IN_FORWARD : IN_BACK);
		}

		if (std::fabsf(globals::m_cur_cmd->m_move.y) > 200.f) {
			globals::m_cur_cmd->m_buttons.add(globals::m_cur_cmd->m_move.y > 0.f ? IN_MOVERIGHT : IN_MOVELEFT);
		}

		return;
	}

	if (globals::m_cur_cmd->m_move.x != 0.f) {
		globals::m_cur_cmd->m_buttons.add(globals::m_cur_cmd->m_move.x > 0.f ? IN_FORWARD : IN_BACK);
	}

	if (globals::m_cur_cmd->m_move.y != 0.f) {
		globals::m_cur_cmd->m_buttons.add(globals::m_cur_cmd->m_move.y > 0.f ? IN_MOVERIGHT : IN_MOVELEFT);
	}
}