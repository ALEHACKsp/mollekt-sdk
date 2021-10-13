#include "../hooks.h"

void __stdcall hooks::client_dll::frame_stage_notify::fn(e_client_frame_stage stage) {
	static const auto original = m_client_dll->get_original<T>(index);

	if (stage == FRAME_RENDER_START && interfaces::m_engine->is_in_game()) {
		interfaces::m_engine->get_view_angles(globals::angles::m_view);
	}

	lag_comp->on_net_update(stage);

	original(stage);
}

__declspec (naked) void __stdcall hooks::client_dll::create_move::gate(int sequence_number, float input_sample_frame_time, bool active) {
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esp
		push dword ptr [active]
		push dword ptr [input_sample_frame_time]
		push dword ptr [sequence_number]
		call fn
		pop ebx
		pop ebp
		retn 0Ch
	}
}

void __stdcall hooks::client_dll::create_move::fn(int sequence_number, float input_sample_frame_time, bool active, bool& packet) {
	static const auto original = m_client_dll->get_original<T>(index);

	original(interfaces::m_client_dll, sequence_number, input_sample_frame_time, active);

	globals::m_packet = packet = true;

	if (!globals::m_local)
		return;

	const auto cmd = interfaces::m_input->get_user_cmd(sequence_number);

	if (!cmd || !cmd->m_command_number)
		return;

	globals::m_cur_cmd = cmd;

	auto get_lerp_time = std::max(interfaces::m_cvar_system->find_var(FNV1A("cl_interp"))->get_float(), interfaces::m_cvar_system->find_var(FNV1A("cl_interp_ratio"))->get_float() / interfaces::m_cvar_system->find_var(FNV1A("cl_updaterate"))->get_float());
	globals::m_lerp = get_lerp_time;

	movement->set_view_angles(cmd->m_view_angles);

	engine_prediction->update();

	movement->auto_strafer(cmd->m_view_angles);

	movement->on_create_move(false);

	engine_prediction->process();

	ragebot->on_create_move(cmd);

	engine_prediction->restore();

	cmd->m_view_angles.sanitize();

	movement->on_create_move(true);

	cmd->m_view_angles.sanitize();

	globals::angles::m_anim = cmd->m_view_angles;

	packet = globals::m_packet;

	const auto verified = interfaces::m_input->get_verified_user_cmd(sequence_number);

	verified->m_cmd = *cmd;
	verified->m_crc = cmd->get_check_sum();
}
