/*
	Gloomy.cc
	https://github.com/Chase1803

	Copyright (c) 2022 Chase1803
	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:
	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
 */

#include "utils.hpp"

using namespace Globals;
using namespace Settings;
using namespace Camera;
using namespace MeshBones;

#include "window.h"

#include "LevelsLoop.h"
#include "PlayersLoop.h"

#include "LevelEntities.h"
#include "doAimbot.h"

bool CheatLoop()
{
	bool bValidEnemyInArea = false;
	float ClosestActorDistance = FLT_MAX;
	Vector3 ClosestActorMouseAimbotPosition = Vector3(0.0f, 0.0f, 0.0f);
	float distance, distaim;

	try
	{
		for (LootEntity LEntityList : LootentityList) 
		{
			LevelEntEsp(LEntityList);
		}

		for (Playertest EntityList : PLIST)
		{
			auto identify = g_functions::f_getbonewithIndex(0, 0);
			g_functions::ConvertWorld2Screen(identify);

			uintptr_t RootComponent = read<uintptr_t>(g_pid, EntityList.ACurrentActor + 0x188);
			LocalPlayerRelativeLocation = read<Vector3>(g_pid, EntityList.USceneComponent + 0x128);

			if (LocalPawn)
			{
				const auto SetupCameraRotationAndFov = [](uintptr_t LocalPlayer, uintptr_t RootComponent, Vector3& CameraRotation, float& FOVAngle)
				{
					auto CameraRotChain_tmp0 = read<uint64_t>(g_pid, LocalPlayer + 0xC8);
					auto CameraRotChain_tmp1 = read<uint64_t>(g_pid, CameraRotChain_tmp0 + 0x8);

					Rotation.x = read<double>(g_pid, CameraRotChain_tmp1 + 0xAE0);

					double tester = asin(Rotation.x);
					double degreees = tester * (180.0 / M_PI);

					Rotation.y = read<double>(g_pid, RootComponent + 0x148);
					Rotation.x = degreees;

					if (g_fovchanger)
					{
						FOVAngle = FOVChangerValue;
					}
					else
					{
						FOVAngle = 80.0f / (read<double>(g_pid, CameraRotChain_tmp1 + 0x610) / 1.19f);
					}

				};
				SetupCameraRotationAndFov(LocalPlayer, LocalPawnRootComponent, Rotation, FovAngle);
			}
			else {
				auto CameraCache = read<FMinimalViewInfo>(g_pid, PlayerCameraManager + 0x28d0 + 0x10);

				Rotation = CameraCache.Rotation;
				Location = CameraCache.Location;

				if (g_fovchanger)
				{
					FovAngle = FOVChangerValue;
				}
				else
				{
					FovAngle = CameraCache.FOV;
				}
			}

			const auto SetupCameraLocation = [](uintptr_t LocalPlayer, Vector3& CameraLocation)
			{
				auto CameraLocChain_tmp0 = read<uint64_t>(g_pid, LocalPlayer + 0x70);
				auto CameraLocChain_tmp1 = read<uint64_t>(g_pid, CameraLocChain_tmp0 + 0x98);
				auto CameraLocChain_tmp2 = read<uint64_t>(g_pid, CameraLocChain_tmp1 + 0x180);

				CameraLocation = read<Vector3>(g_pid, CameraLocChain_tmp2 + 0x20);

			};
			SetupCameraLocation(LocalPlayer, Location);

			uint64_t PlayerState = read<uint64_t>(g_pid, EntityList.ACurrentActor + 0x290);

			int TeamIndex = read<int>(g_pid, PlayerState + 0x1010);

			if (g_fovchanger)
			{
				write(g_pid, PlayerCameraManager + 0x288, FOVChangerValue);
			}

			Vector3 vHeadBone = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 98);
			Vector3 vRootBone = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 0);

			Vector3 vHeadBoneOut = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x, vHeadBone.y, vHeadBone.z + 15));

			Vector3 AimbotHeadOut = g_functions::ConvertWorld2Screen(vHeadBone);
			Vector3 vRootBoneOut = g_functions::ConvertWorld2Screen(vRootBone);

			float BoxHeight = abs(vHeadBoneOut.y - vRootBoneOut.y);
			float BoxWidth = BoxHeight * 0.80;

			Vector3 RelativeInternalLocation = read<Vector3>(g_pid, RootComponent + 0x128);
			if (!RelativeInternalLocation.x && !RelativeInternalLocation.y) {
				continue;
			}

			Vector3 RelativeScreenLocation = g_functions::ConvertWorld2Screen(RelativeInternalLocation);
			if (!RelativeScreenLocation.x && !RelativeScreenLocation.y) {
				continue;
			}

			distance = LocalPlayerRelativeLocation.Distance(RelativeInternalLocation) / 100.f;

			if (g_platform_esp)
			{
				uint64_t AFortPlayerState = read<uint64_t>(g_pid, 0x0);//AFortPlayerState->Platform*FString
				auto nameptr = read<uintptr_t>(g_pid, AFortPlayerState + 0x420);

				wchar_t* Platform = g_functions::ReadFString(nameptr);

				std::string Text = wchar_to_char(Platform);

				if (strstr(Text.c_str(), ("WIN")))
				{
					DrawString(13, vHeadBoneOut.x, vHeadBoneOut.y - 20, &Col.white, true, true, "PC");

				}
				else if (strstr(Text.c_str(), ("XBL")) || strstr(Text.c_str(), ("XSX")))
				{
					DrawString(13, vHeadBoneOut.x, vHeadBoneOut.y - 20, &Col.green, true, true, "Xbox");

				}
				else if (strstr(Text.c_str(), ("PSN")) || strstr(Text.c_str(), ("PS5")))
				{
					DrawString(13, vHeadBoneOut.x, vHeadBoneOut.y - 20, &Col.lightblue, true, true, "PSN");

				}
				else if (strstr(Text.c_str(), ("SWT")))
				{
					DrawString(13, vHeadBoneOut.x, vHeadBoneOut.y - 20, &Col.red, true, true, "Nintendo");

				}
				else
					DrawString(13, vHeadBoneOut.x, vHeadBoneOut.y - 20, &Col.white, true, true, Text.c_str());
			}

			DrawBoxHead(EntityList.USkeletalMeshComponent, vHeadBone);

			if (g_curweaponesp)
			{
				uintptr_t ItemRootComponent = read<uintptr_t>(g_pid, EntityList.ACurrentActor + 0x188);
				Vector3 ItemPosition = read<Vector3>(g_pid, ItemRootComponent + 0x128);
				float ItemDist = LocalPlayerRelativeLocation.Distance(ItemPosition) / 100.f;

				if (ItemDist < LootRenderingDistance) {

					auto CurrentWeapon = read<uintptr_t>(g_pid, EntityList.ACurrentActor + 0x790);
					auto ItemData = read<DWORD_PTR>(g_pid, CurrentWeapon + 0x3d8);
					BYTE tier = read<BYTE>(g_pid, ItemData + 0x74);

					ImColor Color;
					if (tier == 2)
					{
						Color = IM_COL32(0, 255, 0, 255);
					}
					else if ((tier == 3))
					{
						Color = IM_COL32(0, 0, 255, 255);
					}
					else if ((tier == 4))
					{
						Color = IM_COL32(128, 0, 128, 255);
					}
					else if ((tier == 5))
					{
						Color = IM_COL32(255, 255, 0, 255);
					}
					else if ((tier == 6))
					{
						Color = IM_COL32(255, 255, 0, 255);
					}
					else if ((tier == 0) || (tier == 1))
					{
						Color = IM_COL32(255, 255, 255, 255);
					}

					auto AmmoCount = read<int>(g_pid, CurrentWeapon + 0xb24);
					auto bIsReloadingWeapon = read<bool>(g_pid, CurrentWeapon + 0x311);

					auto DisplayName = read<uint64_t>(g_pid, ItemData + 0x90);

					auto Name = g_functions::ReadFText(DisplayName);
					std::string Text = wchar_to_char(Name);

					std::string Check;

					if (strstr(EntityList.GNames.c_str(), "PlayerPawn_Athena_C"))
						Check = XorStr("Player").c_str();
					else
						Check = XorStr("Bot/Npc").c_str();

					ImVec2 TextSize = ImGui::CalcTextSize(Check.c_str());
					ImGui::GetOverlayDrawList()->AddText(ImVec2(vHeadBoneOut.x - 30 - TextSize.x / 2, vHeadBoneOut.y - 15 - TextSize.y / 2), ImGui::GetColorU32({ 255, 255, 255, 255 }), Check.c_str());

					if (AmmoCount)
					{
						char buffer[128];
						sprintf_s(buffer, "Ammo: %i", AmmoCount);
						if (buffer != "?")
						{
							ImGui::GetOverlayDrawList()->AddText(ImVec2(vpelvis.x, vpelvis.y + 15), IM_COL32(255, 255, 255, 255), buffer);
						}
					}

					if (bIsReloadingWeapon)
						ImGui::GetOverlayDrawList()->AddText(ImVec2(vpelvis.x - 30, vpelvis.y), IM_COL32(255, 255, 255, 255), "Reloading");
					else
						ImGui::GetOverlayDrawList()->AddText(ImVec2(vpelvis.x - 30, vpelvis.y), Color, Text.c_str());
				}
			}

			if (TeamIndex != LocalTeam || InLobby) {

				isVis = isVisible(EntityList.USkeletalMeshComponent);

				if (isVis)
				{
					ESPColor = { 255, 215, 0, 255 };
					ESPColorU = ImGui::GetColorU32({ 255, 215, 0, 255 });
				}
				else
				{
					ESPColor = { 255, 255, 255, 255 };
					ESPColorU = ImGui::GetColorU32({ 255, 255, 255, 255 });
				}

				if (distance <= EspRenderingDistance || InLobby)
				{
					if (g_boxesp)
					{
						DrawNormalBox(vRootBoneOut.x - (BoxWidth / 2), vHeadBoneOut.y, BoxWidth, BoxHeight, 1.0f, &ESPColor, &ESPColor);
					}

					else if (g_cornerboxesp)
					{
						DrawCorneredBox(vRootBoneOut.x - (BoxWidth / 2), vHeadBoneOut.y, BoxWidth, BoxHeight, ESPColorU, 1.5);
					}

					if (g_esp_distance) 
					{
						char dist[64];
						sprintf_s(dist, "%.fM", distance);
						ImVec2 TextSize = ImGui::CalcTextSize(dist);
						ImGui::GetOverlayDrawList()->AddText(ImVec2(vRootBoneOut.x - 15 - TextSize.x / 2, vRootBoneOut.y - 15 - TextSize.y / 2), ESPColorU, dist);
					}

					if (g_esp_skeleton) 
					{
						using namespace g_functions;

						GetAllBones(EntityList.USkeletalMeshComponent);

						DrawLine(vleftChest.x, vleftChest.y, vrightChest.x, vrightChest.y, &ESPColor, 0.5f);
						DrawLine(vleftChest.x, vleftChest.y, vleftShoulder.x, vleftShoulder.y, &ESPColor, 0.5f);
						DrawLine(vrightChest.x, vrightChest.y, vrightShoulder.x, vrightShoulder.y, &ESPColor, 0.5f);
						DrawLine(vleftShoulder.x, vleftShoulder.y, vleftElbow.x, vleftElbow.y, &ESPColor, 0.5f);
						DrawLine(vrightShoulder.x, vrightShoulder.y, vrightElbow.x, vrightElbow.y, &ESPColor, 0.5f);
						DrawLine(vleftElbow.x, vleftElbow.y, vleftWrist.x, vleftWrist.y, &ESPColor, 0.5f);
						DrawLine(vrightElbow.x, vrightElbow.y, vrightWrist.x, vrightWrist.y, &ESPColor, 0.5f);
						DrawLine(vneck2.x, vneck2.y, vpelvis.x, vpelvis.y, &ESPColor, 0.5f);
						DrawLine(vpelvis.x, vpelvis.y, vleftAss.x, vleftAss.y, &ESPColor, 0.5f);
						DrawLine(vpelvis.x, vpelvis.y, vrightAss.x, vrightAss.y, &ESPColor, 0.5f);
						DrawLine(vleftAss.x, vleftAss.y, vleftKnee.x, vleftKnee.y, &ESPColor, 0.5f);
						DrawLine(vrightAss.x, vrightAss.y, vrightKnee.x, vrightKnee.y, &ESPColor, 0.5f);
						DrawLine(vleftKnee.x, vleftKnee.y, vleftAnkle.x, vleftAnkle.y, &ESPColor, 0.5f);
						DrawLine(vrightKnee.x, vrightKnee.y, vrightAnkle.x, vrightAnkle.y, &ESPColor, 0.5f);
					}

					if (g_3d_box)
					{
						using namespace MeshBones;

						if (vHeadBoneOut.x != 0 || vHeadBoneOut.y != 0 || vHeadBoneOut.z != 0)
						{
							Init3DESP(vHeadBone, vRootBone);

							ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), ESPColorU, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), ESPColorU, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(top3.x, top3.y), ESPColorU, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(top4.x, top4.y), ESPColorU, 0.1f);

							ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), ESPColorU, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(bottom3.x, bottom3.y), ESPColorU, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(bottom4.x, bottom4.y), ESPColorU, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(bottom1.x, bottom1.y), ESPColorU, 0.1f);

							ImGui::GetForegroundDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), ESPColorU, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(top2.x, top2.y), ImVec2(top3.x, top3.y), ESPColorU, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(top3.x, top3.y), ImVec2(top4.x, top4.y), ESPColorU, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(top4.x, top4.y), ImVec2(top1.x, top1.y), ESPColorU, 0.1f);
						}
					}

					if (g_lineesp)
					{
						ImGui::GetOverlayDrawList()->AddLine(
							ImVec2(ScreenCenterX, Height),
							ImVec2(RelativeScreenLocation.x, RelativeScreenLocation.y + 30),
							ESPColorU,
							0.1f
						);
					}
				}

				auto dx = AimbotHeadOut.x - (Width / 2);
				auto dy = AimbotHeadOut.y - (Height / 2);
				auto dist = sqrtf(dx * dx + dy * dy);
				auto isDBNO = (read<char>(g_pid, EntityList.ACurrentActor + 0x6f2) >> 4) & 1;

				if (g_spinbot)
				{
					auto Mesh = read<uint64_t>(g_pid, LocalPawn + 0x2f0);
					static auto Cached = read<Vector3>(g_pid, Mesh + 0x140);

					if (GetAsyncKeyState(VK_RBUTTON)) {
						write<Vector3>(g_pid, Mesh + 0x140, Vector3(1, rand() % 361, 1));
					}
					else write<Vector3>(g_pid, Mesh + 0x140, Cached);
				}

				if (dist < AimbotValue && dist < closestDistance && TeamIndex != LocalTeam && !InLobby)
				{
					if (g_skipknocked)
					{
						if (!isDBNO)
						{
							closestDistance = dist;
							closestPawn = EntityList.ACurrentActor;
						}
					}
					else
					{
						closestDistance = dist;
						closestPawn = EntityList.ACurrentActor;
					}
				}
			}
		}
		doAimbot(closestPawn);
	}
	catch (...) {}
}

#include "menu.h"

void runRenderTick() 
{
	glfwPollEvents();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	RECT rect = { 0 };

	HWND GameWnd = FindWindowA(XorStr("UnrealWindow").c_str(), XorStr("Fortnite  ").c_str());

	HWND hwnd_active = GetForegroundWindow();
	HWND overlay_window = glfwGetWin32Window(g_window);

	if (hwnd_active == GameWnd || hwnd_active == overlay_window) {
		isFortniteFocused = true;
	}
	else {
		isFortniteFocused = true;
	}

	if (isFortniteFocused)
	{
		if (GetWindowRect(GameWnd, &rect))
		{
			Width = rect.right - rect.left;
			Height = rect.bottom - rect.top;
		}

		ScreenCenterX = (Width / 2.0f);
		ScreenCenterY = (Height / 2.0f);

		CheatLoop();
	}

	float radiusx = AimbotValue * (ScreenCenterX / 100);
	float radiusy = AimbotValue * (ScreenCenterY / 100);

	float calcradius = (radiusx + radiusy) / 2;

	if (LocalPawn)
	{
		if (g_fov)
		{
			DrawLine((float)(Width / 2) - AimbotValue, (float)(Height / 2) - AimbotValue, (float)(Width / 2) + AimbotValue, (float)(Height / 2) - AimbotValue, &Col.white, 2);
			DrawLine((float)(Width / 2) - AimbotValue, (float)(Height / 2) + AimbotValue, (float)(Width / 2) + AimbotValue, (float)(Height / 2) + AimbotValue, &Col.white, 2);
			DrawLine((float)(Width / 2) + AimbotValue, (float)(Height / 2) - AimbotValue, (float)(Width / 2) + AimbotValue, (float)(Height / 2) + AimbotValue, &Col.white, 2);
			DrawLine((float)(Width / 2) - AimbotValue, (float)(Height / 2) - AimbotValue, (float)(Width / 2) - AimbotValue, (float)(Height / 2) + AimbotValue, &Col.white, 2);
			DrawLine((float)(Width / 2) - AimbotValue, (float)(Height / 2) - AimbotValue, (float)(Width / 2) + AimbotValue, (float)(Height / 2) - AimbotValue, &Col.white, 3);
			DrawLine((float)(Width / 2) - AimbotValue, (float)(Height / 2) + AimbotValue, (float)(Width / 2) + AimbotValue, (float)(Height / 2) + AimbotValue, &Col.white, 3);
			DrawLine((float)(Width / 2) - AimbotValue, (float)(Height / 2) - AimbotValue, (float)(Width / 2) + AimbotValue, (float)(Height / 2) - AimbotValue, &Col.black_, 1);
			DrawLine((float)(Width / 2) - AimbotValue, (float)(Height / 2) + AimbotValue, (float)(Width / 2) + AimbotValue, (float)(Height / 2) + AimbotValue, &Col.black_, 1);
			DrawLine((float)(Width / 2) + AimbotValue, (float)(Height / 2) - AimbotValue, (float)(Width / 2) + AimbotValue, (float)(Height / 2) + AimbotValue, &Col.black_, 1);
			DrawLine((float)(Width / 2) - AimbotValue, (float)(Height / 2) - AimbotValue, (float)(Width / 2) - AimbotValue, (float)(Height / 2) + AimbotValue, &Col.black_, 1);
		}

		if (g_crossh) {
			DrawLine((float)(Width / 2), (float)(Height / 2) - 8, (float)(Width / 2), (float)(Height / 2) + 8, &Col.white, 1);
			DrawLine((float)(Width / 2) - 8, (float)(Height / 2), (float)(Width / 2) + 8, (float)(Height / 2), &Col.white, 1);
		}

		if (g_circlefov) {
			ImGui::GetOverlayDrawList()->AddCircle(ImVec2(ScreenCenterX, ScreenCenterY), AimbotValue, ImGui::ColorConvertFloat4ToU32(ImVec4(ESPColor.R / 255.0, ESPColor.G / 255.0, ESPColor.B / 255.0, ESPColor.A / 255.0)), 100);
		}
	}
	DrawText1(15, 15, "Gloomy", &Col.red);
	DrawText1(15, 28, "F1 - MENU", &Col.white);

	char dist[64];
	sprintf_s(dist, "FPS %.f\n", ImGui::GetIO().Framerate);
	ImGui::GetOverlayDrawList()->AddText(ImVec2(15, 40), ImColor(cRainbow), dist);

	if (g_overlay_visible)
		background();

	ImGuiIO& io = ImGui::GetIO();

	if (g_overlay_visible) 
		{
			ImGui::Begin(XorStr(" ").c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar); //  | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground
			ImGui::SetWindowSize(XorStr(" ").c_str(), ImVec2(400, 300));

			ImGuiStyle& s = ImGui::GetStyle();
			ImGuiIO& io = ImGui::GetIO();

			s.AntiAliasedFill = true;
			s.AntiAliasedLines = true;
			s.AntiAliasedLinesUseTex = true;

			s.ChildRounding = 0.0f;
			s.FrameBorderSize = 1.0f;
			s.FrameRounding = 0.0f;
			s.PopupRounding = 0.0f;
			s.ScrollbarRounding = 0.0f;
			s.ScrollbarSize = 0.0f;
			s.TabRounding = 0.0f;
			s.WindowRounding = 0.0f;

			decoration();

			static int Menu_Tab = 0; 1; 2; 3; 4;

			ImGui::SetCursorPos({ 17,29 });
			if (ImGui::Button("Aimbot ")) Menu_Tab = 0;
			ImGui::SetCursorPos({ 17,49 });
			if (ImGui::Button("Visuals")) Menu_Tab = 1;

			ImGui::SetCursorPos({ 17,69 });
			if (ImGui::Button("Misc   ")) Menu_Tab = 2;

			ImGui::SetCursorPos({ 17,89 });
			if (ImGui::Button("World  ")) Menu_Tab = 3;

			ImGui::SetCursorPos({ 17,109 });
			if (ImGui::Button("Exploit")) Menu_Tab = 4;

			ImGui::SetCursorPos(ImVec2(240, 10));

			if (Menu_Tab == 0) AimbotTab();
			else if (Menu_Tab == 1) VisualsTab();
			else if (Menu_Tab == 2) MiscTab();
			else if (Menu_Tab == 3) WorldTab();
			else if (Menu_Tab == 4) ExploitsTab();

			ImGui::SetCursorPos({ 17,270 });
			if (ImGui::Button("Unload")) exit(0);
			ImGui::End();
		}

	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(g_window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(g_window);
}

int main() 
{
	InjectMouseInput = reinterpret_cast<InjectMouseInput_t>(GetProcAddress(LoadLibraryA("user32.dll"), "InjectMouseInput"));

	if (GlobalFindAtomA("innit??") == 0)
	{
		system(XorStr("taskkill /F /IM EpicGamesLauncher.exe").c_str());
		system(XorStr("taskkill /F /IM EasyAntiCheatLauncher.exe").c_str());
		system(XorStr("taskkill /F /IM BEService.exe").c_str());
		system(XorStr("taskkill /F /IM BattleEyeLauncher.exe").c_str());
		system(XorStr("taskkill /F /IM FortniteClient-Win64-Shipping.exe").c_str());
		system(XorStr("taskkill /F /IM FortniteLauncher.exe").c_str());

		VulnerableDriver::Init();
		GlobalAddAtomA("innit??");
	}

	HWND Entryhwnd = NULL;

	while (Entryhwnd == NULL)
	{
		printf(XorStr("Run Fortnite\r").c_str());
		Sleep(1);
		Entryhwnd = FindWindowA(XorStr("UnrealWindow").c_str(), XorStr("Fortnite  ").c_str());
		Sleep(1);
	}

	system(XorStr("cls").c_str());

	g_pid = get_fn_processid();

	if (!g_pid) {
		std::cout << XorStr("Could not find Fortnite.\n").c_str();
		system(XorStr("pause").c_str());
		return 1;
	}

	Drive.Init();

	EnumWindows(retreiveFortniteWindow, NULL);
	if (!fortnite_wnd) {
		std::cout << XorStr("Could not find Fortnite.\n");
		system(XorStr("pause").c_str());
		return 1;
	}

	g_base_address = getBaseAddress(g_pid);
	if (!g_base_address) {
		std::cout << XorStr("Could not get base address.\n").c_str();
		system(XorStr("pause").c_str());
		return 1;
	}

	if (!pattern_uworld) pattern_uworld = find_signature(XorStr("\x48\x89\x05\x00\x00\x00\x00\x48\x8B\x4B\x78").c_str(), XorStr("xxx????xxxx").c_str());

	setupWindow();
	if (!g_window) {
		std::cout << XorStr("Could not setup window.\n").c_str();
		system(XorStr("pause").c_str());
		return 1;
	}

	CloseHandle(CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(CacheGame), nullptr, NULL, nullptr));

	while (!glfwWindowShouldClose(g_window))
	{
		handleKeyPresses();
		runRenderTick();
	}cleanupWindow();

	return 0;
}
