#include "menu.h"

void c_menu::on_paint() {
	if (!(input::m_blocked = input::get_key<TOGGLE>(VK_INSERT)))
		return;

	ImGui::SetNextWindowPos(ImGui::GetIO().DisplaySize / 2.f, ImGuiCond_Once, ImVec2(0.5f, 0.5f));

	ImGui::SetNextWindowSize(ImVec2(500, 450), ImGuiCond_Once);

	if (ImGui::Begin(_("mollekt"), 0, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse)) {
		ImGui::Checkbox(_("ragebot enable"), &cfg::get<bool>(FNV1A("ragebot_enable")));
		ImGui::Checkbox(_("autoshoot enable"), &cfg::get<bool>(FNV1A("autoshoot_enable")));
		ImGui::Checkbox(_("autoscope enable"), &cfg::get<bool>(FNV1A("autoscope_enable")));
		ImGui::Checkbox(_("autostop enable"), &cfg::get<bool>(FNV1A("autostop_enable")));

		const char* hitboxes[] = { "none", "head", "body", "arms", "legs"};
		ImGui::Combo(_("hitboxes"), &cfg::get<int>(FNV1A("hitbox_num")), hitboxes, IM_ARRAYSIZE(hitboxes));

		ImGui::Spacing();

		ImGui::Checkbox(_("point scale enable"), &cfg::get<bool>(FNV1A("point_scale")));

		if (cfg::get<bool>(FNV1A("point_scale"))) {
			ImGui::SliderFloat(_("head scale"), &cfg::get<float>(FNV1A("head_scale")), 1.9f, 100.f);
			ImGui::SliderFloat(_("body scale"), &cfg::get<float>(FNV1A("body_scale")), 1.0f, 100.f);
		}

		ImGui::SliderInt(_("min damage"), &cfg::get<int>(FNV1A("min_damage")), 0, 120);
		ImGui::SliderFloat(_("hitchance amount"), &cfg::get<float>(FNV1A("hitchance_amount")), 0.f, 100.f);

		ImGui::Spacing();

		ImGui::Checkbox(_("visuals enable"), &cfg::get<bool>(FNV1A("visuals_enable")));
		ImGui::Checkbox(_("box esp"), &cfg::get<bool>(FNV1A("box_esp")));
		ImGui::Checkbox(_("name esp"), &cfg::get<bool>(FNV1A("name_esp")));
		ImGui::Checkbox(_("health esp"), &cfg::get<bool>(FNV1A("health_esp")));

		ImGui::Spacing();

		ImGui::Checkbox(_("bunny hop"), &cfg::get<bool>(FNV1A("bunnyhop")));
		ImGui::Checkbox(_("directional strafer"), &cfg::get<bool>(FNV1A("directional_strafer")));

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::Button(_("save"))) {
			cfg::save();
		}

		if (ImGui::Button(_("load"))) {
			cfg::load();
		}
	}
	ImGui::End();
}
