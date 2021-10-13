#include "hooks.h"

namespace hooks {
	void init() {
		// // // // // // // // // // // // // // // // // // // // // // //

		m_d3d_device = std::make_unique<memory::hook_t>(interfaces::m_d3d_device);

		m_d3d_device->hook(d3d_device::reset::index, d3d_device::reset::fn);
		m_d3d_device->hook(d3d_device::present::index, d3d_device::present::fn);

		// // // // // // // // // // // // // // // // // // // // // // //

		m_client_dll = std::make_unique<memory::hook_t>(interfaces::m_client_dll);

		m_client_dll->hook(client_dll::frame_stage_notify::index, client_dll::frame_stage_notify::fn);
		m_client_dll->hook(client_dll::create_move::index, client_dll::create_move::gate);

		// // // // // // // // // // // // // // // // // // // // // // //

		m_net_channel = std::make_unique<memory::hook_t>(interfaces::m_client_state->m_net_channel);

		m_net_channel->hook(net_channel::process_packet::index, net_channel::process_packet::fn);
		m_net_channel->hook(net_channel::send_net_message::index, net_channel::send_net_message::fn);

		// // // // // // // // // // // // // // // // // // // // // // //

		m_engine_client = std::make_unique<memory::hook_t>(interfaces::m_engine);

		m_engine_client->hook(engine_client::is_hltv::index, engine_client::is_hltv::fn);

		m_engine_client->hook(engine_client::is_paused::index, engine_client::is_paused::fn);

		// // // // // // // // // // // // // // // // // // // // // // //

		m_client_mode = std::make_unique<memory::hook_t>(interfaces::m_client_mode);

		m_client_mode->hook(client_mode::override_view::index, client_mode::override_view::fn);

		// // // // // // // // // // // // // // // // // // // // // // //

		m_model_render = std::make_unique<memory::hook_t>(interfaces::m_model_render);

		m_model_render->hook(model_render::draw_model_execute::index, model_render::draw_model_execute::fn);

		// // // // // // // // // // // // // // // // // // // // // // //

		m_panel = std::make_unique<memory::hook_t>(interfaces::m_panel);

		m_panel->hook(panel::paint_traverse::index, panel::paint_traverse::fn);

		// // // // // // // // // // // // // // // // // // // // // // //

		m_surface = std::make_unique<memory::hook_t>(interfaces::m_surface);

		m_surface->hook(surface::lock_cursor::index, surface::lock_cursor::fn);

		// // // // // // // // // // // // // // // // // // // // // // //

		m_renderable = std::make_unique<memory::hook_t>(i_client_renderable::get_vtable());

		m_renderable->hook(renderable::setup_bones::index, renderable::setup_bones::fn);

		// // // // // // // // // // // // // // // // // // // // // // //
	}

	void undo() {
		m_renderable->unhook();
		m_surface->unhook();
		m_panel->unhook();
		m_model_render->unhook();
		m_client_mode->unhook();
		m_client_dll->unhook();
		m_d3d_device->unhook();
		m_net_channel->unhook();
		m_engine_client->unhook();
	}

	std::unique_ptr<memory::hook_t> m_d3d_device = nullptr;
	std::unique_ptr<memory::hook_t> m_client_dll = nullptr;
	std::unique_ptr<memory::hook_t> m_client_mode = nullptr;
	std::unique_ptr<memory::hook_t> m_model_render = nullptr;
	std::unique_ptr<memory::hook_t> m_panel = nullptr;
	std::unique_ptr<memory::hook_t> m_surface = nullptr;
	std::unique_ptr<memory::hook_t> m_renderable = nullptr;
	std::unique_ptr<memory::hook_t> m_net_channel = nullptr;
	std::unique_ptr<memory::hook_t> m_engine_client = nullptr;
}
