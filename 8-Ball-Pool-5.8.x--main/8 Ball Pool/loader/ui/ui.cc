#define CURL_STATICLIB

#include "ui.hh"
#include "../globals.hh"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include <tchar.h>
#include <Windows.h> 
#include "../icons.h"
#include "../iconcpp.h"
#include "curl.h"
#include "../SecureStrings.h"
#include <iostream>
#include <fstream>
#include <string>

#define _WIN32_WINNT 0x0400

#include <windows.h>
#include <stdio.h>


#pragma comment (lib, "libcurl_a.lib")
#pragma comment (lib, "Normaliz.lib")
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Wldap32.lib")
#pragma comment (lib, "Crypt32.lib" )
#pragma comment (lib, "advapi32.lib")

using namespace std;

#include <nlohmann/json.hpp>
#include <array>
#include <intrin.h>
#include <mutex>
#include "../md5.h"

#define CONFIG_NAME SString("userinfo.json")
#define USER_AGENT_NAME "Easy-Victory Private Key"
#define VERSION "2.0"

template <typename T>
inline bool loadVarible(const char* varibleName, T& data, const nlohmann::json& j)
{
	const auto& varibleData = j.find(varibleName);
	if (varibleData == j.end())
		return false;

	data = (*varibleData).get<T>();

	return true;
}

void getUserCredentials()
{
	nlohmann::json j;
	std::string    username;
	std::string    password;
	std::ifstream  inputFile = std::ifstream(CONFIG_NAME.Decrypt());

	if (!inputFile.good())
		return;

	try
	{
		inputFile >> j;
	}
	catch (...)
	{
		goto endLabel;
	}

	if (j.empty())
		goto endLabel;

	if (loadVarible(SString("username").Decrypt(), username, j))
		strncpy_s(globals.user_name, username.c_str(), sizeof(globals.user_name));
	if (loadVarible(SString("password").Decrypt(), password, j))
		strncpy_s(globals.pass_word, password.c_str(), sizeof(globals.pass_word));

endLabel:
	inputFile.close();
}

void saveUserCredentials()
{
	int            fileAttributes;
	nlohmann::json j;
	std::string    username = globals.user_name;
	std::string    password = globals.pass_word;
	std::string    _username = SString("username").Decrypt();
	std::string    _password = SString("password").Decrypt();

	j.emplace(_username, username);
	j.emplace(_password, password);

	std::ofstream o(CONFIG_NAME.Decrypt());
	o << std::setw(4) << j << std::endl;
}

static std::mutex curlMutex;

struct ResponseBuffer
{
	ResponseBuffer()
	{
		size = 0;
		data[0] = '\0';
	}

	char   data[4096];
	int    size;
};

static size_t writeToBuffer(void* buffer, size_t size, size_t nmemb, ResponseBuffer* param)
{
	param->size = size * nmemb;
	memcpy(param->data, buffer, param->size);

	return param->size;
}

std::string GetProcessorId()
{
	std::array<int, 4> cpuInfo;
	std::ostringstream buffer;

	__cpuid(cpuInfo.data(), 1);
	buffer << std::uppercase << std::hex << std::setfill('0')
		<< std::setw(8) << cpuInfo.at(3)
		<< std::setw(8) << cpuInfo.at(0);
	return buffer.str();
}

std::string getFirstHddSerialNumber()
{
	HANDLE h = CreateFileW(L"\\\\.\\PhysicalDrive0", 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (h == INVALID_HANDLE_VALUE)
		return {};

	std::unique_ptr<std::remove_pointer<HANDLE>::type, void(*)(HANDLE)> hDevice{ h, [](HANDLE handle) {CloseHandle(handle); } };

	STORAGE_PROPERTY_QUERY storagePropertyQuery{};
	storagePropertyQuery.PropertyId = StorageDeviceProperty;
	storagePropertyQuery.QueryType = PropertyStandardQuery;

	STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader{};
	DWORD dwBytesReturned = 0;
	if (!DeviceIoControl(hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		&storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER), &dwBytesReturned, NULL))
		return {};

	const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
	std::unique_ptr<BYTE[]> pOutBuffer{ new BYTE[dwOutBufferSize]{} };
	if (!DeviceIoControl(hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		pOutBuffer.get(), dwOutBufferSize, &dwBytesReturned, NULL))
		return {};

	STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(pOutBuffer.get());
	const DWORD dwSerialNumberOffset = pDeviceDescriptor->SerialNumberOffset;
	if (dwSerialNumberOffset == 0)
		return {};

	const char* serialNumber = reinterpret_cast<const char*>(pOutBuffer.get() + dwSerialNumberOffset);

	return serialNumber;
}

typedef UINT(WINAPI* WAIT_PROC)(HANDLE, DWORD); // WaitForSingleObject
typedef BOOL(WINAPI* CLOSE_PROC)(HANDLE);       // CloseHandle
typedef BOOL(WINAPI* DELETE_PROC)(LPCTSTR);     // DeleteFile
typedef VOID(WINAPI* EXIT_PROC)(DWORD);         // ExitProcess
typedef DWORD(WINAPI* REMOTETHREAD)(LPVOID);     // Our remote code

typedef struct
{
	WAIT_PROC   fnWaitForSingleObject;
	CLOSE_PROC  fnCloseHandle;
	DELETE_PROC fnDeleteFile;
	EXIT_PROC   fnExitProcess;
	HANDLE      hProcess;
	TCHAR       szFileName[MAX_PATH];
} INJECT;

#pragma check_stack(off)
DWORD WINAPI RemoteThread(INJECT* remote)
{
	remote->fnWaitForSingleObject(remote->hProcess, INFINITE);
	remote->fnCloseHandle(remote->hProcess);
	remote->fnDeleteFile(remote->szFileName);
	remote->fnExitProcess(0);
	return 0;
}
#pragma check_stack

PVOID GetFunctionAddr(PVOID func)
{
#ifdef _DEBUG
	// get address of function from the JMP <relative> instruction
	DWORD* offset = (BYTE*)func + 1;
	return (PVOID)(*offset + (BYTE*)func + 5);
#else
	return func;
#endif
}

extern "C" __declspec(dllexport) BOOL __cdecl selfDelete()
{
	BYTE*               code;
	DWORD               dwThreadId;
	INJECT*             remote, local;
	HMODULE             kernel32;
	HANDLE              hThread, hCurProc, hRemoteProcess = NULL;
	STARTUPINFO         si = { sizeof(si) };
	PROCESS_INFORMATION pi;

	if (CreateProcess(nullptr, (LPSTR)"explorer.exe", 0, 0, FALSE, (CREATE_SUSPENDED | CREATE_NO_WINDOW | IDLE_PRIORITY_CLASS), 0, 0, &si, &pi)) {
		CloseHandle(pi.hThread);
		hRemoteProcess = pi.hProcess;
	}

	if (hRemoteProcess == NULL)
		return FALSE;

	code = reinterpret_cast<BYTE*>(VirtualAllocEx(hRemoteProcess, 0, sizeof(INJECT) + 128, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
	if (code == nullptr) {
		CloseHandle(hRemoteProcess);
		return FALSE;
	}

	remote = reinterpret_cast<INJECT*>(code + 128);

	kernel32 = GetModuleHandle("kernel32.dll");
	local.fnWaitForSingleObject = (WAIT_PROC)GetProcAddress(kernel32, "WaitForSingleObject");
	local.fnCloseHandle = (CLOSE_PROC)GetProcAddress(kernel32, "CloseHandle");
	local.fnExitProcess = (EXIT_PROC)GetProcAddress(kernel32, "ExitProcess");
	local.fnDeleteFile = (DELETE_PROC)GetProcAddress(kernel32, "DeleteFileA");

	hCurProc = GetCurrentProcess();
	DuplicateHandle(hCurProc, hCurProc, hRemoteProcess, &local.hProcess, 0, FALSE, DUPLICATE_SAME_ACCESS);

	GetModuleFileName(nullptr, local.szFileName, MAX_PATH);

	WriteProcessMemory(hRemoteProcess, code, GetFunctionAddr(RemoteThread), 128, 0);
	WriteProcessMemory(hRemoteProcess, remote, &local, sizeof(local), 0);

	dwThreadId = 0;
	hThread = CreateRemoteThread(hRemoteProcess, NULL, 0, (REMOTETHREAD)code, remote, 0, &dwThreadId);
	if (hThread != nullptr)
		CloseHandle(hThread);

	return TRUE;
}

std::string getUserHWID()
{
	SYSTEM_INFO     systemInfo;
	std::string     result, cpuID, diskID, guid;
	HW_PROFILE_INFO hwProfileInfo;

	GetCurrentHwProfile(&hwProfileInfo);
	guid   = hwProfileInfo.szHwProfileGuid;
	cpuID  = GetProcessorId();
	diskID = getFirstHddSerialNumber();

	GetNativeSystemInfo(&systemInfo);
	result = std::to_string(systemInfo.dwProcessorType);
	result += std::to_string(systemInfo.dwNumberOfProcessors);
	result += cpuID;
	result += diskID;
	result += guid;

	return md5(result);
}

extern "C" __declspec(dllexport) void requestFileData(const char* fileName, ResponseBuffer * buffer)
{
	CURL*       curl;
	std::string url;
	std::string username;
	std::string password;
	std::string hwid;

	buffer->data[0] = '\0';
	curlMutex.lock();
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl) {
		hwid     = getUserHWID();
		username = globals.user_name;
		password = globals.pass_word;
		url = SString("https://beta.aaaaaa.me/api/test.php").Decrypt();
		url += SString("?username=").Decrypt() + username;
		url += SString("&password=").Decrypt() + password;
		url += SString("&hwid=").Decrypt() + hwid;
		url += SString("&file=").Decrypt();
		url += fileName;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, SString(USER_AGENT_NAME).Decrypt());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToBuffer);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	curlMutex.unlock();
}

extern "C" __declspec(dllexport) void makeRequest(const char* filename, std::string & result)
{
	CURL*       curl;
	std::string url;
	std::string username;
	std::string password;
	std::string hwid;

	result = "";
	curlMutex.lock();
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl) {
		hwid     = getUserHWID();
		username = globals.user_name;
		password = globals.pass_word;
		url = SString("https://beta.aaaaaa.me/api/").Decrypt();
		url += filename;
		url += SString("?username=").Decrypt() + username;
		url += SString("&password=").Decrypt() + password;
		url += SString("&hwid=").Decrypt() + hwid;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, SString(USER_AGENT_NAME).Decrypt());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	curlMutex.unlock();
}

void ui::UserBan(char HWID[100])
{
	CURL* curl;

	curlMutex.lock();
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl)
	{
		std::string result;
		std::string HWID2;
		HWID2 = HWID;
		std::string url = SString("https://beta.aaaaaa.me/api/debug-ban.php?hwid=").Decrypt() + HWID2;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, SString(USER_AGENT_NAME).Decrypt());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	curlMutex.unlock();
}

void Role(string username)
{
	std::string result;
	CURL* curl;

	curlMutex.lock();
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl) {
		std::string url = SString("https://beta.aaaaaa.me/api/role.php?username=").Decrypt() + username;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, SString(USER_AGENT_NAME).Decrypt());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	User::role = atoi(result.c_str());
	curlMutex.unlock();
}

void TimeLeft(std::string username)
{
	CURL*       curl;
	std::string result;
	std::string url;

	curlMutex.lock();
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl) {
		url = SString("https://beta.aaaaaa.me/api/time-left.php?username=").Decrypt() + username;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, SString(USER_AGENT_NAME).Decrypt());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
		curl_easy_perform(curl);
		globals.time_left = result;
		if (result == SString("0").Decrypt()) {
			MessageBox(0, SString("Your subscription has been finished, Please renew it!").Decrypt(), SString("Subscription Expired!").Decrypt(), MB_OK);
		}

		curl_easy_cleanup(curl);
	}

	curlMutex.unlock();
}

std::string userHWIDCheck()
{
	std::string username;
	std::string password;
	username = globals.user_name;
	password = globals.pass_word;
	std::string result;
	CURL* curl;

	curlMutex.lock();
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl) {
		std::string url = SString("https://beta.aaaaaa.me/api/hwid-check.php?username=").Decrypt() + username + SString("&password=").Decrypt() + password + SString("&hwid=").Decrypt() + getUserHWID();
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, SString(USER_AGENT_NAME).Decrypt());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	curlMutex.unlock();

	return result;
}

std::string getDecryptionKey()
{
	std::string username;
	std::string password;
	std::string key = "", url;
	CURL*       curl;

	curlMutex.lock();
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl) {
		username = globals.user_name;
		password = globals.pass_word;
		url = SString("https://beta.aaaaaa.me/api/check-modules/motherboardcheck.php?username=").Decrypt() + username;
		url += SString("&password=").Decrypt() + password;
		url += SString("&hwid=").Decrypt() + getUserHWID();
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, SString(USER_AGENT_NAME).Decrypt());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &key);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	curlMutex.unlock();

	return key;
}

void ui::render() 
{
	std::string decryptionKey;

	if (!globals.active) 
		return;

	ImGui::SetNextWindowPos(ImVec2(window_pos.x, window_pos.y), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(window_size.x, window_size.y));
	ImGui::SetNextWindowBgAlpha(1.0f);

	ImGui::Begin(window_title, &globals.active, window_flags);
	{
		if (Settings::Tab == 0) {
			ui::window_title = "Easy Victory - Login Panel";
			ImGui::Button(ICON_FA_BOOK, { 80.f,30.f });
			ImGui::SetCursorPos({ 48.f,114.f });
			ImGui::PushItemWidth(245.000000);
			ImGui::InputText(" ", globals.user_name, IM_ARRAYSIZE(globals.user_name));
			ImGui::PopItemWidth();
			ImGui::SetCursorPos({ 48.f,158.f });
			ImGui::PushItemWidth(245.000000);
			ImGui::InputText("  ", globals.pass_word, IM_ARRAYSIZE(globals.pass_word), ImGuiInputTextFlags_Password);
			ImGui::PopItemWidth();
			ImGui::SetCursorPos({ 48.f,185.f });
			if (ImGui::Button(SString("Login").Decrypt(), { 80.f,30.f })) {
				std::string username;
				std::string password;
				username = globals.user_name;
				password = globals.pass_word;
				std::string result;
				CURL*       curl;
				CURLcode    res;

				curlMutex.lock();
				curl_global_init(CURL_GLOBAL_DEFAULT);
				curl = curl_easy_init();
				if (curl) {
					std::string url = SString("https://beta.aaaaaa.me/api/login.php?username=").Decrypt() + username + SString("&password=").Decrypt() + password;
					curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
					curl_easy_setopt(curl, CURLOPT_USERAGENT, SString(USER_AGENT_NAME).Decrypt());
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
					curl_easy_perform(curl);
					curl_easy_cleanup(curl);
				}

				curlMutex.unlock();

				if (result == SString("incorrect").Decrypt())
				{
					MessageBox(0, SString("Username/Password are incorrect!").Decrypt(), SString("INCORRECT!").Decrypt(), MB_OK);
				}
				else if (result == SString("valid").Decrypt())
				{
					saveUserCredentials();
					TimeLeft(username);
					if (globals.time_left == SString("0").Decrypt())
					{
						Settings::Tab = 3;
					}
					else
					{
						Role(username);
						if (User::role == 0)
						{
							MessageBox(0, "This account has been permentaly banned!", "BANNED!", MB_OK);
						}
						else
						{

							Settings::Tab = 2;
						}
					}

				}
				else
				{
					MessageBoxA(0, SString("Please contact us at the discord!").Decrypt(), SString("ERROR").Decrypt(), MB_OK);
					MessageBoxA(0, result.c_str(), SString("ERROR").Decrypt(), MB_OK);
				}

			}
			ImGui::SetCursorPos({ 48.f,98.f });
			ImGui::PushItemWidth(56.000000);
			ImGui::Text(SString("Username").Decrypt());
			ImGui::PopItemWidth();
			ImGui::SetCursorPos({ 48.f,143.f });
			ImGui::PushItemWidth(56.000000);
			ImGui::Text(SString("Password").Decrypt());
			ImGui::PopItemWidth();
			ImGui::SetCursorPos({ 212.f,185.f });
			if (ImGui::Button(SString("Register").Decrypt(), { 80.f,30.f }))
			{
				Settings::Tab = 1;
			}
		}

		else if (Settings::Tab == 1)
		{
			ImGui::InputText(SString("Username").Decrypt(), globals.user_name, IM_ARRAYSIZE(globals.user_name));
			ImGui::InputText(SString("Password").Decrypt(), globals.pass_word, IM_ARRAYSIZE(globals.pass_word), ImGuiInputTextFlags_Password);
			ImGui::InputText(SString("Key").Decrypt(), globals.key, IM_ARRAYSIZE(globals.key));
			if (ImGui::Button(SString("Register").Decrypt()))
			{
				std::string username2;
				std::string password2;
				std::string key;
				username2 = globals.user_name;
				password2 = globals.pass_word;
				key = globals.key;
				std::string result;
				CURL* curl;

				curlMutex.lock();
				curl_global_init(CURL_GLOBAL_DEFAULT);
				curl = curl_easy_init();
				if (curl) {
					std::string url = SString("https://beta.aaaaaa.me/api/register.php?username=").Decrypt() + username2 + SString("&password=").Decrypt() + password2 + SString("&key=").Decrypt() + key + "&hwid=" + getUserHWID();
					curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
					curl_easy_setopt(curl, CURLOPT_USERAGENT, SString(USER_AGENT_NAME).Decrypt());
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
					curl_easy_perform(curl);
					curl_easy_cleanup(curl);
				}

				curlMutex.unlock();

				if (result == SString("").Decrypt()) {
					globals.user_name == "";
					globals.pass_word == "";
					Settings::Tab = 0;

				}
				else if (result == SString("found").Decrypt())
				{
					MessageBoxA(0, SString("This username is already registered, Please use another one!").Decrypt(), SString("Already exists!").Decrypt(), MB_OK);
				}
				else if (result == SString("invalid").Decrypt())
				{
					MessageBoxA(0, SString("This key is invalid!, Please recheck your key").Decrypt(), SString("ERROR!").Decrypt(), MB_OK);
				}
				else
				{
					MessageBoxA(0, result.c_str(), SString("ERROR!").Decrypt(), MB_OK);
					MessageBoxA(0, SString("Please contact us at our discord server!").Decrypt(), SString("ERROR!").Decrypt(), MB_OK);

				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Back"))
				Settings::Tab = 0;
		}
		else if (Settings::Tab == 2)
		{
			ui::window_size = { 700, 400 };
			ui::window_title = "Easy Victory - Main";

			if (User::role == 1)
			{
				ImGui::SetCursorPos({ 5.f,33.f });
				if (ImGui::Button(SString("Load Premium Version").Decrypt(), ImVec2(240.f, 0.f))) {
					if (userHWIDCheck() == SString("correct").Decrypt()) {
						decryptionKey = getDecryptionKey();
						loadHack(decryptionKey.c_str(), 64);
					}
				}
			}
			else if (User::role == 2)
			{
				ImGui::SetCursorPos({ 5.f,33.f });
				if (ImGui::Button(SString("Load Premium Version").Decrypt(), ImVec2(240.f, 0.f))) {
					if (userHWIDCheck() == SString("correct").Decrypt()) {
						decryptionKey = getDecryptionKey();
						loadHack(decryptionKey.c_str(), 64);
					}
				}

				if (ImGui::Button(SString("Load Beta Version").Decrypt(), ImVec2(240.f, 0.f))) {
					if (userHWIDCheck() == SString("correct").Decrypt()) {
						decryptionKey = getDecryptionKey();
						loadBetaHack(decryptionKey.c_str(), 64);
					}
				}
			}
			else if (User::role == 3)
			{
				ImGui::SetCursorPos({ 5.f,33.f });
				if (ImGui::Button(SString("Load Premium Version").Decrypt(), ImVec2(240.f, 0.f))) {
					if (userHWIDCheck() == SString("correct").Decrypt()) {
						decryptionKey = getDecryptionKey();
						loadHack(decryptionKey.c_str(), 64);
					}
				}

				if (ImGui::Button(SString("Load Beta Version").Decrypt(), ImVec2(240.f, 0.f))) {
					if (userHWIDCheck() == SString("correct").Decrypt()) {
						decryptionKey = getDecryptionKey();
						loadBetaHack(decryptionKey.c_str(), 64);
					}
				}

				if (ImGui::Button(SString("Load Debug Version").Decrypt(), ImVec2(240.f, 0.f))) {
					if (userHWIDCheck() == SString("correct").Decrypt()) {
						decryptionKey = getDecryptionKey();
						loadHack(decryptionKey.c_str(), 64);
					}
				}
			}

			ImGui::SetCursorPos({ 5.f,361.f });
			if (ImGui::Button(SString("Discord").Decrypt(), { 98.f,33.f })) {
				std::string username;
				std::string password;
				username = globals.user_name;
				password = globals.pass_word;
				string url = SString("https://beta.aaaaaa.me/discord/login.php?username=").Decrypt() + username + SString("&password=").Decrypt() + password;
				ShellExecute(NULL, SString("open").Decrypt(), url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
			}

			ImGui::SetCursorPos({ 594.f,361.f });
			if (ImGui::Button(SString("Settings").Decrypt(), { 98.f,33.f }))
			{
				Settings::Tab = 3;
			}
		}
		else if (Settings::Tab == 3)
		{
			if (globals.time_left == SString("0").Decrypt())
			{
				globals.subscribed = SString("No").Decrypt();
			}
			else
			{
				globals.subscribed = SString("Yes").Decrypt();
			}
			ui::window_size = { 465.f,200.f };
			ui::window_title = "Easy Victory - Account Management";
			ImGui::SetCursorPos({ 10.f,30.f });
			ImGui::PushItemWidth(77.000000);
			ImGui::Text(SString("Subscribed:").Decrypt());
			ImGui::PopItemWidth();
			ImGui::SetCursorPos({ 110.f,30.f });
			ImGui::PushItemWidth(28.000000);
			ImGui::Text(globals.subscribed.c_str());
			ImGui::PopItemWidth();
			ImGui::SetCursorPos({ 10.f,55.f });
			ImGui::PushItemWidth(70.000000);
			ImGui::Text(SString("Days Left:").Decrypt());
			ImGui::PopItemWidth();
			ImGui::SetCursorPos({ 100.f,55.f });
			ImGui::PushItemWidth(28.000000);
			ImGui::Text(globals.time_left.c_str());
			ImGui::PopItemWidth();
			ImGui::SetCursorPos({ 10.f,80.f });
			ImGui::PushItemWidth(245.000000);
			ImGui::InputText(SString("Key").Decrypt(), globals.redeem_key, IM_ARRAYSIZE(globals.redeem_key));
			ImGui::PopItemWidth();
			ImGui::SetCursorPos({ 10.f,110.f });
			if (ImGui::Button(SString("Redeem Key").Decrypt(), { 245.f,25.f }))
			{
				std::string result;
				CURL* curl;

				curlMutex.lock();
				curl_global_init(CURL_GLOBAL_DEFAULT);
				curl = curl_easy_init();
				if (curl)
				{
					std::string key = globals.redeem_key;
					std::string username = globals.user_name;
					std::string url = SString("https://beta.aaaaaa.me/api/sub.php?username=").Decrypt() + username + SString("&subCode=").Decrypt() + key;
					curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
					curl_easy_setopt(curl, CURLOPT_USERAGENT, SString(USER_AGENT_NAME).Decrypt());
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
					curl_easy_perform(curl);
					curl_easy_cleanup(curl);
				}

				curlMutex.unlock();

				if (result == SString("Sub activated.").Decrypt()){
					Role(globals.user_name);
					TimeLeft(globals.user_name);
				}
				else if (result == SString("Sub is already active.").Decrypt())
				{
					MessageBox(0, SString("You've already activated your subscription").Decrypt(), SString("Error!").Decrypt(), MB_OK);
				}

				else if (result == SString("Please enter a code.").Decrypt())
				{
					MessageBox(0, SString("Please enter a code.").Decrypt(), SString("Empty!").Decrypt(), MB_OK);
				}
				else if (result == SString("Invalid code.").Decrypt())
				{
					MessageBox(0, SString("Invalid code, Please recheck it or enter another one!").Decrypt(), SString("Error!").Decrypt(), MB_OK);
				}
				else
				{
					MessageBox(0, SString("Please contact us at our discord server!").Decrypt(), SString("Error!").Decrypt(), MB_OK);
					MessageBox(0, result.c_str(), SString("Error!").Decrypt(), MB_OK);
				}

			}

			ImGui::SetCursorPos({ 10.f,165.f });
			if (ImGui::Button(SString("Reset HWID").Decrypt(), { 245.f,25.f }))
			{
				std::string result;
				CURL* curl;

				curlMutex.lock();
				curl_global_init(CURL_GLOBAL_DEFAULT);
				curl = curl_easy_init();
				if (curl)
				{
					std::string username;
					username = globals.user_name;
					std::string url = SString("https://beta.aaaaaa.me/api/hwid-reset.php?username=").Decrypt() + username;
					curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
					curl_easy_setopt(curl, CURLOPT_USERAGENT, SString(USER_AGENT_NAME).Decrypt());
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
					curl_easy_perform(curl);
					curl_easy_cleanup(curl);
				}

				curlMutex.unlock();

				if (result == SString("valid").Decrypt()) {
					globals.user_hwid == "";
				}
				else if (result == SString("invalid").Decrypt())
				{
					MessageBox(0, SString("You've to be subscribed to reset your HWID").Decrypt(), SString("ERROR!").Decrypt(), MB_OK);
				}
				else
				{
					MessageBoxA(0, result.c_str(), SString("ERROR!").Decrypt(), MB_OK);
				}
			}

			ImGui::SetCursorPos({ 355.f,165.f });
			if (globals.time_left != SString("Sub-Ended").Decrypt())
			{
				if (ImGui::Button(SString("Back").Decrypt(), { 100.f,25.f }))
				{
					Settings::Tab = 2;
				}
			}
		}

		ImGui::End();
	}

	ImGuiStyle* style = &ImGui::GetStyle();
	style->Colors[ImGuiCol_TitleBg] = ImColor(255, 101, 53, 255);
	style->Colors[ImGuiCol_TitleBgActive] = ImColor(255, 101, 53, 255);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImColor(0, 0, 0, 130);

	style->Colors[ImGuiCol_Button] = ImColor(255, 101, 53, 255);
	style->Colors[ImGuiCol_ButtonActive] = ImColor(255, 101, 53, 50);
	style->Colors[ImGuiCol_ButtonHovered] = ImColor(255, 101, 53, 125);

	style->Colors[ImGuiCol_Separator] = ImColor(70, 70, 70, 255);
	style->Colors[ImGuiCol_SeparatorActive] = ImColor(76, 76, 76, 255);
	style->Colors[ImGuiCol_SeparatorHovered] = ImColor(76, 76, 76, 255);

	style->Colors[ImGuiCol_FrameBg] = ImColor(37, 36, 37, 255);
	style->Colors[ImGuiCol_FrameBgActive] = ImColor(37, 36, 37, 255);
	style->Colors[ImGuiCol_FrameBgHovered] = ImColor(37, 36, 37, 255);

	style->Colors[ImGuiCol_Header] = ImColor(0, 0, 0, 0);
	style->Colors[ImGuiCol_HeaderActive] = ImColor(0, 0, 0, 0);
	style->Colors[ImGuiCol_HeaderHovered] = ImColor(46, 46, 46, 255);
}

void checkVersion()
{
	CURL*       curl;
	std::string url, result = "";

	curlMutex.lock();
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl) {
		url = SString("https://beta.aaaaaa.me/api/version-check.php").Decrypt();
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, SString(USER_AGENT_NAME).Decrypt());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	curlMutex.unlock();

	if (result != SString(VERSION).Decrypt()) {
		selfDelete();
		exit(EXIT_FAILURE);
	}
}

void ui::init(LPDIRECT3DDEVICE9 device) 
{
	checkVersion();

	dev = device;
	ImGui::StyleColorsDark();

	if (window_pos.x == 0) {
		RECT screen_rect{};
		GetWindowRect(GetDesktopWindow(), &screen_rect);
		screen_res = ImVec2(float(screen_rect.right), float(screen_rect.bottom));
		window_pos = (screen_res - window_size) * 0.5f;
		getUserCredentials();
		// init images here
	}
}
