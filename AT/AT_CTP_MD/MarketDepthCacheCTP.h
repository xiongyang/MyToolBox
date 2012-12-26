#pragma once
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
struct CThostFtdcDepthMarketDataField;


namespace leveldb
{
	class DB;
}

namespace CTP
{
	typedef 	boost::shared_ptr<CThostFtdcDepthMarketDataField> MarketDataPtr;
	typedef		std::vector<MarketDataPtr>						MarketDataPtrVec;
	class MarketDepthCacheCTP
	{
	public:
		
		MarketDepthCacheCTP(void);
		virtual ~MarketDepthCacheCTP(void);

		void InsertMarketTick( MarketDataPtr  aTickData);
		void InitWithDB(const std::string& aDBPath);

	private:
		std::string GenerateTickKey( MarketDataPtr  aTickData);
		void LoadFromDB();

	private:
		std::map<std::string,MarketDataPtr>		m_MarketDataMap;
		leveldb::DB*							m_pDB;
	};



}

