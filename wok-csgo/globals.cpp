#include "globals.h"

namespace globals {
	namespace angles {
		qangle_t    m_view = {};
		qangle_t    m_anim = {};
	}

	HMODULE			m_module = nullptr;
	c_local_player	m_local = {};
	c_user_cmd*		m_cur_cmd = nullptr;
	bool			m_packet = true;
	bool			m_setup_bones = false;
	float           m_lerp = 0.0f;
}