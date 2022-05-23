#pragma once

namespace Globals
{
	uintptr_t GWorld,
		LocalPlayerController,
		MyHUD,
		GameInstance,
		LocalPlayers,
		PersistentLevel,
		LocalPlayer = 0,
		LocalPawn = 0,
		LocalPawnRootComponent = 0;

	uint64_t g_base_address,
		pattern_uworld,
		pattern_gnames,
		PlayerCameraManager,
		LocalPlayerState;

	bool InLobby = false, isVis;

	int g_width,
		g_height,
		g_pid,
		selectedbone = 0,
		hitbox = 0;

	HWND fortnite_wnd;

	float Width = GetSystemMetrics(SM_CXSCREEN),
		Height = GetSystemMetrics(SM_CYSCREEN),
		ScreenCenterX = 0.0f,
		ScreenCenterY = 0.0f,
		AimbotValue = 150.0f,
		AimbotSmoothnessValue = 2.0f,
		closestDistance = FLT_MAX;

	static float FOVChangerValue = 100.0f;

	int Distance, Hitbox = 0,
		EspRenderingDistance = 280,
		AimbotDistance = 280,
		BoxSize = 2.0f,
		LootRenderingDistance = 40,
		LocalTeam;

	DWORD_PTR closestPawn = NULL;

	Vector3 LocalPlayerRelativeLocation = Vector3(0.0f, 0.0f, 0.0f);

	namespace Settings
	{
		bool g_overlay_visible = false,
			g_fovchanger = false,
			g_cornerboxesp = false,
			g_esp_distance = true,
			g_esp_skeleton = false,
			g_3d_box = false,
			g_aimbot = true,
			g_skipknocked = true,
			g_lineesp = false,
			g_boxesp = false,
			g_fov = false,
			g_circlefov = true,
			g_crossh = true,
			g_chests = false,
			g_vehicles = false,
			g_ammo = false,
			g_ammos = false,
			g_loot = false,
			g_curweaponesp = true,
			g_consumables = false,
			g_mouse_aim = false,
			g_utils = false,
			g_controller = false,
			g_gun_tracers = false,
			g_spinbot = false,
			g_platform_esp = false;
	}

	namespace Camera
	{
		Vector3 Rotation, 
		Location;
		float FovAngle;
	}
};

bool Head = true, Neck, Chest;