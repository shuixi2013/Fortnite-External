#pragma once

bool doAimbot(DWORD_PTR closestPawn)
{
	if (closestPawn != 0)
	{
		if (closestPawn && GetAsyncKeyState(hotkeys::aimkey) or Controller::IsPressingLeftTrigger() && isFortniteFocused)
		{
			if (g_aimbot)
			{
				auto AimbotMesh = read<uint64_t>(g_pid, closestPawn + 0x2f0);
				if (!AimbotMesh)
					return false;

				auto CurrentWeapon = read<uintptr_t>(g_pid, closestPawn + 0x790);
				if (!CurrentWeapon)
					return false;

				Vector3 HeadPosition = g_functions::f_getbonewithIndex(AimbotMesh, select_hitbox());
				if (!IsVec3Valid(HeadPosition))
					return false;

				Vector3 Head = g_functions::ConvertWorld2Screen(HeadPosition);
				if (!IsVec3Valid(HeadPosition))
					return false;

				if (Head.x != 0 || Head.y != 0 || Head.z != 0)
				{
					if ((GetDistance(Head.x, Head.y, Head.z, Width / 2, Height / 2) <= AimbotValue))
					{
						if (g_mouse_aim)
						{
							aimbot(Head.x, Head.y);
						}

						if (g_gun_tracers)
						{
							if (GetAsyncKeyState(VK_LBUTTON))
							{
								write<float>(g_pid, CurrentWeapon + 0x64, 99);
							}
							else
							{
								write<float>(g_pid, CurrentWeapon + 0x64, 1);
							}
						}
					}
				}
			}
		}
		else
		{
			closestDistance = FLT_MAX;
			closestPawn = NULL;
		}
	}
	return true;
}