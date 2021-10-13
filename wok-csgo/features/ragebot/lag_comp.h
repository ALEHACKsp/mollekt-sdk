#include "../../globals.h"

class c_lag_record {
public:

	c_cs_player* m_player;
	int m_index;

	bool m_invalid;
	bool is_immune;
	bool is_dormant;

	bit_flag_t<int32_t> m_flags;

	float m_sim_time;
	float m_old_sim_time;
	float m_duck_amount;

	float m_lby;

	qangle_t m_angles;
	qangle_t m_abs_angles;

	vec3_t m_velocity;
	vec3_t m_origin;

	vec3_t m_obb_min;
	vec3_t m_obb_max;

	c_lag_record();

	void reset();

	c_lag_record(c_cs_player* player);

	void store_record(c_cs_player* player);

	void apply_record();

	bool valid(bool extra_checks = true);
};

class c_lag_comp : public c_singleton<c_lag_comp> {
public:

	void on_net_update(e_client_frame_stage stage);

	bool valid(int i, c_cs_player* player);

	void update_animations(c_cs_player* player);

	bool is_dormant[65];
};

extern std::deque<c_lag_record> m_player_records[65];

#define lag_comp c_lag_comp::instance()