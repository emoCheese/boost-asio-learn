#include <iostream>
#include "Session.h"
#include "Server.h"

int main()
{
	try {
		boost::asio::io_context ioc;
		using namespace std;

		Server s(ioc, 9998);
		ioc.run();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	return 0;
}
