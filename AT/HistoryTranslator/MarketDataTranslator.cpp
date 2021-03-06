
#include "MarketDataTranslator.h"
#include "SingleDBWriter.h"
#include <boost\filesystem.hpp>

#include <boost\tokenizer.hpp>
#include <boost\date_time\gregorian\gregorian.hpp>
#include <boost\date_time\posix_time\conversion.hpp>

#include <iostream>
#include <algorithm>

using namespace boost::filesystem;


MarketDataTranslator::MarketDataTranslator( const std::string& aDBstoreDIr )
	:m_DBFoudler(aDBstoreDIr)
{

}

MarketDataTranslator::~MarketDataTranslator(void)
{

}

void MarketDataTranslator::PraseDir(const std::string& aDirName)
{
	path lDirPath(aDirName);
	if(!exists(lDirPath) || !is_directory(lDirPath))
	{
		std::cerr<<"Dir is not exist"<<std::endl;
		return ;
	}

	path lstoreDDPath ( m_DBFoudler) ;
	if(!exists(lstoreDDPath))
	{
		create_directory(lstoreDDPath);
	}

	std::vector<std::string> lFileList ;
   
	for(directory_iterator iter=  directory_iterator(lDirPath) ; iter != directory_iterator() ; ++ iter)
	{
		lFileList.push_back(iter->path().string());
	}
	AT::AT_Time lbegin = AT::AT_Local_Time();
//	 #pragma omp parallel for
	for(int i = 0; i< lFileList.size(); ++i)
	{
		PraseFile(lFileList[i]);
	}
	AT::AT_Time lend = AT::AT_Local_Time();
	std::cout<<lbegin -lend;

}

void MarketDataTranslator::PraseFile(const std::string& aFileName)
{
	std::fstream lfile(aFileName,std::ios_base::in);
	if(! lfile.good())
	{
		std::cerr<<"File "<<aFileName<<" Can not open\n";
		return;
	}



	//std::string lDBPath = m_DBFoudler+ aFileName;
	// m_pDBWriter.reset(new SingleDBWriter(lDBPath.c_str()));


	path lDbPath(m_DBFoudler);
	path lStrreFilePath(aFileName);

	lDbPath /= lStrreFilePath.filename().stem();
	std::shared_ptr< AT::SingleDBHandler> lpDBWriter(new AT::SingleDBHandler(lDbPath.string().c_str()));

	std::string lInstrumentID =  lStrreFilePath.filename().stem().string().substr(0,6);

	size_t lBuffsize = 256;
	char* lineBuff = new char [lBuffsize];
	while(lfile.getline(lineBuff,lBuffsize))
	{
		PraseLine(lInstrumentID,std::string(lineBuff,std::min(strlen(lineBuff),lBuffsize)), lpDBWriter);
	}
}

void MarketDataTranslator::PraseLine(const std::string& aInstrumentID ,const std::string& aLIne,std::shared_ptr< AT::SingleDBHandler> apDBWriter)
{

	try
	{
		boost::char_separator<char> lSep(",");
		boost::tokenizer<boost::char_separator<char> > ltoken(aLIne,lSep);
		std::vector<std::string> lWordVec(ltoken.begin(),ltoken.end());
		if(lWordVec.size() != 8)
		{
			std::cerr<<"Bad Line "<< aLIne<<std::endl;
		}

		std::shared_ptr<AT::MarketData> lpMarket(new AT::MarketData);
		AT::MarketData& lRetMarket = *lpMarket;
		strcpy_s(lRetMarket.InstrumentID,sizeof(lRetMarket.InstrumentID),aInstrumentID.c_str());
		lRetMarket.m_UpdateTime = PraseTime(lWordVec[0],lWordVec[1]);
		lRetMarket.m_LastPrice = AT::TranPriceToInt(std::stod(lWordVec[2]));

		lRetMarket.m_AskPrice = AT::TranPriceToInt(std::stod(lWordVec[3]));
		lRetMarket.m_AskVol = std::stoi(lWordVec[4]);
		lRetMarket.m_BidPrice = AT::TranPriceToInt(std::stod(lWordVec[5]));
		lRetMarket.m_BidVol = std::stoi(lWordVec[6]);

		apDBWriter->StoreMarketData(lpMarket);
	}
	catch (...)
	{
		std::cerr<<"Bad Line "<< aLIne<<std::endl;
	}
}
AT::AT_Time MarketDataTranslator::PraseTime(const std::string aDate, const std::string& aTime)
{
	try
	{
		boost::gregorian::date ldate(boost::gregorian::from_undelimited_string(aDate));
		boost::char_separator<char> lSep(":.");
		boost::tokenizer<boost::char_separator<char>> ltoken(aTime,lSep);
		std::vector<std::string> lWordVec(ltoken.begin(),ltoken.end());
		int hour = std::stoi(lWordVec[0]);
		int min = std::stoi(lWordVec[1]);
		int second = std::stoi(lWordVec[2]);
		int millsecond = std::stoi(lWordVec[3]);
		boost::posix_time::time_duration lTimeDur(hour,min,second,0);
		lTimeDur += boost::posix_time::milliseconds(millsecond*100);
		AT::AT_Time ltime (ldate,lTimeDur);
		return std::move(ltime);
	}
	catch (...)
	{
		std::cerr<<"Bad Date Time "<< aDate<<"  "<< aTime<<'\n';
		throw;
	}
}

