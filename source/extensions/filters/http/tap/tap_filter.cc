#include "extensions/filters/http/tap/tap_filter.h"

#include "envoy/event/dispatcher.h"
#include "envoy/event/timer.h"
#include "envoy/http/codes.h"
#include "envoy/stats/scope.h"

#include "common/common/assert.h"
#include "common/common/enum_to_int.h"
#include "common/http/codes.h"
#include "common/http/header_map_impl.h"
#include "common/http/headers.h"
#include "common/http/utility.h"

#include "extensions/filters/http/well_known_names.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace TapFilter {

TapFilterSettings::TapFilterSettings(
    const envoy::config::filter::http::tap::v2alpha::Tap& proto_config)
    : disabled_(false),
      max_request_bytes_(static_cast<uint64_t>(proto_config.max_request_bytes().value())),
      max_request_time_(
          std::chrono::seconds(PROTOBUF_GET_SECONDS_REQUIRED(proto_config, max_request_time))) {}

TapFilterConfig::TapFilterConfig(
    const envoy::config::filter::http::tap::v2alpha::Tap& proto_config,
    const std::string& stats_prefix, Stats::Scope& scope)
    : stats_(TapFilter::generateStats(stats_prefix, scope)), settings_(proto_config) {}

TapFilter::TapFilter(TapFilterConfigSharedPtr config)
    : config_(config), settings_(config->settings()) {}

TapFilter::~TapFilter() { ASSERT(!request_timeout_); }

void TapFilter::initConfig() {
  ASSERT(!config_initialized_);
  config_initialized_ = true;

  settings_ = config_->settings();

  if (!callbacks_->route() || !callbacks_->route()->routeEntry()) {
    return;
  }

  const std::string& name = HttpFilterNames::get().Tap;
  const auto* entry = callbacks_->route()->routeEntry();

  const TapFilterSettings* route_local =
      entry->perFilterConfigTyped<TapFilterSettings>(name)
          ?: entry->virtualHost().perFilterConfigTyped<TapFilterSettings>(name);

  settings_ = route_local ?: settings_;
}

Http::FilterHeadersStatus TapFilter::decodeHeaders(Http::HeaderMap&, bool end_stream) {
  if (end_stream) {
    // If this is a header-only request, we don't need to do any buffering.
    return Http::FilterHeadersStatus::Continue;
  }

  initConfig();
  if (settings_->disabled()) {
    // The filter has been disabled for this route.
    return Http::FilterHeadersStatus::Continue;
  }

  callbacks_->setDecoderBufferLimit(settings_->maxRequestBytes());
  request_timeout_ = callbacks_->dispatcher().createTimer([this]() -> void { onRequestTimeout(); });
  request_timeout_->enableTimer(settings_->maxRequestTime());

  return Http::FilterHeadersStatus::StopIteration;
}

Http::FilterDataStatus TapFilter::decodeData(Buffer::Instance&, bool end_stream) {
  if (end_stream) {
    resetInternalState();
    return Http::FilterDataStatus::Continue;
  }

  // Buffer until the complete request has been processed or the ConnectionManagerImpl sends a 413.
  return Http::FilterDataStatus::StopIterationAndBuffer;
}

Http::FilterTrailersStatus TapFilter::decodeTrailers(Http::HeaderMap&) {
  resetInternalState();
  return Http::FilterTrailersStatus::Continue;
}

TapFilterStats TapFilter::generateStats(const std::string& prefix, Stats::Scope& scope) {
  std::string final_prefix = prefix + "tap.";
  return {ALL_TAP_FILTER_STATS(POOL_COUNTER_PREFIX(scope, final_prefix))};
}

void TapFilter::onDestroy() { resetInternalState(); }

void TapFilter::onRequestTimeout() {
  // TODO (CEV): "buffer request timeout" => "tap request timeout"
  // maybe leave as: "buffer request timeout"
  //
  callbacks_->sendLocalReply(Http::Code::RequestTimeout, "tap request timeout", nullptr);
  config_->stats().rq_timeout_.inc();
}

void TapFilter::resetInternalState() { request_timeout_.reset(); }

void TapFilter::setDecoderFilterCallbacks(Http::StreamDecoderFilterCallbacks& callbacks) {
  callbacks_ = &callbacks;
}

} // namespace TapFilter
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
