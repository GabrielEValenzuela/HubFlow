#include "bootstrap/shutdownSignal.hpp"

#include <csignal>

namespace hubflow::bootstrap
{
namespace
{
volatile std::sig_atomic_t gs_requested {0};

void onSignal(int)
{
    gs_requested = 1;
}
} // namespace

void ShutdownSignal::install()
{
    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);
}

bool ShutdownSignal::requested() noexcept
{
    return gs_requested != 0;
}

} // namespace hubflow::bootstrap
