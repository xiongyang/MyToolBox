
#include <fstream>
#include <string>
#include "CTP_MD.h"
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include "PrintMDSpi.h"
int main()
{
	CTP::CTP_MD linst;
	std::map<std::string,std::string> lConfigMap;

	//std::fstream lConfigFile("config.ini");
	//std::string lSubListStr;
	//std::string lbufl;
	//lConfigFile>>lbufl;
	//lSubListStr.append( lbufl);
	//while(lConfigFile>>lbufl)
	//{
	//	lSubListStr.append( lbufl);
	//}


	lConfigMap["EnableStateReceiver"] = "0";
	lConfigMap["EnableSubscribeList"] = "1";
	lConfigMap["SubscribeList"] = "IF1304 IF1305 IF1306 IF1307";
	//lConfigMap["SubscribeList"] = lSubListStr;


	PrintMDSpi lPrintMD;
	linst.Init(lConfigMap,&lPrintMD);
	while(true)
	{
		boost::this_thread::sleep(boost::posix_time::millisec(1000));
	}
}