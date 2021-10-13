#include "../../globals.h"

struct autowall_info_t {
	int m_damage = -1;
	int m_hitgroup = -1;
	int m_walls_count = 4;

	bool m_penetrate = false;

	float m_tickness = 1.f;

	c_cs_player* m_player = nullptr;

	vec3_t m_end;
};

struct autowall_bullet_t {
	vec3_t m_start;
	vec3_t m_end;

	vec3_t m_pos;
	vec3_t m_dir;

	i_trace_filter* m_filter = nullptr;
	c_game_trace m_trace;

	float m_tickness = 1.f;
	float m_damage = 1.f;

	int m_walls_count = 4;
};

class c_autowall : public c_singleton<c_autowall> {
public:
	autowall_info_t start(vec3_t start, vec3_t end, c_cs_player* from, c_cs_player* to);
	float get_damage(vec3_t point, c_cs_player* player);
private:
	void scale_damage(c_game_trace* trace, c_cs_weapon_data* data, int& hitgroup, float& damage);
	void clip_trace_to_player(vec3_t start, vec3_t end, c_cs_player* player, int mask, i_trace_filter* filter, c_game_trace* trace);
	bool handle_bullet_penetration(c_cs_weapon_data* data, autowall_bullet_t& bullet);

	bool trace_to_exit(c_game_trace* enter_trace, vec3_t& start, vec3_t& dir, c_game_trace* exit_trace);
	bool is_breakable(c_cs_player* player);
};

#define autowall c_autowall::instance()