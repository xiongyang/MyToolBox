#pragma once
#include "IStrategy.h"
#include "OpenStrategy.h"
#include "CloseStrategy.h"
#include <string>
class StrDemo1 :public AT::IStrategy
{
public:
	StrDemo1();
	virtual ~StrDemo1();

	virtual void NotifyState(const std::string& aErrorCode);
	virtual void SetStrategyPram(const std::string& apStrParam);
public:
	virtual void OnMarketDepth(const std::string& aMarketDepth);
	virtual void OnRtnOrder(const std::string& apOrder);
	virtual void OnRtnTrade(const std::string& apTrade);
	virtual void OnRtnPos(const std::string& aPos);

	void SetupChild();

	void ChangeToOpenState()
	{
		
		m_openState.Reload();
		m_CurrentState = &m_openState;
	}

	void ChangeToClostState(double aPrice,bool isSell)
	{
		m_clostState.Reload();
		m_clostState.SetStartPrice(aPrice,isSell);
		m_CurrentState = &m_clostState;
	}
	virtual void Start() {m_isRuning = true;};
	virtual void Stop() {m_isRuning = false;};

private:
	bool		m_isRuning;
	IStrategy* m_CurrentState;
	OpenStrategy m_openState;
	CloseStrategy m_clostState;
};

