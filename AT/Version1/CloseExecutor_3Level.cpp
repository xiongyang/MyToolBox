#include "CloseExecutor_3Level.h"
#include "IDriver_TD.h"
#include "ATLogger.h"
#include "ExecutorFactory.h"

using namespace std;
namespace AT
{

CloseExecutor_3Level::CloseExecutor_3Level( const std::string& aConfigFile )
:m_IsTriggered(false)
,m_CurrentLevel(CheckStatusLevel::Level0)
,m_MaxPriceDiff(0)
,m_StartPrice(0)
,m_TradeQuantity(0)
{
	InitFromConfigFile(aConfigFile);
}
CloseExecutor_3Level::~CloseExecutor_3Level(void)
{
}

void CloseExecutor_3Level::AddExecution( ExecutorInput aExecutorInput )
{
	if (m_Status.IsFinised == false)
	{
		ATLOG(L_ERROR,"The CloseExecutor can only handle 1 ExecutorInput, the More should create new instance handle");
		return;
	}
	m_Status.AddTastVol = aExecutorInput.vol;
	m_Status.IsFinised = false;
	m_pFirstExecutor->AddExecution(aExecutorInput);

	m_BuySellCode = aExecutorInput.BuySellCode;
	m_InstrumentID  = aExecutorInput.InstrumentID;
	m_ExecutorInput = aExecutorInput;

	m_MaxPriceDiff = 0;
	m_StartPrice = aExecutorInput.TriggerMarketData.m_LastPrice;
	
}

void CloseExecutor_3Level::Abrot()
{
	if (m_Status.IsFinised)
	{
		ATLOG(L_INFO,"CloseExecutor_3Level Is Already Finished or not start yet");
		return;
	}

	m_pFirstExecutor->Abrot();
	m_pQuitExecutor->Abrot();
}

void CloseExecutor_3Level::OnMarketDepth( const AT::MarketData& aMarketDepth )
{
	if (m_Status.IsFinised)
	{
		return;
	}

	if (aMarketDepth.InstrumentID != m_InstrumentID)
	{
		return;
	}

	if(!m_IsTriggered)
	{
		m_IsTriggered = CheckTrigger(aMarketDepth);
		if(m_IsTriggered)
		{
			m_pFirstExecutor->Abrot();
		}
	}

	m_pFirstExecutor->OnMarketDepth(aMarketDepth);
	m_pQuitExecutor->OnMarketDepth(aMarketDepth);

}

void CloseExecutor_3Level::OnRtnTrade( const AT::TradeUpdate& apTrade )
{
	if (m_Status.IsFinised)
	{
		return;
	}

	m_pFirstExecutor->OnRtnTrade(apTrade);
	m_pQuitExecutor->OnRtnTrade(apTrade);
}

void CloseExecutor_3Level::OnRtnOrder( const AT::OrderUpdate& apOrder )
{
	if (m_Status.IsFinised)
	{
		return;
	}

	if(m_pFirstExecutor->GetExecutionStatus().IsFinised == false)
	{
		m_pFirstExecutor->OnRtnOrder(apOrder);
		if(m_pFirstExecutor->GetExecutionStatus().IsFinised == true && m_IsTriggered)
		{
			ExecutorInput lQuitExecutorInput =  BuildQuitExecution();
			m_pQuitExecutor->AddExecution(lQuitExecutorInput);
		}
	}
	m_pQuitExecutor->OnRtnOrder(apOrder);
	m_Status.IsFinised = CheckIsFinished();

}

bool CloseExecutor_3Level::CheckIsFinished()
{
	if(m_pFirstExecutor->GetExecutionStatus().IsFinised && m_pQuitExecutor->GetExecutionStatus().IsFinised )
	{
		return true;
	}
	else
	{
		return false;
	}
}

AT::ExecutionStatus CloseExecutor_3Level::GetExecutionStatus()
{
	return m_Status;
}

std::string CloseExecutor_3Level::GetExecutorID()
{
	return m_ExecutorID;
}

void CloseExecutor_3Level::HandleFirstExecutorResult( ExecutionResult aTrade )
{
	m_TradeQuantity += aTrade.vol;
	m_TradeReport(aTrade);
}

void CloseExecutor_3Level::HandleQuitExecutorResult( ExecutionResult aTrade )
{
	m_TradeReport(aTrade);
}

bool CloseExecutor_3Level::CheckTrigger( const AT::MarketData& aMarketDepth )
{
	int lPriceDiffStart = aMarketDepth.m_LastPrice - m_StartPrice;
	

	int AbsPriceDiff = lPriceDiffStart;
	if (m_BuySellCode == BuySellType::BuyOrder)
	{
		AbsPriceDiff = -lPriceDiffStart;
	}

	 UpdatePriceLevel(AbsPriceDiff);

	 m_MaxPriceDiff = max(m_MaxPriceDiff,AbsPriceDiff);
	 int FallBackFromHigh = m_MaxPriceDiff - AbsPriceDiff;
	 
	 switch (m_CurrentLevel)
	 {
	 case CheckStatusLevel::Level1:
		 if (FallBackFromHigh >= m_Setting.QuitLevel_1)
		 {
			return true;
		 }
		 break;
	 case  CheckStatusLevel::Level2:
		 if (FallBackFromHigh >= m_Setting.QuitLevel_2)
		 {
			return true;
		 }
		 break;
	 case CheckStatusLevel::Level3:
		 if(FallBackFromHigh >= m_Setting.QuitLevel_3)
		 {
			 return true;
		 }
		 break;
	 default:
		 break;
	 }

	 return false;

}

void CloseExecutor_3Level::UpdatePriceLevel( int AbsPriceDiff )
{
	switch (m_CurrentLevel)
	{
	case CheckStatusLevel::Level0:
		if (AbsPriceDiff >= m_Setting.EnterLevel_1)
		{
			m_CurrentLevel  = CheckStatusLevel::Level1;
		}
		break;
	case  CheckStatusLevel::Level1:
		if (AbsPriceDiff >= m_Setting.EnterLevel_2)
		{
			m_CurrentLevel  = CheckStatusLevel::Level2;
		}
		break;
	case CheckStatusLevel::Level2:
		if(AbsPriceDiff >= m_Setting.EnterLevel_3)
		{
			m_CurrentLevel  = CheckStatusLevel::Level3;
		}
		break;
	case CheckStatusLevel::Level3:
		break;
	default:
		break;
	}
}

void CloseExecutor_3Level::InitFromConfigFile( const std::string& aConfig )
{
	boost::property_tree::ptree lConfigPtree;
	read_xml(aConfig,lConfigPtree);
	InitCheckLevelSetting(lConfigPtree);

	InitChildExecutor(lConfigPtree);

}

AT::ExecutorInput CloseExecutor_3Level::BuildQuitExecution()
{

	int		leftVol = m_Status.AddTastVol - m_TradeQuantity;
	ExecutorInput lret = m_ExecutorInput;
	lret.vol = leftVol;
	return lret;
}

void CloseExecutor_3Level::InitChildExecutor( boost::property_tree::ptree &lConfigPtree )
{
	std::string StopLossExecutorType = lConfigPtree.get<std::string>("ExecutorConfig.StopLossExecutorType");
	std::string StopLossExecutorConfig = lConfigPtree.get<std::string>("ExecutorConfig.StopLossExecutorConfig");

	std::string QuitExecutorType = lConfigPtree.get<std::string>("ExecutorConfig.QuitExecutorType");
	std::string QuitExecutorConfig = lConfigPtree.get<std::string>("ExecutorConfig.QuitExecutorConfig");

	m_pFirstExecutor = ExecutorFactory::CreateExecutor(StopLossExecutorType,StopLossExecutorConfig);
	m_pQuitExecutor = ExecutorFactory::CreateExecutor(QuitExecutorType,QuitExecutorConfig);

	m_pFirstExecutor->SetCommandHandler(m_CommandHandle);
	m_pQuitExecutor->SetCommandHandler(m_CommandHandle);

	boost::function<void(ExecutionResult aTrade)> FirstTradeReprotFun = boost::bind(&CloseExecutor_3Level::HandleFirstExecutorResult,this,_1);
	boost::function<void(ExecutionResult aTrade)> QuitTradeReprotFun = boost::bind(&CloseExecutor_3Level::HandleQuitExecutorResult,this,_1);

	m_pFirstExecutor->SetTradeReportCallback(FirstTradeReprotFun);
	m_pQuitExecutor->SetTradeReportCallback(QuitTradeReprotFun);
}

void CloseExecutor_3Level::InitCheckLevelSetting( boost::property_tree::ptree &lConfigPtree )
{
	m_ExecutorID = lConfigPtree.get<std::string>("ExecutorConfig.ExecutorID");

	m_Setting.EnterLevel_1 = lConfigPtree.get<int>("ExecutorConfig.LevelEnter_1");
	m_Setting.QuitLevel_1 = lConfigPtree.get<int>("ExecutorConfig.LevelBack_1");
	m_Setting.EnterLevel_2 = lConfigPtree.get<int>("ExecutorConfig.LevelEnter_2");
	m_Setting.QuitLevel_2 = lConfigPtree.get<int>("ExecutorConfig.LevelBack_2");
	m_Setting.EnterLevel_3 = lConfigPtree.get<int>("ExecutorConfig.LevelEnter_3");
	m_Setting.QuitLevel_3 = lConfigPtree.get<int>("ExecutorConfig.LevelBack_3");
}


}