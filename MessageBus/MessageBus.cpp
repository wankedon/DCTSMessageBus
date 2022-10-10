// MessageBus.cpp: WinMain 的实现


#include "pch.h"
#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_FREE_THREADED

#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 某些 CString 构造函数将是显式的


#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include "MessageBus_i.h"
#include "SubPub.h"
#include "Logger.h"
#include "StringConv.h"
#include "ConfigLoader.h"
using namespace std;

static wstring LocateModulePath()
{
	TCHAR szbufPath[MAX_PATH] = TEXT("");
	TCHAR szLongPath[MAX_PATH] = TEXT("");
	::GetModuleFileName(NULL, szbufPath, MAX_PATH);
	::GetLongPathName(szbufPath, szLongPath, MAX_PATH);
	wstring fileName(szLongPath);
	wstring::size_type pos = fileName.find_last_of(_T("\\"));
	return fileName.substr(0, pos);
}

using namespace ATL;

#include <stdio.h>

class CMessageBusModule : public ATL::CAtlServiceModuleT< CMessageBusModule, IDS_SERVICENAME >
{
public :
	DECLARE_LIBID(LIBID_MessageBusLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_MESSAGEBUS, "{f85dfc1d-3601-4333-9679-3abb4bf9f3a2}")
	HRESULT InitializeSecurity() throw()
	{
		// TODO : 调用 CoInitializeSecurity 并为服务提供适当的安全设置
		// 建议 - PKT 级别的身份验证、
		// RPC_C_IMP_LEVEL_IDENTIFY 的模拟级别
		//以及适当的非 NULL 安全描述符。

		return S_OK;
	}

public:
	HRESULT RegisterAppId(bool bService = false) throw ()
	{
		HRESULT hr = __super::RegisterAppId(bService);
		if (bService)
		{
			SC_HANDLE hSCM = ::OpenSCManagerW(NULL, NULL, SERVICE_CHANGE_CONFIG);
			SC_HANDLE hService = NULL;
			if (hSCM == NULL)
			{
				hr = AtlHresultFromLastError();
			}
			else
			{
				hService = ::OpenService(hSCM, m_szServiceName, SERVICE_CHANGE_CONFIG);
				if (hService != NULL)
				{
					::ChangeServiceConfig(hService, SERVICE_NO_CHANGE,
						SERVICE_AUTO_START,// 修改服务为自动启动
						NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
					SERVICE_DESCRIPTION Description;
					TCHAR szDescription[1024];
					ZeroMemory(szDescription, 1024);
					ZeroMemory(&Description, sizeof(SERVICE_DESCRIPTION));
					lstrcpy(szDescription, _T("电磁态势消息总线服务"));
					Description.lpDescription = szDescription;
					::ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &Description);
					::CloseServiceHandle(hService);
				}
				else
				{
					hr = AtlHresultFromLastError();
				}
				::CloseServiceHandle(hSCM);
			}
		}
		return hr;
	}
	HRESULT PreMessageLoop(int nShowCmd) throw();
	HRESULT PostMessageLoop() throw();
private:
	unique_ptr<Broker> broker;
};

CMessageBusModule _AtlModule;


HRESULT CMessageBusModule::PreMessageLoop(int nShowCmd) throw()
{
	wstring path = LocateModulePath();
	auto u8String = StrConvert::wstringToUTF8(path);
	Logger::PATH = StrConvert::UTF8ToGbk(u8String) + "/log.txt";
	HRESULT hr = __super::PreMessageLoop(nShowCmd);
	const wstring configFileName = L"/总线配置.xml";
	if (SUCCEEDED(hr))
	{
		ConfigLoader configLoader(path + configFileName);
		auto config = configLoader.load();
		if (config != nullptr)
		{
			broker = make_unique<Broker>(config->name, config->pubConfig, config->subConfig);
			broker->subTopic("");	//broker转发所有消息
			SetServiceStatus(SERVICE_RUNNING);	//把服务标记为运行状态
		}
		// Add any custom code to initialize your service
	}
	LOG("PreMessageLoop error {}", hr);
	return hr;
}
//

HRESULT CMessageBusModule::PostMessageLoop() throw()
{
	HRESULT hr = __super::PostMessageLoop();
	if (SUCCEEDED(hr))
	{
		broker.reset();
		zsys_shutdown();
		LOG("---------------message bus quit------------------");
		LOGFlush();
		SetServiceStatus(SERVICE_STOPPED);
	}
	return hr;
}

extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/,
								LPTSTR /*lpCmdLine*/, int nShowCmd)
{
#ifdef SHOW_CONSOLE
	AllocConsole();
#endif
	return _AtlModule.WinMain(nShowCmd);
}

