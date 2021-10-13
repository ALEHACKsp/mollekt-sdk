#include "../hooks.h"

void __fastcall hooks::net_channel::process_packet::fn(void* ecx, void* edx, void* packet, bool header) {
	static const auto original = m_net_channel->get_original<T>(index);

	if (!interfaces::m_client_state->m_net_channel)
		return original(ecx, packet, header);

	for (event_info_t* it{ interfaces::m_client_state->m_events }; it != nullptr; it = it->m_next) {
		if (!it->m_class_id)
			continue;

		it->m_delay = 0.f;
	}

	interfaces::m_engine->fire_events();
}

bool __fastcall hooks::net_channel::send_net_message::fn(i_net_channel* net_chan, void* edx, i_net_msg& msg, bool reliable, bool voice) {
	static const auto original = m_net_channel->get_original<T>(index);

	if (msg.get_group() == 9)
		voice = true;

	return original(net_chan, msg, reliable, voice);
}