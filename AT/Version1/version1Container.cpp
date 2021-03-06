#include "version1Container.h"
#include "IndexContainer.h"
#include "ISignalProducer.h"
#include "ISignalFliter.h"
#include "IExecutor.h"
#include <boost\bind.hpp>
#include "FilterSystemLevel.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "ISignalModule.h"
#include <windows.h>
#include <boost\format.hpp>
#include "TradeSignalProducerDemo1.h"
#include "../AT_Driver/ATLogger.h"
namespace AT
{


version1Container::version1Container(const char* aConfigFile, AT::IDriver_TD* apTD, AT::IStrategySpi* aStrSpi, const  AT::IMarketCache* apMarketCache)
	:m_pTD(apTD)
	,m_pMarketCache(apMarketCache)
	,m_TradeAccountContaner(aConfigFile,apTD)
{

	 InitIndexContainer();
	 InitFliter();
	 InitAccountContainer();
	 InitSignalProducer();
}


version1Container::~version1Container(void)
{
}

void version1Container::OnMarketDepth( const AT::MarketData& aMarketDepth )
{
	UpdateSubPartMarket(aMarketDepth);

	std::vector<Signal> lTradeSignalVec = ProduceSignal(aMarketDepth);
	Signal lFinalSignal = m_pTradeSignalFliter->FliterTradeSignal(lTradeSignalVec);
	
	if(lFinalSignal.m_Valid)
	{
		m_TradeAccountContaner.HandleTradeSignal(lFinalSignal);
	}
	m_TradeAccountContaner.OnMarketDepth(aMarketDepth);

}

std::vector<Signal> version1Container::ProduceSignal( const AT::MarketData& lNow )
{
	std::vector<Signal> lSignalResult;
	Signal tradeSignal;
	for(auto lpSignalProducer :m_TradeSignalProducerVec)
	{
		tradeSignal = lpSignalProducer->ProduceSignal(lNow);
		if(tradeSignal.m_Valid)
		{
			lSignalResult.push_back(tradeSignal);
		}
	}
	return lSignalResult;
}

void version1Container::UpdateSubPartMarket( const AT::MarketData& aMarketDepth )
{
	m_pIndexContaner->OnMarketDepth(aMarketDepth);
	m_pTradeSignalFliter->OnMarketDepth(aMarketDepth);
	//m_TradeAccountContaner.OnMarketDepth(aMarketDepth);
}

void version1Container::OnRtnOrder( const OrderUpdate& apOrder )
{
	m_TradeAccountContaner.OnRtnOrder(apOrder);
}

void version1Container::OnRtnTrade( const TradeUpdate& apTrade )
{
	m_TradeAccountContaner.OnRtnTrade(apTrade);
}

void version1Container::InitIndexContainer()
{
	m_pIndexContaner =  new IndexContainer("IndexContainer.xml",m_pMarketCache);
}

void version1Container::InitAccountContainer()
{
	//m_TradeAccountContaner.SetProfitCallback(boost::bind(&ITradeSignalFliter::UpdateProfit,m_pTradeSignalFliter,_1,_2));
}

void version1Container::InitFliter()
{
	m_pTradeSignalFliter = new FilterSystemLevel();
}
void version1Container::InitSignalProducer()
{
	boost::property_tree::ptree lConfig;
	read_xml("TradeSignalProduceRule.xml",lConfig);

	for( std::pair<std::string,boost::property_tree::ptree>  lSignalConfig : lConfig.get_child("TradeSignalProduceRule"))
	{
		std::string lTradeSignalProducerType  = lSignalConfig.second.get<std::string>("SignalType"); // useless because all info store in
		std::string lSignalConfigFile =  lSignalConfig.second.get<std::string>("ConfigFile");
		if(lTradeSignalProducerType == "TradeSignalProducerDemo1")
		{
			m_TradeSignalProducerVec.push_back(new TradeSignalProducerDemo1(lSignalConfigFile,m_pIndexContaner));
		}

	}


	
}
void version1Container::Start()
{
	m_pIndexContaner->Start();

}
void version1Container::Stop()
{
	for (auto lSignal:m_TradeSignalProducerVec)
	{
		TradeSignalProducerDemo1* lpProducer = reinterpret_cast<TradeSignalProducerDemo1*>(lSignal);
		lpProducer->WriteTradeSignal();
	}
	m_pIndexContaner->Stop();
	//for(auto lHandle :m_LibHandleVec)
	//{
	//	FreeLibrary(lHandle);
	//}
}

}