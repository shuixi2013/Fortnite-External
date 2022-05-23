#pragma once

#include "decryption.asm"

using namespace Globals;
using namespace Globals::Camera;


namespace g_functions
{
	Vector3 ConvertWorld2Screen(Vector3 WorldLocation) {

		Rotation.z = 0;

		if (Rotation.y < 0)
			Rotation.y += 360;

		D3DMATRIX matrix = Matrix(Rotation);

		auto dist = WorldLocation - Location;

		auto transform = Vector3(
			dist.Dot(Vector3(
				matrix.m[1][0], matrix.m[1][1], matrix.m[1][2]
			)),
			dist.Dot(Vector3(
				matrix.m[2][0], matrix.m[2][1], matrix.m[2][2]
			)),
			dist.Dot(Vector3(
				matrix.m[0][0], matrix.m[0][1], matrix.m[0][2]
			))
		);

		if (transform.z < 1.f)
			transform.z = 1.f;

		float ScreenCenterX = Globals::Width / 2;
		float ScreenCenterY = Globals::Height / 2;

		Vector3 Screenlocation;
		Screenlocation.x = ScreenCenterX + transform.x * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / transform.z;
		Screenlocation.y = ScreenCenterY - transform.y * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / transform.z;

		return Screenlocation;
	}

	FTransform f_boneIndex(uint64_t mesh, int index)
	{
		uint64_t bonearray = read<uint64_t>(g_pid, mesh + 0x590);
		if (!bonearray)
			bonearray = read<DWORD_PTR>(g_pid, mesh + 0x5a0);

		return read<FTransform>(g_pid, bonearray + (index * 0x60));
	}

	Vector3 f_getbonewithIndex(DWORD_PTR mesh, int id)
	{
		FTransform bone = f_boneIndex(mesh, id);
		FTransform ComponentToWorld = read<FTransform>(g_pid, mesh + 0x240);

		D3DMATRIX Matrix;
		Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

		return Vector3(Matrix._41, Matrix._42, Matrix._43);
	}
	void GetAllBones(uint64_t USkeletalMeshComponent);

	wchar_t* ReadFString(uintptr_t Address);
	wchar_t* ReadFText(uintptr_t Address);
};

namespace MeshBones
{
	Vector3 neck2,
		vneck2,
		neck,
		vneck,
		rightChest,
		vrightChest,
		leftChest,
		vleftChest,
		leftShoulder,
		vleftShoulder,
		rightShoulder,
		vrightShoulder,
		leftElbow,
		vleftElbow,
		rightElbow,
		vrightElbow,
		leftWrist,
		vleftWrist,
		rightWrist,
		vrightWrist,
		pelvis,
		vpelvis,
		leftAss,
		vleftAss,
		rightAss,
		vrightAss,
		leftKnee,
		vleftKnee,
		rightKnee,
		vrightKnee,
		leftAnkle,
		vleftAnkle,
		rightAnkle,
		vrightAnkle,
		bottom1,
		bottom2,
		bottom3,
		bottom4,
		top1,
		top2,
		top3,
		top4;
};

using namespace MeshBones;

bool isVisible(DWORD_PTR mesh)
{
	if (!mesh)
		return false;
	float fLastSubmitTime = read<float>(g_pid, mesh + 0x330);
	float fLastRenderTimeOnScreen = read<float>(g_pid, mesh + 0x338);

	const float fVisionTick = 0.06f;
	bool bVisible = fLastRenderTimeOnScreen + fVisionTick >= fLastSubmitTime;
	return bVisible;
};

bool IsVec3Valid(Vector3 vec3)
{
	return !(vec3.x == 0 && vec3.y == 0 && vec3.z == 0);
};

wchar_t* g_functions::ReadFString(uintptr_t Address)
{
	uint64_t StringData = read<uint64_t>(g_pid, Address);
	uint32_t StringLength = read<uint32_t>(g_pid, Address + 0x8);
	wchar_t* OutString = new wchar_t[StringLength + 1];
	Drive.ReadPtr(g_pid, StringData, OutString, StringLength * sizeof(wchar_t));
	return OutString;
}

wchar_t* g_functions::ReadFText(uintptr_t Address)
{
	uint32_t FTextLength = read<uint32_t>(g_pid, Address + 0x38);
	wchar_t* OutText = new wchar_t[uint64_t(FTextLength) + 1];
	Drive.ReadPtr(g_pid, (ULONG64)read<PVOID>(g_pid, Address + 0x30), OutText, FTextLength * sizeof(wchar_t));
	return OutText;
}

void g_functions::GetAllBones(uint64_t USkeletalMeshComponent)
{
	neck2 = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 98);
	vneck2 = g_functions::ConvertWorld2Screen(neck2);
	neck = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 66);
	vneck = g_functions::ConvertWorld2Screen(neck);
	rightChest = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 8);
	vrightChest = g_functions::ConvertWorld2Screen(rightChest);
	leftChest = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 37);
	vleftChest = g_functions::ConvertWorld2Screen(leftChest);
	leftShoulder = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 38);
	vleftShoulder = g_functions::ConvertWorld2Screen(leftShoulder);
	rightShoulder = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 9);
	vrightShoulder = g_functions::ConvertWorld2Screen(rightShoulder);
	leftElbow = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 94);
	vleftElbow = g_functions::ConvertWorld2Screen(leftElbow);
	rightElbow = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 10);
	vrightElbow = g_functions::ConvertWorld2Screen(rightElbow);
	leftWrist = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 62);
	vleftWrist = g_functions::ConvertWorld2Screen(leftWrist);
	rightWrist = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 33);
	vrightWrist = g_functions::ConvertWorld2Screen(rightWrist);
	pelvis = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 2);
	vpelvis = g_functions::ConvertWorld2Screen(pelvis);
	leftAss = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 76);
	vleftAss = g_functions::ConvertWorld2Screen(leftAss);
	rightAss = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 69);
	vrightAss = g_functions::ConvertWorld2Screen(rightAss);
	leftKnee = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 77);
	vleftKnee = g_functions::ConvertWorld2Screen(leftKnee);
	rightKnee = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 70);
	vrightKnee = g_functions::ConvertWorld2Screen(rightKnee);
	leftAnkle = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 78);
	vleftAnkle = g_functions::ConvertWorld2Screen(leftAnkle);
	rightAnkle = g_functions::f_getbonewithIndex(USkeletalMeshComponent, 71);
	vrightAnkle = g_functions::ConvertWorld2Screen(rightAnkle);
};

void Init3DESP(Vector3 vHeadBone, Vector3 vRootBone)
{
	bottom1 = g_functions::ConvertWorld2Screen(Vector3(vRootBone.x + 40, vRootBone.y - 40, vRootBone.z));
	bottom2 = g_functions::ConvertWorld2Screen(Vector3(vRootBone.x - 40, vRootBone.y - 40, vRootBone.z));
	bottom3 = g_functions::ConvertWorld2Screen(Vector3(vRootBone.x - 40, vRootBone.y + 40, vRootBone.z));
	bottom4 = g_functions::ConvertWorld2Screen(Vector3(vRootBone.x + 40, vRootBone.y + 40, vRootBone.z));
	top1 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x + 40, vHeadBone.y - 40, vHeadBone.z + 15));
	top2 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x - 40, vHeadBone.y - 40, vHeadBone.z + 15));
	top3 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x - 40, vHeadBone.y + 40, vHeadBone.z + 15));
	top4 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x + 40, vHeadBone.y + 40, vHeadBone.z + 15));
};

void DrawBoxHead(uint64_t USkeletalMeshComponent, Vector3 vHeadBone)
{
	Vector3 bottom1 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x + 10, vHeadBone.y - 10, vHeadBone.z));
	Vector3 bottom2 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x - 10, vHeadBone.y - 10, vHeadBone.z));
	Vector3 bottom3 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x - 10, vHeadBone.y + 10, vHeadBone.z));
	Vector3 bottom4 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x + 10, vHeadBone.y + 10, vHeadBone.z));

	Vector3 top1 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x + 10, vHeadBone.y - 10, vHeadBone.z + 15));
	Vector3 top2 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x - 10, vHeadBone.y - 10, vHeadBone.z + 15));
	Vector3 top3 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x - 10, vHeadBone.y + 10, vHeadBone.z + 15));
	Vector3 top4 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x + 10, vHeadBone.y + 10, vHeadBone.z + 15));

	ImU32 Col = ImGui::GetColorU32({ 255, 215, 0, 255 });

	ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), Col, 0.1f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), Col, 0.1f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(top3.x, top3.y), Col, 0.1f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(top4.x, top4.y), Col, 0.1f);

	ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), Col, 0.1f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(bottom3.x, bottom3.y), Col, 0.1f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(bottom4.x, bottom4.y), Col, 0.1f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(bottom1.x, bottom1.y), Col, 0.1f);

	ImGui::GetForegroundDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), Col, 0.1f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(top2.x, top2.y), ImVec2(top3.x, top3.y), Col, 0.1f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(top3.x, top3.y), ImVec2(top4.x, top4.y), Col, 0.1f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(top4.x, top4.y), ImVec2(top1.x, top1.y), Col, 0.1f);
	RGBA Skellll = { 255, 255, 255, 255 };
	DrawLine(vneck2.x, vneck2.y, vpelvis.x, vpelvis.y, &Skellll, 0.5f);
};