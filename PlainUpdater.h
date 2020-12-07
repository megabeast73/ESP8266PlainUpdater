/*
 * PlainUpdater.h
 *
 *  Licensing ? No licesing!
 *  You can do with the follwoing code whatever you want, instead of buying it!
 *
 *  Working for you ? Like it ? Plan to use it ?
 *      Author: Vladimir Ivanov
 */

#ifndef PLAINUPDATER_H_
#define PLAINUPDATER_H_
#include <Updater.h>
#include <IPAddress.h>

#include <WiFiServer.h>
#include <WiFiClient.h>

#define UDP_LISTENER_PORT 18777
#define UDP_HELLO 0xBAADF00D
#define TCP_LISTENER_PORT 18778
//In msec
#define TCP_CONNECTION_CHECK_INTERVAL 100
#define UDP_BROADCAST_CHECK_INTERVAL 5

class UDP;

class PlainUpdater {

public:
	//set udp_port to zero, if you dont want discovery replies
	PlainUpdater(uint16_t udp_port = UDP_LISTENER_PORT, uint16_t tcp_listen_port = TCP_LISTENER_PORT, uint32_t hello = UDP_HELLO);
	virtual ~PlainUpdater();

	void process();

//	static bool start(IPAddress addr, uint32 port, uint32 size);
	static void emergency(const IPAddress& addr,uint32 port);

protected:
	static void on_progress(uint progress,uint total);

	bool processInt();
	void checkUDP();
	void checkTCP();

protected:
	uint32 m_helloMark;
	uint32 m_tcpPort;
	uint32 m_udpPort;
	int m_tcpCheck;
	int m_udpCheck;
	uint64 m_last;

	UDP* m_udpListener; //Use the base class. Depending of requirements another inheritor may be used
	WiFiClient m_client;
	WiFiServer m_server;
};

#endif /* PLAINUPDATER_H_ */
