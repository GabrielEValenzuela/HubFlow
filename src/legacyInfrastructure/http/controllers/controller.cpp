#include "infrastructure/http/controllers/controller.hpp"

namespace hubflow::infrastructure
{

Json Controller::page(const Json& items, const std::size_t limit, const std::size_t offset)
{
    Json slice = Json::array();
    for (std::size_t i = offset; i < items.size() && i < offset + limit; ++i)
    {
        slice.push_back(items.at(i));
    }
    return Json {{"items", slice}, {"total", items.size()}, {"limit", limit}, {"offset", offset}};
}

} // namespace hubflow::infrastructure
