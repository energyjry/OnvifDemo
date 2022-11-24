#pragma once
#include <iostream>
#include "stdio.h"
#include "onvif/wsseapi.h"
#include "onvif/wsaapi.h"
#include <openssl/rsa.h>
#include "onvif/soapRemoteDiscoveryBindingProxy.h"
#include "onvif/soapDeviceBindingProxy.h"
#include "onvif/soapMediaBindingProxy.h"
#include "onvif/soapPTZBindingProxy.h"


class OnvifClient
{
private:
	std::string m_ip;
	std::string m_username;
	std::string m_password;
	std::string m_manufacturer;
	std::string m_serialNumber;
	std::string m_model;

	struct soap* m_soap;
	DeviceBindingProxy m_proxyDevice;
	MediaBindingProxy m_proxyMedia;
	PTZBindingProxy m_proxyPtz;
	std::string m_device_endpoint;
	std::string m_media_endpoint;
	std::string m_ptz_endpoint;
	std::vector<std::string> m_profileList;
	std::vector<std::string> m_rtspUrlList;

private:
	std::vector<std::string> get_network();


public:
	std::vector<std::string> search_ipc(const char* localIp);
	std::vector<std::string> SearchIpc();
	bool Init(const char* ip, const char* username, const char* password);
	bool GetDeviceInfo(std::string &Manufacturer, std::string &SerialNumber, std::string &Model);
	bool Capabilities();
	bool GetMediaProfile();
	bool GetStreamUri();
	bool PtzControl(std::string command, float speed = 2);
	bool PtzStop();

public:
	OnvifClient();
	virtual ~OnvifClient();
};

