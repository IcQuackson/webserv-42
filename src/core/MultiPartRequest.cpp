#include "core/MultiPartRequest.hpp"

MultiPartRequest::MultiPartRequest() {
	data = "";
	boundary = "";
}

MultiPartRequest::~MultiPartRequest() {}

MultiPartRequest::MultiPartRequest(const MultiPartRequest &multiPartRequest) {
	data = multiPartRequest.data;
	boundary = multiPartRequest.boundary;
}

MultiPartRequest &MultiPartRequest::operator=(const MultiPartRequest &multiPartRequest) {
	if (this != &multiPartRequest) {
		data = multiPartRequest.data;
		boundary = multiPartRequest.boundary;
	}
	return *this;
}

std::string MultiPartRequest::getData() const {
	return data;
}

std::string MultiPartRequest::getBoundary() const {
	return boundary;
}

void MultiPartRequest::setData(const std::string& newData) {
	data = newData;
}

void MultiPartRequest::setBoundary(const std::string& newBoundary) {
	boundary = newBoundary;
}

void MultiPartRequest::appendData(const std::string& newData) {
	data += newData;
}

std::string MultiPartRequest::getBoundary(const std::string& data) {
	std::string boundary;
	std::size_t boundaryStart;
	std::size_t boundaryEnd;

	boundary = "";
	size_t contentTypePos = data.find("Content-Type: ");
    if (contentTypePos != std::string::npos) {
		size_t boundaryPos = data.find("boundary=", contentTypePos);
		if (boundaryPos != std::string::npos) {
			// Extract the boundary value
			boundaryStart = boundaryPos + std::strlen("boundary=");
			boundaryEnd = data.find("\r\n", boundaryStart);
			// Check if the boundary value is valid
			if (boundaryEnd != std::string::npos) {
				boundary = data.substr(boundaryStart, boundaryEnd - boundaryStart);
			}
		}
	}
	return boundary;
}