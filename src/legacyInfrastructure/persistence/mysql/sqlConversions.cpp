#include "infrastructure/persistence/mysql/sqlConversions.hpp"

namespace hubflow::infrastructure
{

std::string SqlColumns::text(const SqlRow& row, const std::size_t column)
{
    return row.at(column).value_or("");
}

std::int64_t SqlColumns::signedNumber(const SqlRow& row, const std::size_t column)
{
    return std::stoll(row.at(column).value_or("0"));
}

std::uint64_t SqlColumns::unsignedNumber(const SqlRow& row, const std::size_t column)
{
    return std::stoull(row.at(column).value_or("0"));
}

double SqlColumns::decimal(const SqlRow& row, const std::size_t column)
{
    return std::stod(row.at(column).value_or("0"));
}

bool SqlColumns::flag(const SqlRow& row, const std::size_t column)
{
    return row.at(column).value_or("0") != "0";
}

} // namespace hubflow::infrastructure
