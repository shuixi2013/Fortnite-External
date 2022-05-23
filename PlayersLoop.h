#pragma once

void CacheGame()
{
	while (true)
	{
		std::vector<Playertest> tmpList;

		GWorld = read<uintptr_t>(g_pid, pattern_uworld);
		GameInstance = read<uintptr_t>(g_pid, GWorld + 0x1A8);
		LocalPlayers = read<uintptr_t>(g_pid, GameInstance + 0x38);
		LocalPlayer = read<uintptr_t>(g_pid, LocalPlayers);
		LocalPlayerController = read<uintptr_t>(g_pid, LocalPlayer + 0x30);
		MyHUD = read<uintptr_t>(g_pid, LocalPlayerController + 0x320);
		PlayerCameraManager = read<uint64_t>(g_pid, LocalPlayerController + 0x328);
		LocalPawn = read<uintptr_t>(g_pid, LocalPlayerController + 0x310);
		LocalPawnRootComponent = read<uintptr_t>(g_pid, LocalPawn + 0x188);
		uint64_t LocalPlayerState = read<uint64_t>(g_pid, LocalPawn + 0x290);
		LocalTeam = read<int>(g_pid, LocalPlayerState + 0x1010);

		InLobby = false;
		if (!LocalPawn) InLobby = true;

		uintptr_t PersistentLevel = read<uintptr_t>(g_pid, GWorld + 0x30);
		auto ActorArray = read<DWORD>(g_pid, PersistentLevel + 0xa0);
		auto Actors = read<uintptr_t>(g_pid, PersistentLevel + 0x98);

		for (int i = 0; i < ActorArray; ++i) {
			uintptr_t CurrentItemPawn = read<uintptr_t>(g_pid, Actors + (i * sizeof(uintptr_t)));

			int Index = read<int>(g_pid, CurrentItemPawn + 0x18);
			auto CurrentItemPawnName = GetNameFromFName(Index);

			if (strcmp(CurrentItemPawnName.c_str(), "PlayerPawn_Athena_C") || strstr(CurrentItemPawnName.c_str(), "PlayerPawn"))
			{
				Playertest Actor{ };

				Actor.ACurrentActor = CurrentItemPawn;
				Actor.USkeletalMeshComponent = read<uint64_t>(g_pid, CurrentItemPawn + 0x2f0);
				Actor.GNames = CurrentItemPawnName;
				Actor.USceneComponent = LocalPawnRootComponent;

				tmpList.push_back(Actor);
			}
		}

		CacheLevels();
		PLIST.clear();
		PLIST = tmpList;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
