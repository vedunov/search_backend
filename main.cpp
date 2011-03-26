#include <iostream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <sqlite3.h>
#include "packet.h"

using namespace std;

static const uint8_t PROTO_VERSION = 1;
static const int MIN_BODY_SIZE = 1;
static const int HEADER_SIZE = 4;

class PacketException {
public:
	PacketException(string msg) {
		this->msg = msg;
	}
	void print() {
		cerr << msg << endl;
	}
private:
	string msg;
};

//class Header implementation
Header::Header(char version = PROTO_VERSION, enum PacketType PacketType = REQ_CODE_UNKNOWN,
		uint16_t BodySize = MIN_BODY_SIZE) {
	this->version = version;
	req_code = PacketType;
	body_size = BodySize;
}
enum PacketType Header::getPacketType() const {
	return req_code;
}
void Header::setPacketType(enum PacketType PacketType) {
	this->req_code = PacketType;
}
uint16_t Header::getBodySize() const {
	return body_size;
}
void Header::setBodySize(uint16_t BodySize) {
	this->body_size = BodySize;
}
uint8_t Header::getVersion() const {
	return version;
}
bool Header::allowablePacketType(enum PacketType PacketType) {
	PacketType & ~128;
	if (PacketType < REQ_CODE_FIRST || PacketType > REQ_CODE_LAST)
		return false;
	return true;
}
Header Header::parseHeader(char *header) {
	char version;
	enum PacketType req_code;
	unsigned char ch_req_code;
	uint16_t body_size;
	int n = sscanf(header, "%hhu%hhu%hu",
			&version, &ch_req_code, &body_size);
	if (n != 3)
		throw PacketException("Cannot parseHeader: number of args");
	req_code = (enum PacketType)ch_req_code;
	if (version != PROTO_VERSION)
		throw PacketException("Cannot parseHeader: invalid proto version");
	if (! allowablePacketType(req_code))
		throw PacketException("Cannot parseHeader: invalid PacketType");
	Header hdr(version, req_code, body_size);
	cout << hdr;
	return hdr;
}

ostream& operator<<(ostream &out, const Header &h) {
	out << "Ver:" << (int)h.getVersion()
	    << "; Typ:" << (int)h.getPacketType() << "; BSize:" << (int)h.getBodySize();
	return out;
}

//class SearchRequest implementation
class SearchRequest: public Packet {
	public:
		SearchRequest(Header &header): Packet(header) {}
		void parseBody() {}
};

ostream& operator<<(ostream &out, SearchRequest &sreq) {
	int bodySize = sreq.getBodySize();
	char *body = sreq.getBody();
	char *zbody = new char[bodySize + 1];
	memcpy(zbody, body, bodySize);
	zbody[bodySize] = '\0';
	out << sreq.getHeader() << "|" << zbody;
	delete[] zbody;
	return out;
}

//class SearchReply implementation
class SearchReply: public Packet {
	public:
		SearchReply(Header &header): Packet(header) {}
		SearchReply(Header &header, vector<string> _pages):
			    Packet(header), pages(_pages) {}
		void parseBody() {}
		int getPagesNum() { return pages.size(); }
		string getPage(int i) {
			return pages[i];
		}
	private:
		vector<string> pages;
};

ostream& operator<<(ostream &out, SearchReply &srepl) {
	out << srepl.getHeader() << "|" << " TotalPg:" << srepl.getPagesNum();
	for (int i = 0; i < srepl.getPagesNum(); i++)
		out << " " << srepl.getPage(i);
	return out;
}

//class InsertReply implementation
class InsertReply: public Packet {
	public:
		InsertReply(Header &header): Packet(header) {}
		void parseBody() {}
		ostream& operator<<(ostream &out) { return out; }
};

//class InsertRequest implementation
class InsertRequest: public Packet {
	public:
		InsertRequest(Header &header): Packet(header) {}
		void parseBody() {}
		ostream& operator<<(ostream &out) { return out; }
};

//class Packet implementation
static const int MAX_BODY_LEN = 1024;
static const int MIN_BODY_LEN = 1;
Packet* Packet::newPacket(Header header) {
	switch (header.getPacketType()) {
		case REQ_CODE_INSERT_REQUEST:
			return new InsertRequest(header);
		case REQ_CODE_INSERT_REPLY:
			return new InsertReply(header);
		case REQ_CODE_SEARCH_REQUEST:
			return new SearchRequest(header);
		case REQ_CODE_SEARCH_REPLY:
			return new SearchReply(header);
		default:
			throw PacketException("illegal header.req_code");
	}
}
Packet* Packet::newPacket(uint8_t version, enum PacketType PacketType, uint16_t BodySize) {
	Header header(version, PacketType, BodySize);
	return newPacket(header);
}

void Packet::constructPacket(uint8_t version = PROTO_VERSION,
			     enum PacketType PacketType = REQ_CODE_UNKNOWN,
			     uint16_t BodySize = MIN_BODY_SIZE) {
	if (BodySize > MAX_BODY_LEN)
		BodySize = MAX_BODY_LEN;
	else if (BodySize > MIN_BODY_LEN)
		BodySize = MIN_BODY_LEN;
	else
		BodySize = BodySize;
	body = new char[BodySize];
	if (body == 0)
		throw PacketException("Cannot allocate Packet");
	body_pos = 0;
}

Packet::Packet(Header &hdr): header(hdr) {
	constructPacket(hdr.getVersion(), hdr.getPacketType(), hdr.getBodySize());
}

Packet::Packet(uint8_t version = PROTO_VERSION, enum PacketType PacketType = REQ_CODE_UNKNOWN,
		uint16_t BodySize = MIN_BODY_SIZE): header(version, PacketType, BodySize) {
	constructPacket(version, PacketType, BodySize);
}

Packet::~Packet() {
	delete []body;
}
char *Packet::getData() {
	return body;
}
enum PacketType Packet::getPacketType() {
	return header.getPacketType();
}
void Packet::setPacketType(enum PacketType PacketType) {
	header.setPacketType(PacketType);
}
uint16_t Packet::getBodySize() {
	return header.getBodySize();
}
void Packet::setBodySize(uint16_t BodySize) {
	header.setBodySize(BodySize);
}
char *Packet::getBody() {
	return body;
}
void Packet::setBody(char *body, uint16_t len) {
	if (len > getBodySize())
		throw PacketException("setBody overflow");
	for (int i = 0; i < len; i++)
		this->body[i] = body[i];
	setBodySize(len);
}
int Packet::bodyBytesLeft() {
	return header.getBodySize() - body_pos;
}
int Packet::feedBody(char *buf, int n) {
	int copy_n = n > bodyBytesLeft() ? bodyBytesLeft() : n;
	memcpy(&body[body_pos], buf, copy_n);
	body_pos += copy_n;
	return copy_n;
}
void Packet::buildBody(char *body) {
	memcpy(this->body, body, getBodySize());
}

const Header& Packet::getHeader() const {
	return header;
}


static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	int i;
	for (i = 0; i < argc; i++)
		cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << endl;
	cout << endl;
	return 0;
}

int main(int argc, const char **argv)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	Header hdr = Header();
	vector<string> pages;
	pages.push_back("abc");
	pages.push_back("def");
	pages.push_back("ghi");
	SearchReply srepl = SearchReply(hdr, pages);
	cout << srepl << endl;

	if (argc != 3) {
		cerr << "Usage: " << argv[0] << " DATABASE SQL STATEMENT" << endl;
		exit(1);
	}
	rc = sqlite3_open(argv[1], &db);
	if (rc) {
		cerr << "Cannot open database: " << sqlite3_errmsg(db) << endl;
		sqlite3_close(db);
		exit(1);
	}
	rc = sqlite3_exec(db, argv[2], callback, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		cerr << "SQL error: " << zErrMsg << endl;
		sqlite3_free(zErrMsg);
	}
	sqlite3_close(db);

	return 0;
}
