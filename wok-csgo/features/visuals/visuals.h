#include "../../globals.h"

class c_visuals : public c_singleton<c_visuals> {
public:

	void on_paint();

	void draw_box(c_cs_player* player, vec3_t pos, vec3_t top, col_t color);
	void draw_name(c_cs_player* player, int index, vec3_t pos, vec3_t top, col_t color);
	void draw_health(c_cs_player* player, vec3_t pos, vec3_t top);
};

#define visuals c_visuals::instance()