#include "../hooks.h"

bool __fastcall hooks::renderable::setup_bones::fn(i_client_renderable* ecx, void* edx, matrix3x4_t* bones, int max_bones, int mask, float time) { 
	static const auto original = m_renderable->get_original<T>(index);

	auto return_value = true;

	static auto r_jiggle_bones = interfaces::m_cvar_system->find_var(FNV1A("r_jiggle_bones"));
	auto backup_bones = r_jiggle_bones->get_int();

	r_jiggle_bones->set_value(0);

	return_value = original(ecx, bones, max_bones, mask, time);

	r_jiggle_bones->set_value(backup_bones);

	return return_value;
}