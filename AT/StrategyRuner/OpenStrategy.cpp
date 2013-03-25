#include "OpenStrategy.h"
#include <sstream>
#include <boost\tokenizer.hpp>
#include <myForeach.h>

using boost::posix_time::time_duration ;


static std::string  g_Instument = "IF1304";
static int    g_timeDiv = 5;


time_duration  Parser( const std::string& aMarket , double& aRefPrice )
{
	boost::char_separator<char> sep(" ");
	boost::tokenizer<boost::char_separator<char>> lTok(aMarket,sep);
	std::vector<std::string> lStrList(lTok.begin(),lTok.end());

	aRefPrice  = std::stod( lStrList[3]);
	time_duration  lSecond = boost::posix_time::duration_from_string (lStrList[1]);
	lSecond += boost::posix_time::millisec(std::stoi(lStrList[2]));
	return lSecond;
}



OpenStrategy::OpenStrategy(void)
{
}


OpenStrategy::~OpenStrategy(void)
{
}

void OpenStrategy::OnMarketDepth( const std::string& aMarketDepth )
{


	if(aMarketDepth.find(g_Instument.c_str())  == aMarketDepth.size())
		return;

	double lLastPrice ;
	
	time_duration lthisTicktime = Parser(aMarketDepth,  lLastPrice);
	for(auto each = m_MarketCache.begin();each!= m_MarketCache.end();)
	{
		if((lthisTicktime - each->first ) > boost::posix_time::minutes(5))
		{
			each = m_MarketCache.erase (each);
		}	
		else
		{
			each++;
		}
	}


	m_MarketCache[lthisTicktime] = lLastPrice;

	if(m_isPlaceOrder)
		return ;


	double low =100000;
	time_duration lowtime;
	MYFOREACH(each , m_MarketCache)
	{
		if(each.second < low ) 
		{
			low =each.second;
			lowtime =each.first;
		}

		if(each.second -low > 5)
		{
			std::cout<< "At time"<< each.first << "Price "<< low << "and Time "<<lthisTicktime << "Price "<< lLastPrice;	
			
			std::string lOrderStr ("IF1304 ");
			lOrderStr += "buy ";
			lOrderStr += "open ";
			lOrderStr += "1 ";
			lOrderStr += std::to_string(lLastPrice);
			m_placePrice = lLastPrice;
			m_ActiveOrder = m_pTD->CreateOrder(lOrderStr);
			std::cout<<"place order ";
			break;
		}

	}
}

void OpenStrategy::OnRtnOrder( const std::string& apOrder )
{
	//std::stringstream lbuf(apOrder);
	//std::string lorderID ;
	//lbuf >> lorderID;
	//int		OrderStates;

}

void OpenStrategy::OnRtnTrade( const std::string& apTrade )
{
	//todo check the Traded Order ID
	m_ExitHandle(m_placePrice);
	//todo held ok

}

void OpenStrategy::Reload()
{
	m_isPlaceOrder = false;
	m_MarketCache.clear();
}
