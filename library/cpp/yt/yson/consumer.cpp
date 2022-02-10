#include "consumer.h" 
 
#include <library/cpp/yt/yson_string/string.h> 
 
namespace NYT::NYson { 
 
//////////////////////////////////////////////////////////////////////////////// 
 
void IYsonConsumer::OnRaw(const TYsonStringBuf& yson)
{
    OnRaw(yson.AsStringBuf(), yson.GetType()); 
} 
 
//////////////////////////////////////////////////////////////////////////////// 
 
} // namespace NYT::NYson 
