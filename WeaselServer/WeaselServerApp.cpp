#include "stdafx.h"
#include "WeaselServerApp.h"

WeaselServerApp::WeaselServerApp()
	: m_handler(std::make_unique<RimeWithWeaselHandler>(&m_ui))
	, tray_icon(m_ui)
{
	//m_handler.reset(new RimeWithWeaselHandler(&m_ui));
	m_server.SetRequestHandler(m_handler.get());
	SetupMenuHandlers();
}

WeaselServerApp::~WeaselServerApp()
{
}

int WeaselServerApp::Run()
{
	if (!m_server.Start())
		return -1;

	//win_sparkle_set_appcast_url("http://localhost:8000/weasel/update/appcast.xml");
	win_sparkle_set_registry_path("Software\\Rime\\Weasel\\Updates");
	win_sparkle_init();
	m_ui.Create(m_server.GetHWnd());

	tray_icon.Create(m_server.GetHWnd());
	tray_icon.Refresh();

	m_handler->Initialize();
	m_handler->OnUpdateUI([this]() {
		tray_icon.Refresh();
	});

	int ret = m_server.Run();

	m_handler->Finalize();
	m_ui.Destroy();
	tray_icon.RemoveIcon();
	win_sparkle_cleanup();

	return ret;
}

void WeaselServerApp::SetupMenuHandlers()
{
	std::wstring dir(install_dir());
	m_server.AddMenuHandler(ID_WEASELTRAY_QUIT, [this] { return m_server.Stop() == 0; });
	m_server.AddMenuHandler(ID_WEASELTRAY_DEPLOY, std::bind(execute, dir + L"\\WeaselDeployer.exe", std::wstring(L"/deploy")));
	m_server.AddMenuHandler(ID_WEASELTRAY_SETTINGS, std::bind(execute, dir + L"\\WeaselDeployer.exe", std::wstring()));
	m_server.AddMenuHandler(ID_WEASELTRAY_DICT_MANAGEMENT, std::bind(execute, dir + L"\\WeaselDeployer.exe", std::wstring(L"/dict")));
	m_server.AddMenuHandler(ID_WEASELTRAY_SYNC, std::bind(execute, dir + L"\\WeaselDeployer.exe", std::wstring(L"/sync")));
	m_server.AddMenuHandler(ID_WEASELTRAY_WIKI, std::bind(open, L"https://rime.im/docs/"));
	m_server.AddMenuHandler(ID_WEASELTRAY_HOMEPAGE, std::bind(open, L"https://rime.im/"));
	m_server.AddMenuHandler(ID_WEASELTRAY_FORUM, std::bind(open, L"https://rime.im/discuss/"));
	m_server.AddMenuHandler(ID_WEASELTRAY_CHECKUPDATE, check_update);
	m_server.AddMenuHandler(ID_WEASELTRAY_INSTALLDIR, std::bind(explore, dir));
	m_server.AddMenuHandler(ID_WEASELTRAY_USERCONFIG, std::bind(explore, WeaselUserDataPath()));

	for (int i = 0; i < 20; ++i)
	{
		UINT menu_id = ID_WEASELTRAY_COLOR_SCHEME_BASE + i;
		m_server.AddMenuHandler(menu_id, [this, i]() {
			return _SwitchColorScheme(i);
		});
	}
}

bool WeaselServerApp::_SwitchColorScheme(int index)
{
	const std::vector<std::wstring>& ids = tray_icon.m_color_scheme_ids;
	if (index < 0 || index >= (int)ids.size())
		return false;

	std::string scheme_id;
	const std::wstring& wid = ids[index];
	scheme_id.assign(wid.begin(), wid.end());

	RimeApi* rime = rime_get_api();
	if (!rime) return false;

	RimeConfig config;
	if (!rime->config_open("weasel", &config)) return false;

	const char* current = rime->config_get_cstring(&config, "style/color_scheme");
	if (current && scheme_id == current)
	{
		rime->config_close(&config);
		return false;
	}

	rime->config_close(&config);

	RimeModule* levers_module = rime->find_module("levers");
	if (!levers_module) return false;

	RimeLeversApi* api = (RimeLeversApi*)levers_module->get_api();
	if (!api) return false;

	RimeCustomSettings* settings = api->custom_settings_init("weasel", "Weasel::UIStyleSettings");
	api->customize_string(settings, "style/color_scheme", scheme_id.c_str());

	m_handler->Reload();
	return true;
}
