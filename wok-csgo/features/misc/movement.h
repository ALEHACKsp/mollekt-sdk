#include "../../globals.h"

class c_movement : public c_singleton<c_movement> {
private:
	void compute_buttons();

	qangle_t m_view_angles;

public:
	void on_create_move(bool a1);

	void set_view_angles(const qangle_t& view_angles) { m_view_angles = view_angles; }

	void bunny_hop();

	void auto_strafer(qangle_t& angle);

};
#define movement c_movement::instance()