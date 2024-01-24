#include <iostream>
#include <string>
#include <cstring>

class MultiPartRequest {

private:
	std::string data;
	std::string boundary;

public:
	MultiPartRequest();
	~MultiPartRequest();
	MultiPartRequest(const MultiPartRequest &multiPartRequest);
	MultiPartRequest &operator=(const MultiPartRequest &multiPartRequest);
	
	std::string getData() const;
	std::string getBoundary() const;
	static std::string getBoundary(const std::string& data);

	void setData(const std::string& data);
	void setBoundary(const std::string& boundary);
	void appendData(const std::string& data);
};
