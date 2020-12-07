/*
 * PlainUpdater.cpp
 *
 *  Created on: Dec 5, 2020
 *      Author: Pc
 */

#include "PlainUpdater.h"
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include "Arduino.h"

#ifdef DEBUG_TCP_PORT
#include "DebugOutput.h"
#else
#define Debug Serial
#endif


//PlainUpdater::PlainUpdater()
//{
//
//}
PlainUpdater::PlainUpdater(uint16_t udp_port, uint16_t tcp_listen_port, uint32_t hello) :
	m_helloMark(hello),
	m_tcpPort(tcp_listen_port),
	m_udpPort(udp_port),
	m_server(tcp_listen_port)

{
	//init the listening TCP server
	Debug.print("UPDATER: TCP@");
	Debug.print(tcp_listen_port);
	m_server.begin(tcp_listen_port,1); //MAX 1 client
	//Init the UDP discovery monitor. port 0, means no discovery
	if (udp_port)
	{
		Debug.print(" UDP@");
		Debug.println(udp_port);
		m_udpListener = new WiFiUDP;
		if (!m_udpListener->begin(udp_port))
			Debug.println("ERR UDP");
	}
	else {
		Debug.println("NO UDP");
		m_udpListener = nullptr;
	}
	m_udpCheck = UDP_BROADCAST_CHECK_INTERVAL;
	m_tcpCheck = TCP_CONNECTION_CHECK_INTERVAL;
	m_last =0;
}


PlainUpdater::~PlainUpdater()
{

	if (m_udpListener)
		delete m_udpListener;
}

void PlainUpdater::emergency(const IPAddress& addr,uint32 port)
{
	static bool emergencyChecked = false;

	if (emergencyChecked)
		return; //We already did the check;
	Debug.print("Emergency update: ");
	WiFiClient client;
	emergencyChecked = true;
	if (!client.connect(addr, port))
	{
		Debug.print("Can't connect: ");
		Debug.println(addr.toString());
		return;
	}
	client.setTimeout(3000); //3 secs for incoming packet when updating

	//Get the size. First 4 bytes at the beginning
	uint32 readBytes;
	while (client.available() < sizeof(uint32))
			;
	client.read((unsigned char*) &readBytes,sizeof(readBytes));

	//Setup the Update
	if (!Update.begin(readBytes))
	{
		Debug.print("E: ");
		Debug.println(Update.getError());
		return;
	}
	//malloc the buffer and start receiving data
	uint8_t* buf = (uint8_t *) malloc(1024);
	int total = 0;
	while (client.connected())
	{
		if (!client.available())
			continue;
		readBytes = client.read(buf, 1024);
		Update.write(buf,readBytes);
		total += readBytes;
	}
	//Completed
	Debug.print("\nTotal: ");
	Debug.print(total);
	if (total && Update.end(false))
	{
		Debug.println(" Restarting");
		ESP.restart();
	}
	else
		Debug.println(Update.getError());
	free(buf);
}

void PlainUpdater::on_progress(uint progress,uint total)
{
	Debug.print("U: ");
	if(total) //avoid division by zero
		Debug.println(progress/(total / 100));
	else
		Debug.println(progress / 1024);

}

//Checks for discovery packets
void PlainUpdater::checkUDP()
{
	if (!m_udpListener)
		return;
	uint a = m_udpListener->available();
	if (a < sizeof(m_helloMark))
		return;
	uint32 rec = 0;
	IPAddress senderAddr(m_udpListener->remoteIP());
	uint16 senderPort = m_udpListener->remotePort();

	m_udpListener->read((char *) &rec,sizeof(uint32));

	if (rec != m_helloMark)
		return; //Someone do not know our hello
	//Reply
	Debug.print("HELLO: ");
	Debug.println(senderAddr.toString());
	m_udpListener->stop();
	m_udpListener->beginPacket(senderAddr,senderPort);
	m_udpListener->write((const unsigned char *) &m_helloMark ,sizeof(m_helloMark));
	m_udpListener->write((const unsigned char *) &m_tcpPort,sizeof(m_tcpPort));
	m_udpListener->flush();
	m_udpListener->endPacket();
	//Start listening again
	m_udpListener->begin(m_udpPort);
}

//Check for incoming PUSH update
void PlainUpdater::checkTCP()
{
	if (m_server.hasClient())
		m_client = m_server.available();

	if (m_client.available() < sizeof(uint32_t))
		return; //Still dont have enough data. Or connection is not established

	uint32_t sz = 0;
	m_client.read((unsigned char*) &sz,sizeof(sz));

	Debug.print("U Push from:");
	Debug.print(m_client.remoteIP().toString());
	Debug.print(" sz:");
	Debug.println(sz);

	Update.onProgress(&PlainUpdater::on_progress);
	Update.begin(sz);
	Update.writeStream(m_client);
	if (Update.end(false))
	{
		Debug.print("Done.Restarting");
		ESP.restart();
	}
	else
		Debug.println("Failed!");
}

void PlainUpdater::process()
{
	if (m_udpCheck < 1)
	{
		checkUDP();
		m_udpCheck = UDP_BROADCAST_CHECK_INTERVAL;
	}
	if (m_tcpCheck < 1)
	{
		checkTCP();
		m_tcpCheck = TCP_CONNECTION_CHECK_INTERVAL;
	}
	uint64 now = millis();
	if (m_last)
	{
		m_udpCheck -= (now - m_last);
		m_tcpCheck -= (now - m_last);
	}
	m_last = millis();
}
