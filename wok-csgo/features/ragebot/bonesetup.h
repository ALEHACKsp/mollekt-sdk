#include "../../globals.h"

class c_bonesetup : public c_singleton<c_bonesetup> {
public:

	bool setup_bones_rebuild(c_cs_player* player, int mask, matrix3x4_t* out);
};

#define bone_setup c_bonesetup::instance()