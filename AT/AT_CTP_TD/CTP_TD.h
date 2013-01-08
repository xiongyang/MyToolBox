#pragma once
#include <map>
#include <string>
#include "IDriver_TD.h"
#include "CTP_API.h"
#include "DataCache_CTP_TD.h"
namespace AT
{
	class ITradeSpi;
}
namespace CTP
{
	class CTP_TD_Imp;
	enum CTP_TD_CODE
	{
		UnInit,
		Connecting,
		Logining,
		LoginFail,
		ConfirmingSettlement,
		Ready,
		Disconnect,

		Cancel_Failed,
		CreateOrder_Failed,
	};


	enum CTP_INPUT_ORDER_FILED
	{
		order_name = 0,
		order_buysell,
		order_openclose,
		order_val,
		order_price,
	};

	class CTP_TD :public AT::IDriver_TD
		,public CThostFtdcTraderSpi
	{
	public:
		CTP_TD();
		virtual ~CTP_TD();
		virtual void Init(const std::map<std::string,std::string>& aConfigMap,AT::ITradeSpi* apTradeSpi) ;
	public:
		virtual std::string CreateOrder(const std::string& aNewOrder) ;
		virtual void DeleteOrder(const std::string& aClientOrderID) ;
		virtual	void ModifyOrder(const std::string& aRequest);
		virtual void QueryPosition(const std::string& aRequest);

		//from CTP not for user call
	public:
		virtual void OnFrontConnected();
		virtual void OnFrontDisconnected(int nReason);
		virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *apRspUserLogin,
			CThostFtdcRspInfoField *apRspInfo, int anRequestID, bool abIsLast);
		virtual void OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
			CThostFtdcRspInfoField *apRspInfo, int nRequestID, bool bIsLast) ;
		virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, 
			CThostFtdcRspInfoField *apRspInfo, int nRequestID, bool bIsLast);

		///报单通知
		virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);
		///成交通知
		virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);

		//删单失败？
		virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
		//下单失败
		virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);




	private:
		void  NotifyState();
		InputOrderTypePtr BuildExchangeOrder(const std::string& aNewOrder);
		std::string BuildRtnTradeStr(boost::shared_ptr<CThostFtdcTradeField> apTrade);
		std::string BuildRtnOrderStr(boost::shared_ptr<CThostFtdcOrderField> apOrder);


	private:
		CThostFtdcTraderApi*  m_pTraderAPI;
		int m_RequestID;
		AT::ITradeSpi* m_pTradeSpi;
		std::map<std::string,std::string> m_ConfigMap;
		CTP_TD_CODE			m_RuningState;
	private:
		std::string		m_BrokerID;
		std::string		m_UserID;
		std::string		m_Password;

		int				m_FrontID;
		int				m_SessionID;
		unsigned int				m_MaxOrderRef;
		DataCache_CTP_TD	m_DataCache;

	};
}


