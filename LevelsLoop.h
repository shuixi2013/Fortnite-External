#pragma once

void CacheLevels()
{
	if (LocalPawn)
	{
		std::vector<LootEntity> tmpList;
		uintptr_t ItemLevels = read<uintptr_t>(g_pid, GWorld + 0x160);

		for (int i = 0; i < read<DWORD>(g_pid, GWorld + (0x160 + sizeof(PVOID))); ++i) {

			uintptr_t ItemLevel = read<uintptr_t>(g_pid, ItemLevels + (i * sizeof(uintptr_t)));

			for (int i = 0; i < read<DWORD>(g_pid, ItemLevel + (0x98 + sizeof(PVOID))); ++i) {
				uintptr_t ItemsPawns = read<uintptr_t>(g_pid, ItemLevel + 0x98);
				uintptr_t CurrentItemPawn = read<uintptr_t>(g_pid, ItemsPawns + (i * sizeof(uintptr_t)));
				uintptr_t ItemRootComponent = read<uintptr_t>(g_pid, CurrentItemPawn + 0x188);
				Vector3 ItemPosition = read<Vector3>(g_pid, ItemRootComponent + 0x128);
				float ItemDist = LocalPlayerRelativeLocation.Distance(ItemPosition) / 100.f;

				if (ItemDist < LootRenderingDistance) {

					int ItemIndex = read<int>(g_pid, CurrentItemPawn + 0x18);
					auto CurrentItemPawnName = GetNameFromFName(ItemIndex);

					if ((g_loot && strstr(CurrentItemPawnName.c_str(), (XorStr("FortPickupAthena")).c_str())) || strstr(CurrentItemPawnName.c_str(), (XorStr("Tiered_Chest").c_str())) ||
						(g_vehicles && strstr(CurrentItemPawnName.c_str(), XorStr("Vehicl").c_str()) || strstr(CurrentItemPawnName.c_str(), XorStr("Valet_Taxi").c_str()) ||
							strstr(CurrentItemPawnName.c_str(), XorStr("Valet_BigRig").c_str()) || strstr(CurrentItemPawnName.c_str(), XorStr("Valet_BasicTr").c_str()) ||
							strstr(CurrentItemPawnName.c_str(), XorStr("Valet_SportsC").c_str()) || strstr(CurrentItemPawnName.c_str(), XorStr("Valet_BasicC").c_str()) ||
							strstr(CurrentItemPawnName.c_str(), XorStr("Tiered_Ammo").c_str())))
					{
						LootEntity LevelObjects{ };
						LevelObjects.ACurrentItem = CurrentItemPawn;
						LevelObjects.GNames = CurrentItemPawnName;
						tmpList.push_back(LevelObjects);
					}
				}
			}
		}
		LootentityList.clear();
		LootentityList = tmpList;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}