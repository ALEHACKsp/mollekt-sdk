#include "../hooks.h"

bool __fastcall hooks::engine_client::is_hltv::fn(void* ecx, void* edx) {
	static const auto original = m_engine_client->get_original<T>(index);

	if (globals::m_setup_bones)
		return true;

	static auto accum_layers = *SIG("client.dll", "84 C0 75 0D F6 87");

	if (uintptr_t(_ReturnAddress()) == accum_layers)
		return true;

	static auto setup_velocity = *SIG("client.dll", "84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ? FF D0");

	if (uintptr_t(_ReturnAddress()) == setup_velocity)
		return true;

	return original(ecx);
}

bool __fastcall hooks::engine_client::is_paused::fn(void* ecx, void* edx) {
	static const auto original = m_engine_client->get_original<T>(index);

	static auto enable_extrapolation = SIG("client.dll", "FF D0 A1 ?? ?? ?? ?? B9 ?? ?? ?? ?? D9 1D ?? ?? ?? ?? FF 50 34 85 C0 74 22 8B 0D ?? ?? ?? ??").self_offset(0x29).cast<DWORD*>();

	if (_ReturnAddress() == (void*)enable_extrapolation)
		return true;

	return original(ecx);
}
