#include "BoostTools.h"
#include "MainTools.h"

//Server Class Methods

size_t readMessage(std::shared_ptr<tcp::socket> socket, char* buffer, int bufferLen, boost::system::error_code err) {
	size_t bytesRead = boost::asio::read(*socket, boost::asio::buffer(buffer, bufferLen), err);
	if (err == boost::asio::error::eof || err == boost::asio::error::connection_reset || err == boost::asio::error::connection_aborted) {
		throw heavyException("Client disconnected.\n");
	}
	else if (err) {
		throw lightException("Error while reading message.\n");
	}

	return bytesRead;
}

size_t writeMessage(std::shared_ptr<tcp::socket> socket, const char* buffer, int bufferLen, boost::system::error_code err) {
	size_t bytesRead = boost::asio::write(*socket, boost::asio::buffer(buffer, bufferLen), err);
	if (err == boost::asio::error::eof || err == boost::asio::error::connection_reset || err == boost::asio::error::connection_aborted) {
		throw heavyException("Client disconnected.\n");
	}
	else if (err) {
		throw lightException("Error while writing message.\n");
	}

	return bytesRead;
}