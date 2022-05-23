#pragma once

void LevelEntEsp(LootEntity LEntityList)
{
	if (LocalPawn)
	{
		uintptr_t ItemRootComponent = read<uintptr_t>(g_pid, LEntityList.ACurrentItem + 0x188);
		Vector3 ItemPosition = read<Vector3>(g_pid, ItemRootComponent + 0x128);
		float ItemDist = LocalPlayerRelativeLocation.Distance(ItemPosition) / 100.f;
		std::string null = ("");

		auto IsSearched = read<BYTE>(g_pid, (uintptr_t)LEntityList.ACurrentItem + 0xf41);
		if (!IsSearched >> 7 & 1)
		{
			if (strstr(LEntityList.GNames.c_str(), ("Tiered_Chest")) && g_chests)
			{
				if (ItemDist < LootRenderingDistance) {
					Vector3 ChestPosition;
					ChestPosition = g_functions::ConvertWorld2Screen(ItemPosition);
					std::string Text = null + ("Chest [") + std::to_string((int)ItemDist) + ("m]");
					DrawString(14, ChestPosition.x, ChestPosition.y, &Col.yellow, true, true, Text.c_str());
				}
			}

			else if ((g_vehicles && (strstr(LEntityList.GNames.c_str(), XorStr("Vehicl").c_str()) || strstr(LEntityList.GNames.c_str(), XorStr("Valet_Taxi").c_str()) || strstr(LEntityList.GNames.c_str(), XorStr("Valet_BigRig").c_str()) || strstr(LEntityList.GNames.c_str(), XorStr("Valet_BasicTr").c_str()) || strstr(LEntityList.GNames.c_str(), XorStr("Valet_SportsC").c_str()) || strstr(LEntityList.GNames.c_str(), XorStr("Valet_BasicC").c_str()))))
			{
				if (ItemDist < LootRenderingDistance) {
					Vector3 VehiclePosition = g_functions::ConvertWorld2Screen(ItemPosition);
					std::string Text = null + ("Vehicle [") + std::to_string((int)ItemDist) + ("m]");
					DrawString(14, VehiclePosition.x, VehiclePosition.y, &Col.red, true, true, Text.c_str());
				}
			}

			else if (strstr(LEntityList.GNames.c_str(), ("AthenaSupplyDrop_C")) && g_loot)
			{
				if (ItemDist < LootRenderingDistance) {
					Vector3 ChestPosition;
					ChestPosition = g_functions::ConvertWorld2Screen(ItemPosition);

					std::string Text = null + ("Supply Drop [") + std::to_string((int)ItemDist) + ("m]");
					DrawString(14, ChestPosition.x, ChestPosition.y, &Col.blue, true, true, Text.c_str());

				}
			}

			else if (strstr(LEntityList.GNames.c_str(), ("Tiered_Ammo")) && g_ammo)
			{
				if (ItemDist < LootRenderingDistance) {
					Vector3 ChestPosition;
					ChestPosition = g_functions::ConvertWorld2Screen(ItemPosition);
					std::string Text = null + ("Ammo Box [") + std::to_string((int)ItemDist) + ("m]");
					DrawString(14, ChestPosition.x, ChestPosition.y, &Col.white, true, true, Text.c_str());

				}
			}

			else if (g_loot && strstr(LEntityList.GNames.c_str(), ("FortPickupAthena")))
			{
				if (ItemDist < LootRenderingDistance) {

					auto definition = read<uint64_t>(g_pid, LEntityList.ACurrentItem + 0x2f8 + 0x18);
					BYTE tier = read<BYTE>(g_pid, definition + 0x74);

					RGBA Color, RGBAColor;
					Vector3 ChestPosition = g_functions::ConvertWorld2Screen(ItemPosition);

					if (g_loot)
					{
						auto DisplayName = read<uint64_t>(g_pid, definition + 0x90);
						auto WeaponLength = read<uint32_t>(g_pid, DisplayName + 0x38);
						wchar_t* WeaponName = new wchar_t[uint64_t(WeaponLength) + 1];

						Drive.ReadPtr(g_pid, (ULONG64)read<PVOID>(g_pid, DisplayName + 0x30), WeaponName, WeaponLength * sizeof(wchar_t));

						std::string Text = wchar_to_char(WeaponName);
						std::string wtf2 = Text + " [" + std::to_string((int)ItemDist) + ("m]");
						if (tier == 2)
						{
							Color = Col.green;
						}
						else if ((tier == 3))
						{
							Color = Col.blue;
						}
						else if ((tier == 4))
						{
							Color = Col.purple;
						}
						else if ((tier == 5))
						{
							Color = Col.yellow;
						}
						else if ((tier == 6))
						{
							Color = Col.yellow;
						}
						else if ((tier == 0) || (tier == 1))
						{
							Color = Col.white;
						}

						DrawString(14, ChestPosition.x, ChestPosition.y, &Color, true, true, wtf2.c_str());
					}
				}
			}
		}
	}
}