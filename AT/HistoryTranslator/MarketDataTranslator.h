#ifndef MARKETDATATRANSLATOR_H_INCLUDED
#define MARKETDATATRANSLATOR_H_INCLUDED
#include "AT_Struct.h"

#include <string>
namespace AT
{
	struct  MarketData;
}
class SingleDBWriter;

class MarketDataTranslator
{
public:
    MarketDataTranslator(const std::string& aDBstoreDIr);
    virtual ~MarketDataTranslator(void);


    void PraseDir(const std::string& aDirName);
    void PraseFile(const std::string& aFileName);
    void PraseLine(const std::string& aLIne);
    AT::AT_Time PraseTime(const std::string aDate, const std::string& aTime);

private:
    std::auto_ptr<SingleDBWriter>   m_pDBWriter;
    std::string                     m_DBFoudler;

};

#endif // MARKETDATATRANSLATOR_H_INCLUDED
