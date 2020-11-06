#ifndef BOOSTTOOLS_H
#define BOOSTTOOLS_H

#include "stdafx.h"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

//Communication Constants
const int PORT = 27015;

//Server Class
/*
class ServerPAUI {
public:
	ServerPAUI(boost::asio::io_service& ioContext, short port)
		: ioService(ioContext),
		  clientAcceptor(ioContext, tcp::endpoint(tcp::v4(), port)),
		  clientSocket(new tcp::socket(ioContext)) {
		clientAcceptor.accept(*clientSocket);
	}
	~ServerPAUI();
	size_t readMessage(char* buf, int bufLen);
	size_t writeMessage(char* buf, int bufLen);
	bool checkConnection();
	void disableNagle();
private:
	boost::asio::io_service& ioService;
	boost::system::error_code errCode;
	std::shared_ptr<tcp::socket> clientSocket;
	tcp::acceptor clientAcceptor;
};
*/
//Boost I/O Reduced Functions
size_t readMessage(std::shared_ptr<tcp::socket> socket, char* buffer, int bufferLen, boost::system::error_code err);
size_t writeMessage(std::shared_ptr<tcp::socket> socket, const char* buffer, int bufferLen, boost::system::error_code err);

#endif