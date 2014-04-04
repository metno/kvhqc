
#ifndef ERRORS_ERRORFLITER_HH
#define ERRORS_ERRORFLITER_HH 1

#include "access/ObsFilter.hh"

class ErrorFilter : public ObsFilter
{
public:
  ErrorFilter(bool errorsForSalen)
    : mErrorsForSalen(errorsForSalen) { }
  
  virtual bool hasSQL() const
    { return true; }
    virtual std::string acceptingSQL(const std::string& data_alias) const;

  /* \param afterSQL true if accept is called after running the SQL
   * contraints from acceptingSQL(...); if false, acceptingSQL may
   * have run or not */
  virtual bool accept(ObsData_p obs, bool afterSQL) const;

  virtual bool equals(const ObsFilter& other) const;
  virtual bool subsetOf(const ObsFilter& other) const;

private:
  bool mErrorsForSalen;
};

#endif // ERRORS_ERRORFLITER_HH
