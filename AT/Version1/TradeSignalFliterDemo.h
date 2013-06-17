#pragma once
#include "ITradeSignalFliter.h"
namespace AT
{


class TradeSignalFliterDemo :public ITradeSignalFliter
{
public:
	TradeSignalFliterDemo(void);
	virtual ~TradeSignalFliterDemo(void);

public:
	virtual TradeSignal FliterTradeSignal(std::vector<TradeSignal> aList) override;

	virtual void UpdateTime(AT_Time aTime )override;
	virtual void UpdateProfit(int32_t aProfit,AT_Time aTime) override;
};

}