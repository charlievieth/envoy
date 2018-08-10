#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include "envoy/config/filter/http/tap/v2alpha/tap.pb.h"
#include "envoy/data/httptap/v2alpha/capture.pb.h"
#include "envoy/http/filter.h"
#include "envoy/stats/scope.h"
#include "envoy/stats/stats_macros.h"

#include "common/buffer/buffer_impl.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace TapFilter {

// TODO (CEV): add more stats - see gzip filter
/**
 * All stats for the tap filter. @see stats_macros.h
 */
// clang-format off
#define ALL_TAP_FILTER_STATS(COUNTER)                                                           \
  COUNTER(rq_timeout)
// clang-format on

// TODO (CEV): add more stats - see gzip filter
/**
 * Wrapper struct for tap filter stats. @see stats_macros.h
 */
struct TapFilterStats {
  ALL_TAP_FILTER_STATS(GENERATE_COUNTER_STRUCT)
};

// CEV: removed public Router::RouteSpecificFilterConfig
class TapFilterSettings {
public:
  TapFilterSettings(const envoy::config::filter::http::tap::v2alpha::Tap&);

  bool disabled() const { return disabled_; }
  uint64_t maxRequestBytes() const { return max_request_bytes_; }
  std::chrono::seconds maxRequestTime() const { return max_request_time_; }

private:
  bool disabled_;
  uint64_t max_request_bytes_;
  std::chrono::seconds max_request_time_;
};

/**
 * Configuration for the tap filter.
 */
class TapFilterConfig {
public:
  TapFilterConfig(const envoy::config::filter::http::tap::v2alpha::Tap& proto_config,
                     const std::string& stats_prefix, Stats::Scope& scope);

  TapFilterStats& stats() { return stats_; }
  const TapFilterSettings* settings() const { return &settings_; }

private:
  TapFilterStats stats_;
  const TapFilterSettings settings_;
};

typedef std::shared_ptr<TapFilterConfig> TapFilterConfigSharedPtr;

// TODO (CEV): fix comment
/**
 * A filter that is capable of buffering an entire request before dispatching it upstream.
 */
class TapFilter : public Http::StreamDecoderFilter {
public:
  TapFilter(TapFilterConfigSharedPtr config);
  ~TapFilter();

  static TapFilterStats generateStats(const std::string& prefix, Stats::Scope& scope);

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamDecoderFilter
  Http::FilterHeadersStatus decodeHeaders(Http::HeaderMap& headers, bool end_stream) override;
  Http::FilterDataStatus decodeData(Buffer::Instance& data, bool end_stream) override;
  Http::FilterTrailersStatus decodeTrailers(Http::HeaderMap& trailers) override;
  void setDecoderFilterCallbacks(Http::StreamDecoderFilterCallbacks& callbacks) override;

private:
  void onRequestTimeout();
  void resetInternalState();
  void initConfig();

  TapFilterConfigSharedPtr config_;
  const TapFilterSettings* settings_;
  Http::StreamDecoderFilterCallbacks* callbacks_{};
  Event::TimerPtr request_timeout_;
  bool config_initialized_{};
};

} // namespace TapFilter
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
