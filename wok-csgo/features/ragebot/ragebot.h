#include "../../globals.h"

class c_ragebot : public c_singleton<c_ragebot> {
public:
	void on_create_move(c_user_cmd* cmd);
	void autostop();
	void reset_player();

	int find_target();

	float point_scale(float hitbox_radius, vec3_t pos, vec3_t point, int hitbox);

	std::vector<vec3_t> get_hitbox(c_cs_player* player);
	vec3_t scan(c_cs_player* player, int* hitbox = nullptr, int* est_damage = nullptr);

	bool hitchance(c_cs_player* player, qangle_t angles, float hitchance);
	bool is_able_to_shoot();
	bool is_valid_player(c_cs_player* player);
	bool get_multi_points(c_cs_player* player, matrix3x4_t bones[], int index, std::vector<vec3_t>& points);
	bool run(c_cs_player* player, vec3_t point, int index);

	c_lag_record* find_record(int index);

public:
	// variables
	matrix3x4_t	  m_matrix[128];
	c_user_cmd* m_cmd;
	int			  m_index;
	c_lag_record* m_record;
};

#define ragebot c_ragebot::instance()