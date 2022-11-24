//============================================================================
// Name        : OnvifDemo.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <string>
#include "OnvifClient.h"
using namespace std;

int main()
{
	OnvifClient mOnvifClient;
	std::vector<std::string> ipList = mOnvifClient.SearchIpc();
	//mOnvifClient.search_ipc("10.0.16.117");
	bool ret;
	ret = mOnvifClient.Init("10.0.16.118", "admin", "aykj1234");
	std::string  Manufacturer;
	std::string SerialNumber;
	std::string Model;
	ret = mOnvifClient.GetDeviceInfo(Manufacturer, SerialNumber, Model);
	ret = mOnvifClient.Capabilities();
	ret = mOnvifClient.GetMediaProfile();
	ret = mOnvifClient.GetStreamUri();
	ret = mOnvifClient.PtzControl("right", 2);
	ret = mOnvifClient.PtzStop();

	std::cout << "hello world" << std::endl;
	return 0;
}
