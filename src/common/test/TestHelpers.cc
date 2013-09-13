
#include "TestHelpers.hh"

#include "ObsHelpers.hh"

::testing::AssertionResult AssertCorrControl(const char* ec_expr, const char* eci_expr, const char* a_expr,
    float ec, const std::string& eci, const ObsDataPtr& a)
{
  ::testing::Message msg;
  if (not a) {
    msg << "(no obs " << a_expr << ")";
    return ::testing::AssertionFailure(msg);
  }
  bool failed = false;
  if (not Helpers::float_eq()(ec, a->corrected())) {
    msg << "(corrected ex:" << ec_expr << " != ac:" << a->corrected() << ")";
    failed = true;
  }
  if( eci != a->controlinfo().flagstring() ) {
    if( failed )
      msg << "; ";
    msg << "(controlinfo ex:" << eci_expr << " != ac:" << a->controlinfo().flagstring() << ")";
    failed = true;
  }
  return failed ? ::testing::AssertionFailure(msg) : ::testing::AssertionSuccess();
}
