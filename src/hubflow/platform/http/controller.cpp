#include "hubflow/platform/http/controller.hpp"

namespace hubflow::platform
{

    Json Controller::page(const Json& items, const std::size_t limit, const std::size_t offset)
    {
        Json slice = Json::array();
        for (std::size_t itemIndex = offset; itemIndex < items.size() && itemIndex < offset + limit; ++itemIndex)
        {
            slice.push_back(items.at(itemIndex));
        }
        return Json {{"items", slice}, {"total", items.size()}, {"limit", limit}, {"offset", offset}};
    }

    std::string Controller::locationOf(const std::string& resourcePath)
    {
        return std::string {ApiRouter::PREFIX} + resourcePath;
    }

} // namespace hubflow::platform
