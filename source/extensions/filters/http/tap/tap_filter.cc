#include "extensions/filters/http/tap/tap_filter.h"

#include "envoy/event/dispatcher.h"
#include "envoy/event/timer.h"
#include "envoy/http/codes.h"
#include "envoy/stats/stats.h"

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

void TapFilter::initConfig() {

}


Http::FilterHeadersStatus TapFilter::decodeHeaders(Http::HeaderMap& headers, bool end_stream) {
  // headers.iterate(Envoy::Http::HeaderMap::ConstIterateCb cb, void *context)

  // TODO (CEV): iterate headers and copy to local def
  //
  // for (std::map<char,int>::iterator it=mymap.begin(); it!=mymap.end(); ++it)
  //   std::cout << it->first << " => " << it->second << '\n';

  if (end_stream) {
    // If this is a header-only request, we don't need to do any buffering.
    return Http::FilterHeadersStatus::Continue;
  }

  // initConfig();
  // if (settings_->disabled()) {
  //   // The filter has been disabled for this route.
  //   return Http::FilterHeadersStatus::Continue;
  // }

  // callbacks_->setDecoderBufferLimit(settings_->maxRequestBytes());
  // request_timeout_ = callbacks_->dispatcher().createTimer([this]() -> void { onRequestTimeout(); });
  // request_timeout_->enableTimer(settings_->maxRequestTime());

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
  std::string final_prefix = prefix + "buffer.";
  return {ALL_BUFFER_FILTER_STATS(POOL_COUNTER_PREFIX(scope, final_prefix))};
}

void TapFilter::onDestroy() { resetInternalState(); }

void TapFilter::onRequestTimeout() {
  callbacks_->sendLocalReply(Http::Code::RequestTimeout, "buffer request timeout", nullptr);
  config_->stats().rq_timeout_.inc();
}

void TapFilter::resetInternalState() {
  headers_.clear();
  request_timeout_.reset();
}

void TapFilter::setDecoderFilterCallbacks(Http::StreamDecoderFilterCallbacks& callbacks) {
  callbacks_ = &callbacks;
}

} // namespace TapFilter
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
