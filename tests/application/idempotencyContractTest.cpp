#include "support/applicationHarness.hpp"

#include <gtest/gtest.h>

using namespace hubflow;
using shared::application::Idempotency;

TEST(IdempotencyContract, EveryUseCaseDeclaresItsIdempotency)
{
    EXPECT_EQ(identity::application::SignInUseCase::IDEMPOTENCY, Idempotency::NonIdempotent);
    EXPECT_EQ(identity::application::RegisterUserUseCase::IDEMPOTENCY, Idempotency::NonIdempotent);
    EXPECT_EQ(identity::application::UpdateUserUseCase::IDEMPOTENCY, Idempotency::Idempotent);
    EXPECT_EQ(logistics::application::RegisterShipmentUseCase::IDEMPOTENCY, Idempotency::NonIdempotent);
    EXPECT_EQ(logistics::application::ClassifyShipmentUseCase::IDEMPOTENCY, Idempotency::NonIdempotent);
    EXPECT_EQ(logistics::application::RescheduleShipmentUseCase::IDEMPOTENCY, Idempotency::NonIdempotent);
    EXPECT_EQ(logistics::application::DeliverShipmentUseCase::IDEMPOTENCY, Idempotency::NonIdempotent);
    EXPECT_EQ(logistics::application::DispatchNextShipmentUseCase::IDEMPOTENCY, Idempotency::NonIdempotent);
    EXPECT_EQ(sorting::application::OpenSortingBatchUseCase::IDEMPOTENCY, Idempotency::NonIdempotent);
    EXPECT_EQ(sorting::application::SortSortingBatchUseCase::IDEMPOTENCY, Idempotency::Idempotent);
    EXPECT_EQ(sorting::application::PrepareSortingBatchUseCase::IDEMPOTENCY, Idempotency::Idempotent);
    EXPECT_EQ(sorting::application::ProcessSortingBatchItemUseCase::IDEMPOTENCY, Idempotency::NonIdempotent);
    EXPECT_EQ(sorting::application::ReturnProcessedItemUseCase::IDEMPOTENCY, Idempotency::Idempotent);
    EXPECT_EQ(routing::application::AssignDeliveryRouteUseCase::IDEMPOTENCY, Idempotency::NonIdempotent);
    EXPECT_EQ(routing::application::ResolveRouteStopUseCase::IDEMPOTENCY, Idempotency::Idempotent);
}

TEST(IdempotencyContract, QueriesAreIdempotent)
{
    EXPECT_EQ(logistics::application::ListPendingShipmentsUseCase::IDEMPOTENCY, Idempotency::Idempotent);
    EXPECT_EQ(routing::application::GetRouteUseCase::IDEMPOTENCY, Idempotency::Idempotent);
    EXPECT_EQ(insights::application::CombineCodeSetsUseCase::IDEMPOTENCY, Idempotency::Idempotent);
    EXPECT_EQ(audit::application::ListAuditEventsUseCase::IDEMPOTENCY, Idempotency::Idempotent);
}
