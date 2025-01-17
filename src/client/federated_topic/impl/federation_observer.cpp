#include <src/api/grpc/ydb_federation_discovery_v1.grpc.pb.h>

#include <src/client/federated_topic/impl/federation_observer.h>

namespace NYdb::NFederatedTopic {

constexpr TDuration REDISCOVERY_DELAY = TDuration::Seconds(30);

TFederatedDbObserverImpl::TFederatedDbObserverImpl(std::shared_ptr<TGRpcConnectionsImpl> connections, const TFederatedTopicClientSettings& settings)
    : TClientImplCommon(std::move(connections), settings)
    , FederatedDbState(std::make_shared<TFederatedDbState>())
    , PromiseToInitState(NThreading::NewPromise())
    , FederationDiscoveryRetryPolicy(settings.RetryPolicy_)
{
    RpcSettings.ClientTimeout = settings.ConnectionTimeout_;
    RpcSettings.EndpointPolicy = TRpcRequestSettings::TEndpointPolicy::UseDiscoveryEndpoint;
    RpcSettings.UseAuth = true;
}

TFederatedDbObserverImpl::~TFederatedDbObserverImpl() {
    Stop();
}

std::shared_ptr<TFederatedDbState> TFederatedDbObserverImpl::GetState() {
    std::lock_guard guard(Lock);
    return FederatedDbState;
}

NThreading::TFuture<void> TFederatedDbObserverImpl::WaitForFirstState() {
    return PromiseToInitState.GetFuture();
}

void TFederatedDbObserverImpl::Start() {
    std::lock_guard guard(Lock);
    if (Stopping) {
        return;
    }
    ScheduleFederationDiscoveryImpl(TDuration::Zero());
}

void TFederatedDbObserverImpl::Stop() {
    NYdbGrpc::IQueueClientContextPtr ctx;
    {
        std::lock_guard guard(Lock);
        Stopping = true;
        ctx = std::exchange(FederationDiscoveryDelayContext, nullptr);
    }
    if (ctx) {
        ctx->Cancel();
    }
}

// If observer is stale it will never update state again because of client retry policy
bool TFederatedDbObserverImpl::IsStale() const {
    std::lock_guard guard(const_cast<TSpinLock&>(Lock));
    return PromiseToInitState.HasValue() && !FederatedDbState->Status.IsSuccess();
}

Ydb::FederationDiscovery::ListFederationDatabasesRequest TFederatedDbObserverImpl::ComposeRequest() const {
    return {};
}

void TFederatedDbObserverImpl::RunFederationDiscoveryImpl() {
    Y_ABORT_UNLESS(Lock.IsLocked());

    FederationDiscoveryDelayContext = Connections_->CreateContext();
    if (!FederationDiscoveryDelayContext) {
        Stopping = true;
        // TODO log DRIVER_IS_STOPPING_DESCRIPTION
        return;
    }

    auto extractor = [selfCtx = SelfContext]
        (google::protobuf::Any* any, TPlainStatus status) mutable {
        if (auto self = selfCtx->LockShared()) {
            Ydb::FederationDiscovery::ListFederationDatabasesResult result;
            if (any) {
                any->UnpackTo(&result);
            }
            self->OnFederationDiscovery(std::move(status), std::move(result));
        }
    };

    Connections_->RunDeferred<Ydb::FederationDiscovery::V1::FederationDiscoveryService,
                             Ydb::FederationDiscovery::ListFederationDatabasesRequest,
                             Ydb::FederationDiscovery::ListFederationDatabasesResponse>(
        ComposeRequest(),
        std::move(extractor),
        &Ydb::FederationDiscovery::V1::FederationDiscoveryService::Stub::AsyncListFederationDatabases,
        DbDriverState_,
        {},  // no polling unready operations, so no need in delay parameter
        RpcSettings,
        FederationDiscoveryDelayContext);
}

void TFederatedDbObserverImpl::ScheduleFederationDiscoveryImpl(TDuration delay) {
    Y_ABORT_UNLESS(Lock.IsLocked());
    auto cb = [selfCtx = SelfContext](bool ok) {
        if (ok) {
            if (auto self = selfCtx->LockShared()) {
                std::lock_guard guard(self->Lock);
                if (self->Stopping) {
                    return;
                }
                self->RunFederationDiscoveryImpl();
            }
        }
    };

    FederationDiscoveryDelayContext = Connections_->CreateContext();
    if (!FederationDiscoveryDelayContext) {
        Stopping = true;
        // TODO log DRIVER_IS_STOPPING_DESCRIPTION
        return;
    }
    Connections_->ScheduleCallback(delay,
                                  std::move(cb),
                                  FederationDiscoveryDelayContext);

}

void TFederatedDbObserverImpl::OnFederationDiscovery(TStatus&& status, Ydb::FederationDiscovery::ListFederationDatabasesResult&& result) {
    {
        std::lock_guard guard(Lock);
        if (Stopping) {
            // TODO log something
            return;
        }

        if (status.GetStatus() == EStatus::CLIENT_CALL_UNIMPLEMENTED) {
            // fall back to single db mode
            FederatedDbState->Status = TPlainStatus{};  // SUCCESS
            auto dbState = Connections_->GetDriverState(std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
            FederatedDbState->ControlPlaneEndpoint = dbState->DiscoveryEndpoint;
            // FederatedDbState->SelfLocation = ???;
            auto db = std::make_shared<Ydb::FederationDiscovery::DatabaseInfo>();
            db->set_path(dbState->Database);
            db->set_endpoint(dbState->DiscoveryEndpoint);
            db->set_status(Ydb::FederationDiscovery::DatabaseInfo_Status_AVAILABLE);
            db->set_weight(100);
            FederatedDbState->DbInfos.emplace_back(std::move(db));

        } else {
            if (!status.IsSuccess()) {
                if (!FederationDiscoveryRetryState) {
                    FederationDiscoveryRetryState = FederationDiscoveryRetryPolicy->CreateRetryState();
                }
                std::optional<TDuration> retryDelay = FederationDiscoveryRetryState->GetNextRetryDelay(status.GetStatus());
                if (retryDelay.has_value()) {
                    ScheduleFederationDiscoveryImpl(*retryDelay);
                    return;
                }
            } else {
                ScheduleFederationDiscoveryImpl(REDISCOVERY_DELAY);
            }

            // TODO validate new state and check if differs from previous

            auto newInfo = std::make_shared<TFederatedDbState>(std::move(result), std::move(status));
            // TODO update only if new state differs
            std::swap(FederatedDbState, newInfo);
        }
    }

    if (!PromiseToInitState.HasValue()) {
        PromiseToInitState.SetValue();
    }
}

} // namespace NYdb::NFederatedTopic
