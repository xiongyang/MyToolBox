#ifndef ORDER_TRADERSPI_H_
#define ORDER_TRADERSPI_H_
#pragma once

#include "api/trade/win/public/ThostFtdcTraderApi.h"

class CtpTraderSpi : public CThostFtdcTraderSpi
{
public:
   CtpTraderSpi(CThostFtdcTraderApi* api):pUserApi(api){};

	///??????????bj??l??è?d?¼jéì??????c
	virtual void OnFrontConnected();

	///?¼????
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///???????????
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///????????
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///??????????
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///???????????
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///??¼?????
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///????????
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///????
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///???????????l????ì??????c??"?????ìAPI?????l?ì????h??mc
	virtual void OnFrontDisconnected(int nReason);
		
	///??????c????d???t?ì??????c
	virtual void OnHeartBeatWarning(int nTimeLapse);
	
	///????
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

	///????
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);

public:
	///???¼??
	void ReqUserLogin(TThostFtdcBrokerIDType	appId,
	        TThostFtdcUserIDType	userId,	TThostFtdcPasswordType	passwd);
	///?????????
	void ReqSettlementInfoConfirm();
	///??????
	void ReqQryInstrument(TThostFtdcInstrumentIDType instId);
	///????????
	void ReqQryTradingAccount();
	///?????????
	void ReqQryInvestorPosition(TThostFtdcInstrumentIDType instId);
	///??¼???
  void ReqOrderInsert(TThostFtdcInstrumentIDType instId,
        TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
        TThostFtdcPriceType price,   TThostFtdcVolumeType vol);
	///??????
	void ReqOrderAction(TThostFtdcSequenceNoType orderSeq);

	// ?????????
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

  void PrintOrders();
  void PrintTrades();

private:
  CThostFtdcTraderApi* pUserApi;

};

#endif