#include <cstdio>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <memory.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <array>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <fstream>

using namespace std;

int resolvehelper(const char* hostname, int family, const char* service, sockaddr_storage* pAddr)
{
	int result;
	addrinfo* result_list = NULL;
	addrinfo hints = {};
	hints.ai_family = family;
	hints.ai_socktype = SOCK_DGRAM; // without this flag, getaddrinfo will return 3x the number of addresses (one for each socket type).
	result = getaddrinfo(hostname, service, &hints, &result_list);
	if (result == 0)
	{
		//ASSERT(result_list->ai_addrlen <= sizeof(sockaddr_in));
		memcpy(pAddr, result_list->ai_addr, result_list->ai_addrlen);
		freeaddrinfo(result_list);
	}

	return result;
}


int main()
{
	int result = 0;
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	string line;
	char szIP[100];

	sockaddr_in addrListen = {}; // zero-int, sin_port is 0, which picks a random port for bind.
	addrListen.sin_family = AF_INET;
	result = bind(sock, (sockaddr*)&addrListen, sizeof(addrListen));
	if (result == -1)
	{
		int lasterror = errno;
		std::cout << "error: " << lasterror;
		exit(1);
	}


	sockaddr_storage addrDest = {};
	result = resolvehelper("192.168.1.11", AF_INET, "9000", &addrDest);
	if (result != 0)
	{
		int lasterror = errno;
		std::cout << "error: " << lasterror;
		exit(1);
	}




	while (true)
	{
		ifstream myfile("/sys/class/thermal/thermal_zone0/temp");
		if (myfile.is_open())
		{
			getline(myfile, line);

			myfile.close();
		}
		const char *msg = line.c_str();
		double x = atof(msg);
		x = x / 1000;
		msg = to_string(x).c_str();
		size_t msg_length = strlen(msg);

		result = sendto(sock, msg, msg_length, 0, (sockaddr*)&addrDest, sizeof(addrDest));
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	}

	return 0;
}