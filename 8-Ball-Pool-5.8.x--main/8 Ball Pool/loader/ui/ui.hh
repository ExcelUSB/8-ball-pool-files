#pragma once
#include <d3d9.h>
#include "../imgui/imgui.h"

namespace ui {
	void init(LPDIRECT3DDEVICE9);
	void HWID_CHECK();
	void UserBan(char HWID[100]);
	void render();


}

namespace ui {
	inline LPDIRECT3DDEVICE9 dev;
	inline const char* window_title = "Loader base";
}

namespace ui {
	inline ImVec2 screen_res{ 000, 000 };
	inline ImVec2 window_pos{ 0, 0 };
	inline ImVec2 window_size{ 360, 300 };
	inline DWORD  window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
}

extern void loadHack(const char* decryptionKey, int keySize);
extern void loadBetaHack(const char* decryptionKey, int keySize);