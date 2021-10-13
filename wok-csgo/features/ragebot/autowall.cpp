#include "../features.h"

void c_autowall::scale_damage(c_game_trace* trace, c_cs_weapon_data* data, int& hitgroup, float& damage) {
	auto weapon = globals::m_local->get_active_weapon();

	auto target = static_cast<c_cs_player*>(trace->m_hit_entity);

	if (!target)
		return;

	if (!globals::m_local || !globals::m_local->is_alive())
		return;

	bool has_heavy = false;
	int armor_value = target->get_armor_value();
	int hit_group = trace->m_hitgroup;

	auto is_armored = [&trace]() -> bool {
		switch (trace->m_hitgroup) {
		case HITGROUP_HEAD:
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFT_ARM:
		case HITGROUP_RIGHT_ARM:
			return true;
		default:
			return false;
		}
	};

	switch (hit_group) {
	case HITGROUP_HEAD:
		damage *= 2.f;
		break;
	case HITGROUP_CHEST:
	case HITGROUP_LEFT_ARM:
	case HITGROUP_RIGHT_ARM:
		damage *= 1.f;
		break;
	case HITGROUP_STOMACH:
		damage *= 1.25f;
		break;
	case HITGROUP_LEFT_LEG:
	case HITGROUP_RIGHT_LEG:
		damage *= 0.75f;
		break;
	default:
		break;
	}

	if (armor_value > 0 && is_armored()) {
		float bonus_value = 1.f, armor_bonus_ratio = 0.5f, armor_ratio = data->m_armor_ratio / 2.f;

		if (has_heavy)
		{
			armor_bonus_ratio = 0.33f;
			armor_ratio *= 0.5f;
			bonus_value = 0.33f;
		}

		auto new_damage = damage * armor_ratio;

		if (((damage - (damage * armor_ratio)) * (bonus_value * armor_bonus_ratio)) > armor_value)
		{
			new_damage = damage - (armor_value / armor_bonus_ratio);
		}

		damage = new_damage;
	}
}

void c_autowall::clip_trace_to_player(vec3_t start, vec3_t end, c_cs_player* player, int mask, i_trace_filter* filter, c_game_trace* trace) {
	if (!player)
		return;

	auto mins = player->get_collideable()->obb_mins();
	auto maxs = player->get_collideable()->obb_maxs();

	vec3_t dir(end - start);
	vec3_t center = (maxs + mins) / 2, pos(center + player->get_abs_origin());

	auto to = pos - start;
	auto range_along = dir.dot_product(to);

	float range;

	if (range_along < 0.f)
		range = -to.length();
	else if (range_along > dir.length())
		range = -(pos - end).length();
	else {
		auto ray(pos - (dir * range_along + start));
		range = ray.length();
	}

	if (range <= 60.f) {
		c_game_trace new_trace;

		ray_t ray(start, end);

		interfaces::m_trace_system->clip_ray_to_collideable(ray, mask, player->get_collideable(), &new_trace);

		if (trace->m_fraction > new_trace.m_fraction)
			*trace = new_trace;
	}
}

bool c_autowall::handle_bullet_penetration(c_cs_weapon_data* data, autowall_bullet_t& bullet) {
	c_game_trace trace;

	auto surface_data = interfaces::m_surface_data->get_surface_data(bullet.m_trace.m_surface.m_surface_props);
	auto material = surface_data->m_game.m_material;

	auto surface_penetration_modifier = surface_data->m_game.m_penetration_modifier;
	auto surface_damage_modifier = surface_data->m_game.m_damage_modifier;

	auto penetration_modifier = 0.f;

	auto solid_surf = (bullet.m_trace.m_contents >> 3) & CONTENTS_SOLID;
	auto light_surf = (bullet.m_trace.m_surface.m_flags >> 7) & SURF_LIGHT;

	if (bullet.m_walls_count <= 0 || (!bullet.m_walls_count && !light_surf && !solid_surf && material != CHAR_TEX_GLASS && material != CHAR_TEX_GRATE) || data->m_penetration <= 0.f || (!trace_to_exit(&bullet.m_trace, bullet.m_trace.m_end_pos, bullet.m_dir, &trace) && !(interfaces::m_trace_system->get_point_contents(bullet.m_trace.m_end_pos, MASK_SHOT_HULL | CONTENTS_HITBOX, NULL) & (MASK_SHOT_HULL | CONTENTS_HITBOX))))
		return false;

	auto prop_surface_data = interfaces::m_surface_data->get_surface_data(trace.m_surface.m_surface_props);
	auto prop_material = prop_surface_data->m_game.m_material;
	auto prop_penetration_modifier = prop_surface_data->m_game.m_penetration_modifier;

	if (material == CHAR_TEX_GRATE || material == CHAR_TEX_GLASS) {
		penetration_modifier = 3.f;
		surface_damage_modifier = 0.05f;
	}
	else if (light_surf || solid_surf) {
		penetration_modifier = 1.f;
		surface_damage_modifier = 0.16f;
	}
	else if (material == CHAR_TEX_FLESH) {
		penetration_modifier = 1.f;
		surface_damage_modifier = 0.16f;
	}
	else {
		penetration_modifier = (surface_penetration_modifier + prop_penetration_modifier) / 2.f;
		surface_damage_modifier = 0.16f;
	}

	if (material == prop_material) {
		if (prop_material == CHAR_TEX_CARDBOARD || prop_material == CHAR_TEX_WOOD)
			penetration_modifier = 3.f;
		else if (prop_material == CHAR_TEX_PLASTIC)
			penetration_modifier = 2.f;
	}

	auto thickness = (trace.m_end_pos - bullet.m_trace.m_end_pos).length_sqr();

	auto modifier = std::max(0.f, 1.f / penetration_modifier);

	auto lost_damage = fmax(((modifier * thickness) / 24.f) + ((bullet.m_damage * surface_damage_modifier) + (fmax(3.75f / data->m_penetration, 0.f) * 3.f * modifier)), 0.f);

	if (lost_damage > bullet.m_damage)
		return false;

	if (lost_damage > 0.f)
		bullet.m_damage -= lost_damage;

	if (bullet.m_damage < 1.f)
		return false;

	bullet.m_pos = trace.m_end_pos;
	bullet.m_walls_count--;

	return true;
}

bool c_autowall::trace_to_exit(c_game_trace* enter_trace, vec3_t& start, vec3_t& dir, c_game_trace* exit_trace) {
	vec3_t end;
	auto distance = 0.f;
	auto distance_check = 23;
	auto first_contents = 0;

	do {
		distance += 4.f;
		end = start + dir * distance;

		if (!first_contents)
			first_contents = interfaces::m_trace_system->get_point_contents(end, MASK_SHOT | CONTENTS_GRATE, NULL);

		auto point_contents = interfaces::m_trace_system->get_point_contents(end, MASK_SHOT | CONTENTS_GRATE, NULL);
		if (!(point_contents & (MASK_SHOT_HULL)) || (point_contents & CONTENTS_HITBOX && point_contents != first_contents)) {
			auto new_end = end - (dir * 4.f);

			ray_t ray(end, new_end);

			interfaces::m_trace_system->trace_ray(ray, MASK_SHOT | CONTENTS_GRATE, nullptr, exit_trace);

			if (exit_trace->m_start_solid && exit_trace->m_surface.m_flags & SURF_HITBOX) {
				ray_t ray1(start, end);

				c_trace_filter filter;
				filter.m_skip = exit_trace->m_hit_entity;

				interfaces::m_trace_system->trace_ray(ray, MASK_SHOT | CONTENTS_GRATE, &filter, exit_trace);

				if (exit_trace->did_hit() && !exit_trace->m_start_solid)
					return true;

				continue;
			}

			if (exit_trace->did_hit() && !exit_trace->m_start_solid) {
				if (enter_trace->m_surface.m_flags & SURF_NODRAW || !(exit_trace->m_surface.m_flags & SURF_NODRAW)) {
					if (exit_trace->m_plane.m_normal.dot_product(dir) <= 1.f)
						return true;

					continue;
				}

				if (is_breakable((c_cs_player*)enter_trace->m_hit_entity) && is_breakable((c_cs_player*)exit_trace->m_hit_entity))
					return true;

				continue;
			}

			if (exit_trace->m_surface.m_flags & SURF_NODRAW) {
				if (is_breakable((c_cs_player*)enter_trace->m_hit_entity) && is_breakable((c_cs_player*)exit_trace->m_hit_entity))
					return true;
				else if (!(enter_trace->m_surface.m_flags & SURF_NODRAW))
					continue;
			}

			if ((!enter_trace->m_hit_entity || enter_trace->m_hit_entity->get_index() == 0) && is_breakable((c_cs_player*)enter_trace->m_hit_entity)) {
				exit_trace = enter_trace;
				exit_trace->m_end_pos = start + dir;
				return true;
			}
			continue;
		}

		distance_check--;
	} while (distance_check);

	return false;
}

autowall_info_t c_autowall::start(vec3_t start, vec3_t end, c_cs_player* from, c_cs_player* to) {
	autowall_info_t return_info{};

	autowall_bullet_t bullet;
	bullet.m_start = start;
	bullet.m_end = end;
	bullet.m_pos = start;
	bullet.m_tickness = 0.f;
	bullet.m_walls_count = 4;
	bullet.m_dir = math::calc_angle(start, end).vector();

	auto filter_player = c_trace_filter();
	filter_player.m_skip = to;

	auto filter_self = c_trace_filter();
	filter_self.m_skip = from;

	if (!to)
		bullet.m_filter = &filter_self;

	auto weapon = globals::m_local->get_active_weapon();

	if (!weapon)
		return return_info;

	auto data = weapon->get_cs_weapon_data();

	if (!data)
		return return_info;

	end = start + bullet.m_dir * (data->m_weapon_type == WEAPON_TYPE_KNIFE ? 45.f : data->m_range);
	bullet.m_damage = data->m_damage;

	while (bullet.m_damage > 0 && bullet.m_walls_count > 0) {
		return_info.m_walls_count = bullet.m_walls_count;

		ray_t ray(bullet.m_pos, end);

		c_trace_filter filter;
		filter.m_skip = from;

		interfaces::m_trace_system->trace_ray(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &bullet.m_trace);
		clip_trace_to_player(bullet.m_pos, bullet.m_pos + bullet.m_dir * 40.f, to, MASK_SHOT | CONTENTS_GRATE, bullet.m_filter, &bullet.m_trace);

		bullet.m_damage *= std::pow(data->m_range_modifier, (bullet.m_trace.m_end_pos - start).length() / 500.f);

		if (bullet.m_trace.m_fraction == 1.f) {
			return_info.m_damage = bullet.m_damage;
			return_info.m_hitgroup = -1;
			return_info.m_end = bullet.m_trace.m_end_pos;
			return_info.m_player = nullptr;
		}

		if (bullet.m_trace.m_hitgroup > 0 && bullet.m_trace.m_hitgroup <= 7) {
			if ((to && bullet.m_trace.m_hit_entity->get_index() != to->get_index()) | ((c_cs_player*)bullet.m_trace.m_hit_entity)->get_team() == from->get_team()) {
				return_info.m_damage = -1;

				return return_info;
			}

			scale_damage(&bullet.m_trace, data, bullet.m_trace.m_hitgroup, bullet.m_damage);

			return_info.m_damage = bullet.m_damage;
			return_info.m_hitgroup = bullet.m_trace.m_hitgroup;
			return_info.m_end = bullet.m_trace.m_end_pos;
			return_info.m_player = (c_cs_player*)bullet.m_trace.m_hit_entity;

			break;
		}

		if (!handle_bullet_penetration(data, bullet))
			break;

		return_info.m_penetrate = true;
	}

	return_info.m_walls_count = bullet.m_walls_count;
	return return_info;
}

bool c_autowall::is_breakable(c_cs_player* player) {
	using func = bool(__fastcall*)(c_cs_player*); 

	static auto function = *SIG("client.dll", "55 8B EC 51 56 8B F1 85 F6 74 68 83 BE").cast<func>();

	if (!player)
		return false;

	auto take_damage{ (char*)((uintptr_t)player + *(size_t*)((uintptr_t)function + 38)) };
	auto take_damage_backup{ *take_damage };

	auto cclass = interfaces::m_client_dll->get_all_classes();

	if ((cclass->m_network_name[1]) != 'F' || (cclass->m_network_name[4]) != 'c' || (cclass->m_network_name[5]) != 'B' || (cclass->m_network_name[9]) != 'h')
		*take_damage = 2;

	auto breakable = function(player);
	*take_damage = take_damage_backup;

	return breakable;
}

float c_autowall::get_damage(vec3_t point, c_cs_player* player) {
	return start(globals::m_local->get_eye_pos(), point, globals::m_local, player).m_damage;
}