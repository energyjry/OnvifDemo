#include "OnvifClient.h"
#include "onvif/wsdd.nsmap"
#ifdef WIN32
//#include <Iphlpapi.h>
#else
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#endif

std::vector<std::string> OnvifClient::SearchIpc()
{
    std::vector<std::string> targetIpList;
    std::vector<std::string> localIpList = get_network();
    for (int i = 0; i < int(localIpList.size()); i++)
    {
        std::cout << "search :" << localIpList[i] << std::endl;
        std::vector<std::string> target = search_ipc(localIpList[i].c_str());
        for (int j = 0; j < int(target.size()); j++)
        {
            targetIpList.push_back(target[j]);
        }
    }
    for (int i = 0; i < int(targetIpList.size()); i++)
    {
        std::cout << targetIpList[i] << std::endl;
    }
    return targetIpList;
}

bool OnvifClient::Init(const char* ip, const char* username, const char* password)
{
    m_ip = ip;
    m_username = username;
    m_password = password;
    
    m_device_endpoint = "http://" + m_ip + "/onvif/device_service";

    m_proxyDevice.soap_endpoint = m_device_endpoint.c_str();
   
    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(m_proxyDevice.soap, NULL, m_username.c_str(), m_password.c_str()))
    {
        return false;
    }
    if (SOAP_OK != soap_wsse_add_Timestamp(m_proxyDevice.soap, "Time", 10))
    {
        return false;
    }

    _tds__GetDeviceInformation* tds__GetDeviceInformation = soap_new__tds__GetDeviceInformation(m_soap, -1);
    _tds__GetDeviceInformationResponse* tds__GetDeviceInformationResponse = soap_new__tds__GetDeviceInformationResponse(m_soap, -1);

    if (SOAP_OK != m_proxyDevice.GetDeviceInformation(tds__GetDeviceInformation, tds__GetDeviceInformationResponse))
    {
        soap_destroy(m_soap);
        soap_end(m_soap);
        return false;
    }
    m_manufacturer = tds__GetDeviceInformationResponse->Manufacturer;
    m_serialNumber = tds__GetDeviceInformationResponse->SerialNumber;
    m_model = tds__GetDeviceInformationResponse->Model;
    std::cout << m_manufacturer << "\t" << m_manufacturer << "\t" << m_model << std::endl;
    return true;
}

bool OnvifClient::GetDeviceInfo(std::string &Manufacturer, std::string &SerialNumber, std::string &Model)
{
    Manufacturer = m_manufacturer;
    SerialNumber = m_serialNumber;
    Model = m_model;
    return true;
}

bool OnvifClient::Capabilities()
{
    m_proxyDevice.soap_endpoint = m_device_endpoint.c_str();
    
    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(m_proxyDevice.soap, NULL, m_username.c_str(), m_password.c_str()))
    {
        return false;
    }
    if (SOAP_OK != soap_wsse_add_Timestamp(m_proxyDevice.soap, "Time", 10))
    {
        return false;
    }

    _tds__GetCapabilities* tds__GetCapabilities = soap_new__tds__GetCapabilities(m_soap, -1);
    tds__GetCapabilities->Category.push_back(tt__CapabilityCategory__All);

    _tds__GetCapabilitiesResponse* tds__GetCapabilitiesResponse = soap_new__tds__GetCapabilitiesResponse(m_soap, -1);
    if (SOAP_OK != m_proxyDevice.GetCapabilities(tds__GetCapabilities, tds__GetCapabilitiesResponse))
    {
        return false;
    }

    if (tds__GetCapabilitiesResponse->Capabilities->Media != NULL)
    {
        m_media_endpoint = tds__GetCapabilitiesResponse->Capabilities->Media->XAddr;
        std::cout << m_media_endpoint << std::endl;
    }
    if (tds__GetCapabilitiesResponse->Capabilities->PTZ != NULL)
    {
        m_ptz_endpoint = tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr;
        std::cout << m_ptz_endpoint << std::endl;
    }
    return true;
}

bool OnvifClient::GetMediaProfile()
{
    if (m_media_endpoint.length() == 0)
    {
        return false;
    }
    m_proxyMedia.soap_endpoint = m_media_endpoint.c_str();

    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(m_proxyMedia.soap, NULL, m_username.c_str(), m_password.c_str()))
    {
        return false;
    }

    if (SOAP_OK != soap_wsse_add_Timestamp(m_proxyMedia.soap, "Time", 10))
    {
        return false;
    }
    
    _trt__GetProfiles* trt__GetProfiles = soap_new__trt__GetProfiles(m_soap, -1);
    _trt__GetProfilesResponse* trt__GetProfilesResponse = soap_new__trt__GetProfilesResponse(m_soap, -1);

    if (SOAP_OK != m_proxyMedia.GetProfiles(trt__GetProfiles, trt__GetProfilesResponse))
    {
        return false;
    }
    for (int i = 0; i < int(trt__GetProfilesResponse->Profiles.size()); i++)
    {
        std::cout << trt__GetProfilesResponse->Profiles[i]->token << std::endl;
        m_profileList.push_back(trt__GetProfilesResponse->Profiles[i]->token);
    }
    return true;
}

#ifdef WIN32
std::vector<std::string> OnvifClient::get_network()
{
	std::vector<std::string> ipList;
	/*
    PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
    int netCardNum = 0;
    int IPnumPerNetCard = 0;
    if (ERROR_BUFFER_OVERFLOW == nRel) {
        delete pIpAdapterInfo;
        pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
        nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
    }
    if (ERROR_SUCCESS == nRel)
    {
        while (pIpAdapterInfo)
        {
            if (pIpAdapterInfo->Type == MIB_IF_TYPE_ETHERNET)
            {
                std::string ip_address = pIpAdapterInfo->IpAddressList.IpAddress.String;
                if (ip_address != "0.0.0.0")
                {
                    ipList.push_back(pIpAdapterInfo->IpAddressList.IpAddress.String);
                }
            }
            pIpAdapterInfo = pIpAdapterInfo->Next;
        }
    }
    //�ͷ��ڴ�ռ�
    if (pIpAdapterInfo) {
        delete pIpAdapterInfo;
    }
    */
	return ipList;
}
#else
std::vector<std::string> OnvifClient::get_network()
{
	std::vector<std::string> ipList;
	struct ifreq ifr;
	struct ifconf ifc;
	char buf[20480];
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == -1) {
		printf("socket error\n");
		return ipList;
	}
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
		printf("ioctl error\n");
		return ipList;
	}
	struct ifreq* it = ifc.ifc_req;
	const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

	uint8_t i = 0;
	for (; it != end; ++it)
	{
		strcpy(ifr.ifr_name, it->ifr_name);
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0)
		{
			if (! (ifr.ifr_flags & IFF_LOOPBACK))
			{
				// don't count loopback
				char mac[20];
				char ip[16];
				char name[32];
				bool isLink = ifr.ifr_flags & IFF_UP;
				if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0)
				{
					unsigned char * ptr ;
					ptr = (unsigned char  *)&ifr.ifr_ifru.ifru_hwaddr.sa_data[0];
					snprintf(mac,20,"%02X:%02X:%02X:%02X:%02X:%02X",*ptr,*(ptr+1),*(ptr+2),*(ptr+3),*(ptr+4),*(ptr+5));
				}
				if (ioctl(sock, SIOCGIFADDR, &ifr) == 0)
				{
					unsigned char * ptr ;
					ptr = (unsigned char  *)&ifr.ifr_ifru.ifru_addr.sa_data;

					snprintf(ip,16,"%d.%d.%d.%d",*(ptr+2),*(ptr+3),*(ptr+4),*(ptr+5));
				}
				snprintf(name,32,"%s",ifr.ifr_name);
				printf("%s %20s   \t%s\t %s\n",isLink?"true":"false", name,ip,mac);
				ipList.push_back(std::string(ip));
			}
			i++;
		}else
		{
			printf("get mac info error\n");
			return ipList;
		}
	}
	return ipList;
}
#endif

std::vector<std::string> OnvifClient::search_ipc(const char* localIp)
{
    std::vector<std::string> ipList;
    struct soap* soap;
    struct wsdd__ProbeType req;
    struct __wsdd__ProbeMatches resp;
    struct wsdd__ScopesType sScope;
    struct SOAP_ENV__Header header;

    int result = 0;
    soap = soap_new();
    if (soap == NULL)
    {
        return ipList;
    }
    soap_set_namespaces(soap, namespaces);
    soap->recv_timeout = 1;
    soap_default_SOAP_ENV__Header(soap, &header);


    header.wsa__MessageID = const_cast<char*>(soap_wsa_rand_uuid(soap));
    header.wsa__To = (char*)"urn:schemas-xmlsoap-org:ws:2005:04:discovery";
    header.wsa__Action = (char*)"http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";
    soap->header = &header;
    struct in_addr if_req;
    if_req.s_addr = inet_addr(localIp);
    soap->ipv4_multicast_if = (char*)soap_malloc(soap, sizeof(in_addr));
    memset(soap->ipv4_multicast_if, 0, sizeof(in_addr));
    memcpy(soap->ipv4_multicast_if, (char*)&if_req, sizeof(if_req));
    soap_default_wsdd__ScopesType(soap, &sScope);
    sScope.__item = (char*)"onvif://www.onvif.org";
    soap_default_wsdd__ProbeType(soap, &req);
    req.Scopes = &sScope;
    req.Types = (char*)"ns1:NetworkVideoTransmitter";
    result = soap_send___wsdd__Probe(soap, "soap.udp://239.255.255.250:3702", NULL, &req);
    while (result == SOAP_OK)
    {
        result = soap_recv___wsdd__ProbeMatches(soap, &resp);
        if (result == SOAP_OK)
        {
            if (!(soap->header))
            {
                continue;
            }
            std::cout << "wsdd:UriListType:" << resp.wsdd__ProbeMatches->ProbeMatch->XAddrs << std::endl;
            std::string xAddrs = resp.wsdd__ProbeMatches->ProbeMatch->XAddrs;
            std::string tempUri = xAddrs.substr(xAddrs.find_first_of('/') + 2);
            std::string ip = tempUri.substr(0, tempUri.find_first_of('/'));
            ipList.push_back(ip);
        }
    }
    soap_destroy(soap);
    soap_end(soap);
    return ipList;
}

bool OnvifClient::GetStreamUri()
{
    if (m_media_endpoint.length() == 0)
    {
        return false;
    }
    m_proxyMedia.soap_endpoint = m_media_endpoint.c_str();

    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(m_proxyMedia.soap, NULL, m_username.c_str(), m_password.c_str()))
    {
        return false;
    }

    if (SOAP_OK != soap_wsse_add_Timestamp(m_proxyMedia.soap, "Time", 10))
    {
        return false;
    }

    _trt__GetStreamUri* trt__GetStreamUri = soap_new__trt__GetStreamUri(m_soap, -1);
    trt__GetStreamUri->StreamSetup = soap_new_tt__StreamSetup(m_soap, -1);
    trt__GetStreamUri->StreamSetup->Stream = tt__StreamType__RTP_Unicast;
    trt__GetStreamUri->StreamSetup->Transport = soap_new_tt__Transport(m_soap, -1);
    trt__GetStreamUri->StreamSetup->Transport->Protocol = tt__TransportProtocol__RTSP;

    _trt__GetStreamUriResponse* trt__GetStreamUriResponse = soap_new__trt__GetStreamUriResponse(m_soap, -1);

    for (int i = 0; i < int(m_profileList.size()); i++)
    {
        trt__GetStreamUri->ProfileToken = m_profileList[i];

        if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(m_proxyMedia.soap, NULL, m_username.c_str(), m_password.c_str()))
        {
            return false;
        }

        if (SOAP_OK == m_proxyMedia.GetStreamUri(trt__GetStreamUri, trt__GetStreamUriResponse))
        {
            std::string url_head = trt__GetStreamUriResponse->MediaUri->Uri.substr(0, trt__GetStreamUriResponse->MediaUri->Uri.find_first_of('/') + 2);
            std::string url_tail = trt__GetStreamUriResponse->MediaUri->Uri.substr(trt__GetStreamUriResponse->MediaUri->Uri.find_first_of('/') + 2);
            std::string rtspUrl = url_head + std::string(m_username) + ":" + std::string(m_password) + "@" + url_tail;
            std::cout << "get rtsp url:" << rtspUrl << std::endl;
            m_rtspUrlList.push_back(rtspUrl);
        }
    }
    return true;
}

bool OnvifClient::PtzControl(std::string command, float speed)
{
    if (m_ptz_endpoint.length() == 0)
    {
        return false;
    }
    m_proxyPtz.soap_endpoint = m_ptz_endpoint.c_str();

    if (m_profileList.size() == 0)
    {
        return false;
    }

    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(m_proxyPtz.soap, NULL, m_username.c_str(), m_password.c_str()))
    {
        return false;
    }
    if (SOAP_OK != soap_wsse_add_Timestamp(m_proxyPtz.soap, "Time", 10))
    {
        return false;
    }
    
    _tptz__ContinuousMove* continuousMove = soap_new__tptz__ContinuousMove(m_soap);
    _tptz__ContinuousMoveResponse* response = soap_new__tptz__ContinuousMoveResponse(m_soap);
    continuousMove->ProfileToken = const_cast<char*>(m_profileList[0].c_str());
    tt__PTZSpeed* velocity = soap_new_tt__PTZSpeed(m_soap);
    continuousMove->Velocity = velocity;
    tt__Vector2D* panTilt = soap_new_tt__Vector2D(m_soap);
    continuousMove->Velocity->PanTilt = panTilt;
    std::string space = "http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace";
    continuousMove->Velocity->PanTilt->space = &space;
    tt__Vector1D* zoom = soap_new_tt__Vector1D(m_soap);
    continuousMove->Velocity->Zoom = zoom;
    if (command == "up")
    {
        continuousMove->Velocity->PanTilt->x = 0;
        continuousMove->Velocity->PanTilt->y = ((float)speed / 10);
    }
    else if (command == "down")
    {
        continuousMove->Velocity->PanTilt->x = 0;
        continuousMove->Velocity->PanTilt->y = -((float)speed / 10);
    }
    else if (command == "left")
    {
        continuousMove->Velocity->PanTilt->x = -((float)speed / 10);
        continuousMove->Velocity->PanTilt->y = 0;
    }
    else if(command == "right")
    {
        continuousMove->Velocity->PanTilt->x = ((float)speed / 10);
        continuousMove->Velocity->PanTilt->y = 0;
    }else if (command == "up_left")
    {
        continuousMove->Velocity->PanTilt->x = -((float)speed / 10);
        continuousMove->Velocity->PanTilt->y = ((float)speed / 10);
    }
    else if (command == "down_left")
    {
        continuousMove->Velocity->PanTilt->x = -((float)speed / 10);
        continuousMove->Velocity->PanTilt->y = -((float)speed / 10);
    }
    else if (command == "up_right")
    {
        continuousMove->Velocity->PanTilt->x = ((float)speed / 10);
        continuousMove->Velocity->PanTilt->y = ((float)speed / 10);
    }
    else if (command == "down_right")
    {
        continuousMove->Velocity->PanTilt->x = ((float)speed / 10);
        continuousMove->Velocity->PanTilt->y = -((float)speed / 10);
    }
    else if (command == "zoom_in")
    {
        continuousMove->Velocity->Zoom->x = ((float)speed / 10);
    }
    else if (command == "zoom_out")
    {
        continuousMove->Velocity->Zoom->x = -((float)speed / 10);
    }
    else 
    {
        return false;
    }
    if (m_proxyPtz.ContinuousMove(continuousMove, response) != SOAP_OK)
    {
        return false;
    }
    std::cout << "ptz:" << command << "\t" << speed << std::endl;
    return true;
}

bool OnvifClient::PtzStop()
{
    if (m_ptz_endpoint.length() == 0)
    {
        return false;
    }
    m_proxyPtz.soap_endpoint = m_ptz_endpoint.c_str();

    if (m_profileList.size() == 0)
    {
        return false;
    }

    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(m_proxyPtz.soap, NULL, m_username.c_str(), m_password.c_str()))
    {
        return false;
    }
    if (SOAP_OK != soap_wsse_add_Timestamp(m_proxyPtz.soap, "Time", 10))
    {
        return false;
    }
    _tptz__Stop* stop = soap_new__tptz__Stop(m_soap);
    _tptz__StopResponse* response = soap_new__tptz__StopResponse(m_soap);
    stop->ProfileToken = const_cast<char*>(m_profileList[0].c_str());
    bool* pantilt = new bool;
    stop->PanTilt = pantilt;
    *(stop->PanTilt) = true;
    bool* zoom = new bool;
    stop->Zoom = zoom;
    *(stop->Zoom) = true;
    if (SOAP_OK == !m_proxyPtz.Stop(stop, response))
    {
        return false;
    }
    std::cout << "ptz stop"  <<std::endl;
    return true;
}

OnvifClient::OnvifClient()
{
    m_soap = soap_new();
    soap_register_plugin(m_proxyDevice.soap, soap_wsse);
    soap_register_plugin(m_proxyMedia.soap, soap_wsse);
    soap_register_plugin(m_proxyPtz.soap, soap_wsse);
}

OnvifClient::~OnvifClient()
{
    soap_destroy(m_soap);
    soap_end(m_soap);
}
