#pragma once

#include "node/nodeInternal.pb.h"
#include "pugixml.hpp"

class ConfigLoader
{
public:
	ConfigLoader(const std::wstring& filePath);
	~ConfigLoader(void) = default;
public:
	struct BusConfig
	{
		std::string name;
		ZeromqLinkCfg pubConfig;
		ZeromqLinkCfg subConfig;
	};
public:
	std::unique_ptr<BusConfig> load();
private:
	ZeromqLinkCfg loadLinkCfg(pugi::xml_node xmlNode);
private:
	const std::wstring filePath;

};

