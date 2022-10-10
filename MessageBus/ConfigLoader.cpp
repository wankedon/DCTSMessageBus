#include "pch.h"
#include "ConfigLoader.h"
#include "Miscellaneous.h"
#include "StringConv.h"
#include "Logger.h"

using namespace std;

ConfigLoader::ConfigLoader(const std::wstring& fp)
	:filePath(fp)
{
}

std::unique_ptr<ConfigLoader::BusConfig> ConfigLoader::load()
{
	pugi::xml_document doc;
	if (!doc.load_file(filePath.c_str()))
	{
		LOG("configuration load failed");
		return nullptr;
	}
	auto root = doc.child(L"配置");
	if (root == nullptr)
		return nullptr;
	auto result = make_unique<ConfigLoader::BusConfig>();
	auto name = root.child(L"名字");
	if (name)
	{
		result->name = StrConvert::wstringToUTF8(name.text().as_string());
	}
	auto xmlPub = root.child(L"发布");
	if (xmlPub)
	{
		result->pubConfig = loadLinkCfg(xmlPub);
	}

	auto xmlSub = root.child(L"订阅");
	if (xmlSub)
	{
		result->subConfig = loadLinkCfg(xmlSub);
	}
	return result;
}

ZeromqLinkCfg ConfigLoader::loadLinkCfg(pugi::xml_node xmlNode)
{
	ZeromqLinkCfg cfg;

	auto xmlName = xmlNode.child(L"名字");
	if (xmlName)
	{
		cfg.set_link_name(StrConvert::wstringToUTF8(xmlName.text().as_string()));
	}

	auto xmlAddress = xmlNode.child(L"地址");
	if (xmlAddress)
	{
		auto address = cfg.mutable_address();
		auto u8Ip = StrConvert::wstringToUTF8(xmlAddress.child(L"ip").text().as_string());
		address->set_ip(u8Ip);
		address->set_port(xmlAddress.child(L"端口").text().as_uint());
	}
	auto xmlHeartbeat = xmlNode.child(L"心跳");
	if (xmlHeartbeat)
	{
		auto heartbeat = cfg.mutable_heartbeat_cfg();
		heartbeat->set_heartbeat_interval(xmlHeartbeat.child(L"间隔").text().as_int());
		heartbeat->set_heartbeat_timeout(xmlHeartbeat.child(L"超时").text().as_int());
		heartbeat->set_heartbeat_ttl(xmlHeartbeat.child(L"TTL").text().as_int());
	}
	auto xmlReconnection = xmlNode.child(L"重连");
	if (xmlReconnection)
	{
		auto reconnection = cfg.mutable_reconnect_cfg();
		reconnection->set_reconnect_interval(xmlReconnection.child(L"间隔").text().as_int());
		reconnection->set_reconnect_interval_max(xmlReconnection.child(L"最大间隔").text().as_int());
	}

	auto xmlSendOption = xmlNode.child(L"发送");
	if (xmlSendOption)
	{
		auto sendOption = cfg.mutable_msg_send_cfg();
		sendOption->set_hwm(xmlSendOption.child(L"高水位").text().as_int());
		sendOption->set_linger(xmlSendOption.child(L"悬挂").text().as_int());
		sendOption->set_timeout(xmlSendOption.child(L"超时").text().as_int());
	}
	return cfg;
}