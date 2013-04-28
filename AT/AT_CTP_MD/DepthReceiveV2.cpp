#include "DepthReceiveV2.h"

#include <sstream>
#include <iostream>
#include <boost/lexical_cast.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/date_time/gregorian/conversion.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
using namespace AT;
namespace CTP
{


	DepthReceiverV2::DepthReceiverV2( const std::string aConfigXml ,MarketHandlerFun aHandle , MarketStateHandle aStateHandle )
		: m_Markethandle(aHandle)
		, m_RequestID(1)
		, m_MarketStateHandle(aStateHandle)
	{
		std::stringstream lbuf(aConfigXml);
		using boost::property_tree::ptree;
		ptree pt;
		read_xml(lbuf,pt);

		for(auto lVale: pt.get_child("MDConfig.InstrumentList"))
		{
			m_SubcribeList.insert(lVale.second.data());
		}

		m_Front = pt.get<std::string>("MDConfig.Front");
		m_WorkFlowDir =  pt.get<std::string>("MDConfig.WorkFlowDir");
	}

	void DepthReceiverV2::Start()
	{
		//TODO check the Dir if not exist ,create it
		m_pMDAPI = CThostFtdcMdApi::CreateFtdcMdApi(/*m_WorkFlowDir.c_str()*/);
		m_pMDAPI->RegisterSpi(this);
		char buf_front[256];
		strcpy_s(buf_front,sizeof(buf_front),m_Front.c_str());
		m_pMDAPI->RegisterFront(buf_front);
		m_pMDAPI->Init();
		if(m_Markethandle)	m_MarketStateHandle(CTP_Market_Status_Enum::E_CTP_MD_CONNECTING,"开始连接\n");

		InitDelayTimer();
	}

	void DepthReceiverV2::Stop()
	{
		StopDelayTimer();
		if(m_pMDAPI)
		{
			m_pMDAPI->RegisterSpi(nullptr);
			m_pMDAPI->Join();
		}
		m_pMDAPI->Release();
	}

	void DepthReceiverV2::OnFrontConnected()
	{
		CThostFtdcReqUserLoginField lLoginReq;
		memset(&lLoginReq,0,sizeof(CThostFtdcReqUserLoginField));
		m_pMDAPI->ReqUserLogin(&lLoginReq,++m_RequestID);
		if(m_Markethandle) m_MarketStateHandle(CTP_Market_Status_Enum::E_CTP_MD_LOGIN,"连接成功开始登陆与订阅列表\n"); 
	}
	void DepthReceiverV2::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		//TODO more work about auto reconnect whether cause  recall the flow
		if(IsErrorRspInfo(pRspInfo))
		{
			return;
		}
		else
		{
			SubscribeList();
		}
	}
	void DepthReceiverV2::SubscribeList()
	{
		char buf[10] = {0};
		char* lList[1] ={0};
		std::cout<<"开始订阅行情\n";
		for(std::string lInstrumen : m_SubcribeList)
		{
			strcpy_s(buf,sizeof(buf),lInstrumen.c_str());
			lList[0] = buf;
			m_pMDAPI->SubscribeMarketData(lList,1);
			std::cout<<"订阅行情 "<< lInstrumen<< "\n";
		}
		std::cout<<"结束订阅行情列表\n";
		
	}


	void DepthReceiverV2::OnRspSubMarketData( CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast )
	{
		if(IsErrorRspInfo(pRspInfo))
		{
			std::cerr<<"行情订阅失败 Error "<< pSpecificInstrument->InstrumentID<<std::endl;
		}
		else
		{
			std::cout<<"行情订阅成功 "<<pSpecificInstrument->InstrumentID<< "  启动行情监视计时器\n";
			
		}
		if(bIsLast)
		{
			if(m_Markethandle)	m_MarketStateHandle(CTP_Market_Status_Enum::E_CTP_MD_READY,"开始接受行情 启动行情监视计时器\n");
			ResetDelayTimer();
		}
	}


	void DepthReceiverV2::OnRtnDepthMarketData( CThostFtdcDepthMarketDataField *pDepthMarketData )
	{
		ResetDelayTimer();
		std::shared_ptr<AT::MarketData> lBuildMarket = Build_AT_Market(pDepthMarketData);
		m_Markethandle(lBuildMarket);
	}

	void DepthReceiverV2::OnFrontDisconnected( int nReason )
	{
		std::string ErrMessage;
		switch(nReason)
		{
		case 0x1001:
			ErrMessage = "网络读失败";
			break;
		case 0x1002:
			ErrMessage = "网络写失败";
			break;
		case 0x2001:
			ErrMessage = "接收心跳超时";
			break;
		case 0x2002:
			ErrMessage = "发送心跳失败";
			break;
		case 0x2003:
			ErrMessage = "收到错误报文";
			break;
		default:
			ErrMessage = "其他错误序号";
			ErrMessage += boost::lexical_cast<std::string>(nReason);
			break;
		}
		std::cerr<<"Market===================== 断开连接 稍后将会重新连接 "<<ErrMessage<<std::endl;
		if(m_Markethandle) m_MarketStateHandle(CTP_Market_Status_Enum::E_CTP_MD_CONNECTING,"连接中断\n");
	}

	bool DepthReceiverV2::IsValidPrice( double aPrice )
	{
		//note 1.79769e+308; is the max val .
		return aPrice < 1.79768e+308;
	}

	double DepthReceiverV2::GetDispalyPrice( double aPrice )
	{
		return IsValidPrice(aPrice)? aPrice: -1;
	}

	DepthReceiverV2::~DepthReceiverV2()
	{

	}

	std::shared_ptr<AT::MarketData> DepthReceiverV2::Build_AT_Market( CThostFtdcDepthMarketDataField* aMarketPtr )
	{
		std::shared_ptr<AT::MarketData> lRet(new AT::MarketData);
		AT::MarketData& lMarket = *lRet;
		memcpy(lMarket.InstrumentID,aMarketPtr->InstrumentID,sizeof(aMarketPtr->InstrumentID));
		lMarket.m_UpdateTime = Build_AT_Time( aMarketPtr->UpdateTime, aMarketPtr->UpdateMillisec);

		lMarket.m_LastPrice =  IsValidPrice( aMarketPtr->LastPrice)?  TranPriceToInt( aMarketPtr->LastPrice) : AT_INVALID_PRICE;
		lMarket. m_BidPrice =  IsValidPrice( aMarketPtr->BidPrice1)? TranPriceToInt( aMarketPtr->BidPrice1) : AT_INVALID_PRICE;
		lMarket. m_AskPrice  =  IsValidPrice( aMarketPtr->AskPrice1)? TranPriceToInt( aMarketPtr->AskPrice1) : AT_INVALID_PRICE;
	//	lRet.m_trunover = IsValidPrice( aMarketPtr->AskPrice1)?  TranPriceToInt( aMarketPtr->Turnover):AT_INVALID_PRICE;
		lMarket.m_AskVol  = aMarketPtr->AskVolume1;
		lMarket. m_BidVol  = aMarketPtr->BidVolume1;

		return lRet;

	}

	AT::AT_Time DepthReceiverV2::Build_AT_Time( TThostFtdcTimeType aSecond, int millsecond )
	{
		try
		{
			boost::gregorian::date ldate = boost::gregorian::day_clock::local_day();
			boost::posix_time::time_duration lTimeSecond = boost::posix_time::duration_from_string(aSecond);
			boost::posix_time::milliseconds ltimeMill(millsecond);
			AT::AT_Time atime (ldate , lTimeSecond );
			atime +=  ltimeMill;
			return std::move(atime);

		}
		catch (...)
		{
			return AT::AT_INVALID_TIME;
		}

	}

	void DepthReceiverV2::CheckMarketDelay( const boost::system::error_code& errCOde )
	{
		if(errCOde == boost::asio::error::operation_aborted )
			return;
		if(m_MarketStateHandle) m_MarketStateHandle(CTP_Market_Status_Enum::E_CTP_MD_MARKET_DELAY,"Delay Makret");
	}

	void DepthReceiverV2::ResetDelayTimer()
	{
		m_pCheckDelayTimer->cancel();
		m_pCheckDelayTimer->expires_from_now(boost::posix_time::seconds(2));

		auto CheckHandle = [this]( const boost::system::error_code& errCOde)
		{
			this->CheckMarketDelay(errCOde);
		};

		m_pCheckDelayTimer->async_wait(CheckHandle);
	}

	void DepthReceiverV2::InitDelayTimer()
	{
		m_pBoostIOService.reset(new boost::asio::io_service);
		m_pBoostWorker.reset(new boost::asio::io_service::work(*m_pBoostIOService));
		m_CheckDelayThread = std::thread(boost::bind(&boost::asio::io_service::run,m_pBoostIOService.get()));
		m_pCheckDelayTimer.reset(new boost::asio::deadline_timer(*m_pBoostIOService));
	}

	void DepthReceiverV2::StopDelayTimer()
	{
		m_pCheckDelayTimer->cancel();
		m_pBoostWorker.reset();
		if(m_CheckDelayThread.joinable()) m_CheckDelayThread.join();
	}



}
