#include "hubflow/shared/adapters/out/memory/slidingWindowRateLimiter.hpp"

#include <gtest/gtest.h>

using hubflow::shared::adapters::SlidingWindowRateLimiter;

TEST(SlidingWindowRateLimiter, AllowsUpToTheLimitAndThenBlocksWithTheRemainingWait)
{
    SlidingWindowRateLimiter adapter {3, 100};
    hubflow::shared::application::RateLimiter& limiter = adapter;

    EXPECT_TRUE(limiter.attempt("a", 1000).allowed);
    EXPECT_TRUE(limiter.attempt("a", 1010).allowed);
    EXPECT_TRUE(limiter.attempt("a", 1020).allowed);
    const auto blocked = limiter.attempt("a", 1030);

    EXPECT_FALSE(blocked.allowed);
    EXPECT_EQ(blocked.retryAfterSeconds, 70U);
}

TEST(SlidingWindowRateLimiter, TheWindowSlidesOneAttemptAtATime)
{
    SlidingWindowRateLimiter adapter {2, 100};
    hubflow::shared::application::RateLimiter& limiter = adapter;
    limiter.attempt("a", 1000);
    limiter.attempt("a", 1050);

    EXPECT_FALSE(limiter.attempt("a", 1099).allowed);
    EXPECT_TRUE(limiter.attempt("a", 1100).allowed);
    EXPECT_FALSE(limiter.attempt("a", 1101).allowed);
}

TEST(SlidingWindowRateLimiter, RejectedAttemptsDoNotExtendTheBlock)
{
    SlidingWindowRateLimiter adapter {1, 100};
    hubflow::shared::application::RateLimiter& limiter = adapter;
    limiter.attempt("a", 1000);
    for (int i = 1; i < 50; ++i)
    {
        EXPECT_FALSE(limiter.attempt("a", 1000 + i).allowed);
    }

    EXPECT_TRUE(limiter.attempt("a", 1100).allowed);
}

TEST(SlidingWindowRateLimiter, KeysAreIndependent)
{
    SlidingWindowRateLimiter adapter {1, 100};
    hubflow::shared::application::RateLimiter& limiter = adapter;

    EXPECT_TRUE(limiter.attempt("ip1|HF-A", 1000).allowed);
    EXPECT_TRUE(limiter.attempt("ip2|HF-A", 1000).allowed);
    EXPECT_TRUE(limiter.attempt("ip1|HF-B", 1000).allowed);
    EXPECT_FALSE(limiter.attempt("ip1|HF-A", 1001).allowed);
}
