#ifndef HEADER_H
#define HEADER_H

#include <iostream>

using namespace std;

enum PacketType {
	       REQ_CODE_FIRST = 1,
	       REQ_CODE_INSERT_REQUEST = REQ_CODE_FIRST,
	       REQ_CODE_SEARCH_REQUEST = 2,
	       REQ_CODE_INSERT_REPLY = REQ_CODE_INSERT_REQUEST | 128, 
	       REQ_CODE_SEARCH_REPLY = REQ_CODE_SEARCH_REQUEST | 128, 
	       REQ_CODE_UNKNOWN,
	       REQ_CODE_LAST = REQ_CODE_UNKNOWN
};

class Header {
public:
	Header(char version, enum PacketType PacketType, uint16_t BodySize);
	uint8_t getVersion() const;
	enum PacketType getPacketType() const;
	void setPacketType(enum PacketType PacketType);
	uint16_t getBodySize() const;
	void setBodySize(uint16_t BodySize);
	bool allowablePacketType(enum PacketType PacketType);
	Header parseHeader(char *header);
private:
	char version;
	enum PacketType req_code;
	uint16_t body_size;
};

ostream& operator<<(ostream &out, const Header &h);

#endif
