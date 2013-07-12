
#include <planet/tcp/common.hpp>

namespace planet {
namespace net {
namespace tcp {


    namespace path_reg {
        boost::regex ctl            {R"(/tcp/(\d+)/ctl)"};
        boost::regex data           {R"(/tcp/(\d+)/data)"};
        boost::regex local          {R"(/tcp/(\d+)/local)"};
        boost::regex remote         {R"(/tcp/(\d+)/remote)"};
        boost::regex session_dir    {R"(/tcp/(\d+))"};
    }   // namespace path_reg


}   // namespace tcp
}   // namespace net
}   // namespace planet
