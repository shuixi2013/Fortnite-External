#pragma once

void AimbotTab()
{
	ImGui::PushItemWidth(180.f);
	ImGui::SetCursorPos(ImVec2(140, 35));
	ImGui::Checkbox(XorStr("Enable Aimbot").c_str(), &g_aimbot);

	if (g_aimbot)
	{
		ImGui::SameLine(0, 1);

		HotkeyButton(hotkeys::aimkey, ChangeKey, keystatus);
		ImGui::SetCursorPos(ImVec2(140, 55));
		ImGui::Checkbox(XorStr("Controller LTrigger").c_str(), &g_controller);
		if (g_controller)
			hotkeys::aimkey = false;
	}

	if (g_aimbot)
	{
		ImGui::SetCursorPos(ImVec2(140, 75));
		ImGui::Checkbox(XorStr("Mouse Aimbot").c_str(), &g_mouse_aim);
		ImGui::SetCursorPos(ImVec2(140, 95));
		ImGui::Checkbox(XorStr("Memory Aimbot (Humanized)").c_str(), &g_mouse_aim);
		ImGui::SetCursorPos(ImVec2(140, 115));
		ImGui::Checkbox(XorStr("Skip Knocked Players").c_str(), &g_skipknocked);

		ImGui::SetCursorPos(ImVec2(140, 135));
		ImGui::Text(XorStr("Bone Target").c_str());
		ImGui::SetCursorPos(ImVec2(140, 150));
		ImGui::Checkbox(XorStr("Head").c_str(), &Head);
		if (Head)
		{
			Neck = false;
			Chest = false;
			hitbox = 0;
		}

		ImGui::SameLine();
		ImGui::Checkbox(XorStr("Neck").c_str(), &Neck);
		if (Neck)
		{
			Head = false;
			Chest = false;
			hitbox = 1;
		}

		ImGui::SameLine();
		ImGui::Checkbox(XorStr("Chest").c_str(), &Chest);
		if (Chest)
		{
			Neck = false;
			Head = false;
			hitbox = 2;
		}

	}

	if (g_aimbot)
	{
		ImGui::SetCursorPos(ImVec2(140, 170));
		ImGui::Text("Smoothness");
		ImGui::SetCursorPos(ImVec2(140, 185));
		ImGui::SliderFloat(XorStr("    ").c_str(), &AimbotSmoothnessValue, .5, 30);
	}

	ImGui::SetCursorPos(ImVec2(140, 210));
	ImGui::Text("Max Aimbot Distance");
	ImGui::SetCursorPos(ImVec2(140, 235));
	ImGui::SliderInt(XorStr("").c_str(), &AimbotDistance, 10, 280);
};

void VisualsTab()
{
	ImGui::PushItemWidth(180.f);

	ImGui::SetCursorPos(ImVec2(140, 35));
	ImGui::Checkbox(XorStr("3D Bounding Box").c_str(), &g_3d_box);
	ImGui::SetCursorPos(ImVec2(140, 55));
	ImGui::Checkbox(XorStr("Corner Box ESP").c_str(), &g_cornerboxesp);
	ImGui::SetCursorPos(ImVec2(140, 75));
	ImGui::Checkbox(XorStr("Basic Box ESP").c_str(), &g_boxesp);
	ImGui::SetCursorPos(ImVec2(140, 95));
	ImGui::Checkbox(XorStr("Line Esp").c_str(), &g_lineesp);

	ImGui::SetCursorPos(ImVec2(140, 115));
	ImGui::Checkbox(XorStr("Skeleton ESP").c_str(), &g_esp_skeleton);

	ImGui::SetCursorPos(ImVec2(140, 135));
	ImGui::Checkbox(XorStr("Distance ESP").c_str(), &g_esp_distance);
	ImGui::SetCursorPos(ImVec2(140, 155));
	ImGui::Checkbox(XorStr("Equipped Weapon ESP").c_str(), &g_curweaponesp);
	ImGui::SetCursorPos(ImVec2(140, 175));
	ImGui::Checkbox(XorStr("Platform ESP").c_str(), &g_platform_esp);
	ImGui::SetCursorPos(ImVec2(140, 195));
	ImGui::Text("Max ESP Render Distance");
	ImGui::SetCursorPos(ImVec2(140, 210));
	ImGui::SliderInt(XorStr(" ").c_str(), &EspRenderingDistance, 10, 250);
};

void MiscTab()
{
	ImGui::SetCursorPos(ImVec2(140, 35));
	ImGui::Checkbox(XorStr("Draw Crosshair").c_str(), &g_crossh);
	ImGui::SetCursorPos(ImVec2(140, 55));
	ImGui::Checkbox(XorStr("Draw Circle FOV").c_str(), &g_circlefov);

	ImGui::PushItemWidth(180.f);

	if (g_fov || g_circlefov)
	{
		ImGui::SetCursorPos(ImVec2(140, 75));
		ImGui::Text("FOV");
		ImGui::SetCursorPos(ImVec2(140, 90));
		ImGui::SliderFloat(XorStr("    ").c_str(), &AimbotValue, 10, 1000);
	}
}

void WorldTab()
{
	ImGui::Spacing();
	ImGui::SetCursorPos(ImVec2(140, 35));
	ImGui::Checkbox(XorStr("Loot ESP").c_str(), &g_loot);
	ImGui::SetCursorPos(ImVec2(140, 55));
	ImGui::Checkbox(XorStr("Utils ESP").c_str(), &g_utils);
	ImGui::SetCursorPos(ImVec2(140, 75));
	ImGui::Checkbox(XorStr("Vehicle Esp").c_str(), &g_vehicles);
	ImGui::SetCursorPos(ImVec2(140, 95));
	ImGui::Checkbox(XorStr("Chest ESP").c_str(), &g_chests);
	ImGui::SetCursorPos(ImVec2(140, 115));
	ImGui::Checkbox(XorStr("Ammo Box ESP").c_str(), &g_ammo);
	ImGui::PushItemWidth(180.f);
	ImGui::Spacing();
	ImGui::SetCursorPos(ImVec2(140, 135));
	ImGui::Text("Max Rendering");
	ImGui::SetCursorPos(ImVec2(140, 150));
	ImGui::SliderInt(XorStr(" ").c_str(), &LootRenderingDistance, 5, 50);
	ImGui::Spacing();
}

void ExploitsTab()
{
	ImGui::SetCursorPos(ImVec2(140, 35));
	ImGui::Checkbox(XorStr("No Bloom").c_str(), &g_gun_tracers);

	ImGui::SetCursorPos(ImVec2(140, 55));
	ImGui::Checkbox(XorStr("Spinbot").c_str(), &g_spinbot);

	ImGui::PushItemWidth(180.f);
	ImGui::SetCursorPos(ImVec2(140, 75));
	ImGui::Checkbox("Fov Changer", &g_fovchanger);
	if (g_fovchanger)
	{
		ImGui::SetCursorPos(ImVec2(140, 95));
		ImGui::Text("FOV CHANGER");
		ImGui::SetCursorPos(ImVec2(140, 115));
		ImGui::SliderFloat(("                 "), &FOVChangerValue, 90.0f, 170.0f, ("%.2f"));
	}
}