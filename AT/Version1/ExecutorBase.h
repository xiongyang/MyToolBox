#pragma once
#include "IExecutor.h"
#include <boost\property_tree\ptree.hpp>
namespace AT
{


	//这个类是为了消除各个Executor里面的重复内容
	//提供以下的功能
	// ExecutionID 读取初始化
	// 1 品种过滤
	// 2 订单成交过滤


class ExecutorBase :public IExecutor
{
public:
	ExecutorBase(const boost::property_tree::ptree& aConfig);
	ExecutorBase(const std::string& aConfigFile);
	virtual ~ExecutorBase(void);

public:
	//输入1 来自于命令
	virtual	void AddExecution(ExecutorInput aExecutorInput) final; //添加新任务
	virtual void Abrot() final;	
	virtual	void OnMarketDepth(const AT::MarketData& aMarketDepth)final;
	virtual void OnRtnOrder(const  AT::OrderUpdate& apOrder) final;
	virtual void OnRtnTrade(const  AT::TradeUpdate& apTrade)final;
	virtual void SetTradeReportCallback (TradeReportFun aFinishCallback)  final;
	virtual	void SetCommandHandler(CommandHandler aCommandHandler) final;

	virtual std::string GetExecutorID() override;
	virtual ExecutionStatus GetExecutionStatus() override ;

protected:
	virtual	void DoAddExecution(ExecutorInput aExecutorInput) =0; //添加新任务
	virtual void DoAbrot()  =0;	
	virtual	void DoOnMarketDepth(const AT::MarketData& aMarketDepth) =0;
	virtual void DoOnRtnOrder(const  AT::OrderUpdate& apOrder)  =0;
	virtual void DoOnRtnTrade(const  AT::TradeUpdate& apTrade) =0;


private:
	void	CommandPoster(Command);
	CommandHandler				m_RealCommandHandle;

protected:
	void		SetupTradeInfoBase( const AT::OrderUpdate &aOrder );
	CommandHandler				m_CommandHandle;
	TradeReportFun				m_TradeReport;

	std::string					m_ExecutorIDBase;
	std::string					m_InstrumentIDBase;
	AT_Order_Key				m_OrderKeyBase;
	ExecutionStatus				m_ExecutionStatusBase;



};

}