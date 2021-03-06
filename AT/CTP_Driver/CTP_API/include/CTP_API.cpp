#include "CTP_API.h"
#include <boost\lexical_cast.hpp>
#include <iostream>
#include <sstream>
namespace CTP
{

	std::string MakePlatformOrderID(const char* aOrderRef,int aFrontID,int aSessionID)
	{
		std::string lRet;
		lRet += std::to_string(aSessionID);
		lRet += '_';
		lRet += std::to_string(aFrontID);
		lRet += '_';
		lRet += aOrderRef;
		return std::move(lRet);
	}



	std::string GenerateThostOrderID( InputOrderTypePtr aOrderPtr,int aFrontID,int aSessionID )
	{
		std::string lRet;
		lRet += boost::lexical_cast<std::string>(aSessionID);
		lRet += '_';
		lRet += boost::lexical_cast<std::string>(aFrontID);
		lRet += '_';
		lRet += boost::lexical_cast<std::string>(aOrderPtr->OrderRef);
		return lRet;
	}



	std::string GenerateClientOrderID( OrderTypePtr aOrderPtr )
	{
		std::string lRet;
		lRet += boost::lexical_cast<std::string>(aOrderPtr->OrderRef);
		return lRet;
	}

	std::string GenerateTradeID( TradeTypePtr aTradePtr )
	{
		std::string lRet;
		lRet += boost::lexical_cast<std::string>(aTradePtr->TraderID);
		return lRet;
	}

	bool IsErrorRspInfo( CThostFtdcRspInfoField *pRspInfo )
	{
		bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
		if(ret)
		{
			//TODO LOG
			std::cerr<<"Error Code="<<pRspInfo->ErrorID << "Error Message = "<<pRspInfo->ErrorMsg <<std::endl;
		}
		return ret;
	}

	std::string ResolveThostOrderID( const std::string& lThostOrderID,int& ar_SessionID, int& ar_FrointID )
	{
		std::stringstream lbuf(lThostOrderID);
		char lplaceholder;
		lbuf>>ar_SessionID;
		lbuf >>lplaceholder;
		lbuf >> ar_FrointID;
		lbuf >> lplaceholder;
		std::string lOrderRef;
		lbuf>> lOrderRef;
		return std::move(lOrderRef);
	}

	std::string MakeExchangeOrderID( const char* ExchangeID, const char* ExchangeOrderID )
	{
		std::string lRet;
		lRet += ExchangeID;
		lRet += ExchangeOrderID;
		return std::move(lRet);
	}

}
