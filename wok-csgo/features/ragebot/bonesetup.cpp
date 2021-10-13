#include "../features.h"

bool c_bonesetup::setup_bones_rebuild(c_cs_player* player, int mask, matrix3x4_t* out) {
	auto backup_occlusion_flags = player->get_occlusion_flags();
	auto backup_occlusion_framecount = player->get_occlusion_frame_count();

	auto backup_abs_origin = player->get_abs_origin();

	const auto backup_curtime = interfaces::m_global_vars->m_cur_time;
	const auto backup_realtime = interfaces::m_global_vars->m_real_time;
	const auto backup_frametime = interfaces::m_global_vars->m_frame_time;

	const auto backup_abs_frametime = interfaces::m_global_vars->m_absolute_frame_time;

	const auto backup_tickcount = interfaces::m_global_vars->m_tick_count;
	const auto backup_framecount = interfaces::m_global_vars->m_frame_count;

	const auto backup_interpolation = interfaces::m_global_vars->m_interpolation_amount;

	interfaces::m_global_vars->m_cur_time = player->get_sim_time();
	interfaces::m_global_vars->m_frame_time = interfaces::m_global_vars->m_interval_per_tick;
	interfaces::m_global_vars->m_absolute_frame_time = interfaces::m_global_vars->m_interval_per_tick;

	interfaces::m_global_vars->m_real_time = player->get_sim_time();

	interfaces::m_global_vars->m_frame_count = TIME_TO_TICKS(player->get_sim_time());
	interfaces::m_global_vars->m_tick_count = TIME_TO_TICKS(player->get_sim_time());

	if (!globals::m_local)
		player->set_abs_origin(player->get_origin());

	player->get_effects().add(0x8);

	globals::m_setup_bones = true;

	player->invalidate_bone_cache();

	player->setup_bones(out, 128, mask, player->get_sim_time());

	interfaces::m_global_vars->m_cur_time = backup_curtime;
	interfaces::m_global_vars->m_frame_time = backup_frametime;
	interfaces::m_global_vars->m_absolute_frame_time = backup_abs_frametime;

	interfaces::m_global_vars->m_real_time = backup_realtime;

	interfaces::m_global_vars->m_frame_count = backup_framecount;
	interfaces::m_global_vars->m_tick_count = backup_tickcount;

	interfaces::m_global_vars->m_interpolation_amount = backup_interpolation;

	player->get_occlusion_flags() = backup_occlusion_flags;
	player->get_occlusion_frame_count() = backup_occlusion_framecount;

	player->get_abs_origin() = backup_abs_origin;

	player->get_effects().remove(0x8);

	globals::m_setup_bones = false;

	return true;
}