#include "includes/ConfigParser.hpp"
#include "includes/Server.hpp"
#include "includes/Webserv.hpp"

int main(int argc, char **argv)
{
	try
	{
		ConfigParser parser(argc, argv);
		Server webserv(parser.getServers());

		webserv.run();
	}
	catch (std::exception &e)
	{
		outputLogs("main error: " + std::string(e.what()));
		std::cout << e.what() << std::endl;
	}
	return (0);
}
