#ifndef PACKET_H
#define PACKET_H

#include "header.h"

class Packet {
public:
	static const int MAX_BODY_LEN = 1024;
	static const int MIN_BODY_LEN = 1;
	static Packet *newPacket(Header header);
	static Packet *newPacket(uint8_t version, enum PacketType PacketType, uint16_t BodySize);
	Packet(Header &header);
	Packet(uint8_t version, enum PacketType PacketType, uint16_t BodySize);
	~Packet();
	char *getData();
	enum PacketType getPacketType();
	void setPacketType(enum PacketType PacketType);
	uint16_t getBodySize();
	void setBodySize(uint16_t BodySize);
	char *getBody();
	void setBody(char *body, uint16_t len);
	int bodyBytesLeft();
	int feedBody(char *buf, int n);
	virtual void parseBody() = 0;
	void buildBody(char *body);
	const Header& getHeader() const;
private:
	void constructPacket(uint8_t version, enum PacketType PacketType, uint16_t BodySize);
	Header header;
	int body_pos;
protected:
	char *body;
};

#endif
