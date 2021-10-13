#include "../features.h"

std::deque<c_lag_record> m_player_records[65];

c_lag_record::c_lag_record() {
	reset();
}

void c_lag_record::reset() {
	m_player = nullptr;

	m_index = -1;

	m_invalid = false;
	is_immune = false;
	is_dormant = false;

	m_flags.clear();

	m_angles = qangle_t(0, 0, 0);
	m_abs_angles = qangle_t(0, 0, 0);

	m_velocity = vec3_t(0, 0, 0);
	m_origin = vec3_t(0, 0, 0);

	m_obb_min = vec3_t(0, 0, 0);
	m_obb_max = vec3_t(0, 0, 0);
}

c_lag_record::c_lag_record(c_cs_player* player) {
	m_invalid = false;

	store_record(player);
}

void c_lag_record::store_record(c_cs_player* player) {
	if (!player->is_alive())
		return;

	m_player = player;
	m_index = m_player->get_index();

	is_immune = m_player->is_immune();
	is_dormant = m_player->is_dormant();

	m_flags = m_player->get_flags();

	m_sim_time = m_player->get_sim_time();
	m_old_sim_time = m_player->get_old_sim_time();

	m_lby = m_player->get_lby();
	m_duck_amount = m_player->get_duck_amount();

	m_angles = m_player->get_eye_angles();
	m_abs_angles = m_player->get_abs_angles();

	m_velocity = m_player->get_velocity();
	m_origin = m_player->get_origin();

	m_obb_min = m_player->get_collideable()->obb_mins();
	m_obb_max = m_player->get_collideable()->obb_maxs();
}

void c_lag_record::apply_record() {
	if (!valid(false))
		return;

	m_player->get_flags() = m_flags;

	m_player->get_sim_time() = m_sim_time;
	m_player->get_duck_amount() = m_duck_amount;
	m_player->get_lby() = m_lby;

	m_player->get_velocity() = m_velocity;

	m_player->get_eye_angles() = m_angles;
	m_player->set_abs_angles(m_abs_angles);

	m_player->get_origin() = m_origin;
	m_player->set_abs_origin(m_origin);

	m_player->get_collideable()->obb_mins() = m_obb_min;
	m_player->get_collideable()->obb_maxs() = m_obb_max;
}

bool c_lag_record::valid(bool extra_checks) {
	if (!this)
		return false;

	if (m_index > 0)
		m_player = (c_cs_player*)interfaces::m_entity_list->get_client_entity(m_index);

	if (!m_player)
		return false;

	if (m_player->get_life_state() != LIFE_ALIVE)
		return false;

	if (is_dormant)
		return false;

	if (!extra_checks)
		return true;

	auto net_channel_info = interfaces::m_engine->get_net_channel_info();

	if (!net_channel_info)
		return false;

	auto correct = math::clamp(net_channel_info->get_latency(FLOW_INCOMING) + net_channel_info->get_latency(FLOW_OUTGOING) + globals::m_lerp, 0.0f, 0.2f);

	auto curtime = TICKS_TO_TIME(globals::m_local->get_tick_base());

	auto delta_time = correct - (curtime - m_sim_time);

	if (fabs(delta_time) > 0.2f)
		return false;

	return true;
}

void c_lag_comp::on_net_update(e_client_frame_stage stage) {
	if (stage != FRAME_NET_UPDATE_END)
		return;

	if (!cfg::get<bool>(FNV1A("ragebot_enable")))
		return;

	for (auto i = 1; i < interfaces::m_global_vars->m_max_clients; i++) {
		auto entity = static_cast<c_cs_player*>(interfaces::m_entity_list->get_client_entity(i));

		if (entity == static_cast<c_cs_player*>(globals::m_local))
			continue;

		if (!valid(i, entity))
			continue;

		if (entity && (m_player_records[i].empty() || entity->get_sim_time() != entity->get_old_sim_time())) {
			m_player_records[i].emplace_front(c_lag_record());

			update_animations(entity);
		}

		while (m_player_records[i].size() > 32)
			m_player_records[i].pop_back();
	}
}

bool c_lag_comp::valid(int i, c_cs_player* player) {
	if (!cfg::get<bool>(FNV1A("ragebot_enable"))) {
		m_player_records[i].clear();

		return false;
	}

	if (!player || !player->is_alive() || player == static_cast<c_cs_player*>(globals::m_local))
		return false;

	return true;
}

void c_lag_comp::update_animations(c_cs_player* player) {
	auto animstate = player->get_anim_state();

	if (!animstate)
		return;

	anim_layers_t anim_layers;
	memcpy(&anim_layers, &player->get_anim_layers(), sizeof(anim_layers_t));

	const auto backup_curtime = interfaces::m_global_vars->m_cur_time;
	const auto backup_realtime = interfaces::m_global_vars->m_real_time;
	const auto backup_frametime = interfaces::m_global_vars->m_frame_time;

	const auto backup_abs_frametime = interfaces::m_global_vars->m_absolute_frame_time;

	const auto backup_tickcount = interfaces::m_global_vars->m_tick_count;
	const auto backup_framecount = interfaces::m_global_vars->m_frame_count;

	const auto backup_interpolation = interfaces::m_global_vars->m_interpolation_amount;

	auto records = &m_player_records[player->get_index()];

	auto record = &records->front();
	c_lag_record* previous_record = nullptr;

	if (records->size() >= 2)
		previous_record = &records->at(1);

	interfaces::m_global_vars->m_cur_time = player->get_sim_time();
	interfaces::m_global_vars->m_frame_time = interfaces::m_global_vars->m_interval_per_tick;
	interfaces::m_global_vars->m_absolute_frame_time = interfaces::m_global_vars->m_interval_per_tick;

	interfaces::m_global_vars->m_real_time = player->get_sim_time();

	interfaces::m_global_vars->m_frame_count = TIME_TO_TICKS(player->get_sim_time());
	interfaces::m_global_vars->m_tick_count = TIME_TO_TICKS(player->get_sim_time());

	interfaces::m_global_vars->m_interpolation_amount = 0.0f;

	if (previous_record) {
		auto time_diff = player->get_sim_time() - previous_record->m_sim_time;

		player->get_velocity() = (player->get_origin() - previous_record->m_origin) * (1.f / TIME_TO_TICKS(time_diff));

		if (!(player->get_flags().has(FL_ONGROUND))) {
			static auto sv_gravity = interfaces::m_cvar_system->find_var(FNV1A("sv_gravity"));

			auto fixed_timing = math::clamp(time_diff, interfaces::m_global_vars->m_interval_per_tick, 1.f);
			player->get_velocity().z -= sv_gravity->get_float() * fixed_timing * 0.5f;
		}
		else
			player->get_velocity().z = 0.f;
	}

	player->get_eflags().remove(0x1000);

	if (player->get_flags().has(FL_ONGROUND) && player->get_velocity().length() > 0.0f && anim_layers[6].m_weight <= 0.0f)
		player->get_velocity() = vec3_t(0.f, 0.f, 0.f);

	player->get_abs_velocity() = player->get_velocity();

	auto updated_anims = false;

	if (previous_record) {
		auto sim_ticks = TIME_TO_TICKS(player->get_sim_time() - previous_record->m_sim_time);

		sim_ticks = math::clamp(sim_ticks, 0, 17);

		auto land_time = 0.0f;
		auto land_in_cycle = false;
		auto is_landed = false;
		auto on_ground = false;

		if (anim_layers[4].m_cycle < 0.5f && (!(player->get_flags().has(FL_ONGROUND)) || !(previous_record->m_flags.has(FL_ONGROUND)))) {
			land_time = player->get_sim_time() - anim_layers[4].m_playback_rate * anim_layers[4].m_cycle;
			land_in_cycle = land_time >= previous_record->m_sim_time;
		}

		for (auto i = 1; i < sim_ticks; i++) {
			auto sim_time = previous_record->m_sim_time + TICKS_TO_TIME(i);

			auto duck_amount_per_tick = (player->get_duck_amount() - previous_record->m_duck_amount) / sim_ticks;

			if (duck_amount_per_tick)
				player->get_duck_amount() = previous_record->m_duck_amount + duck_amount_per_tick * (float)i;

			on_ground = player->get_flags().has(FL_ONGROUND);

			if (land_in_cycle && !is_landed) {
				if (land_time <= sim_time) {
					is_landed = true;
					on_ground = true;
				}
				else
					on_ground = previous_record->m_flags.has(FL_ONGROUND);
			}

			if (on_ground)
				player->get_flags().add(FL_ONGROUND);
			else
				player->get_flags().remove(FL_ONGROUND);

			interfaces::m_global_vars->m_cur_time = sim_time;
			interfaces::m_global_vars->m_frame_time = interfaces::m_global_vars->m_interval_per_tick;
			interfaces::m_global_vars->m_absolute_frame_time = interfaces::m_global_vars->m_interval_per_tick;

			interfaces::m_global_vars->m_real_time = sim_time;

			interfaces::m_global_vars->m_frame_count = TIME_TO_TICKS(sim_time);
			interfaces::m_global_vars->m_tick_count = TIME_TO_TICKS(sim_time);

			interfaces::m_global_vars->m_interpolation_amount = 0.0f;

			player->get_client_side_animation() = true;
			player->update_client_side_animation();
			player->get_client_side_animation() = false;

			interfaces::m_global_vars->m_cur_time = backup_curtime;
			interfaces::m_global_vars->m_frame_time = backup_frametime;
			interfaces::m_global_vars->m_absolute_frame_time = backup_abs_frametime;

			interfaces::m_global_vars->m_real_time = backup_realtime;

			interfaces::m_global_vars->m_frame_count = backup_framecount;
			interfaces::m_global_vars->m_tick_count = backup_tickcount;

			interfaces::m_global_vars->m_interpolation_amount = backup_interpolation;

			updated_anims = true;
		}
	}

	if (!updated_anims) {
		player->get_client_side_animation() = true;
		player->update_client_side_animation();
		player->get_client_side_animation() = false;
	}

	interfaces::m_global_vars->m_cur_time = backup_curtime;
	interfaces::m_global_vars->m_frame_time = backup_frametime;
	interfaces::m_global_vars->m_absolute_frame_time = backup_abs_frametime;

	interfaces::m_global_vars->m_real_time = backup_realtime;

	interfaces::m_global_vars->m_frame_count = backup_framecount;
	interfaces::m_global_vars->m_tick_count = backup_tickcount;

	interfaces::m_global_vars->m_interpolation_amount = backup_interpolation;

	record->store_record(player);
}
