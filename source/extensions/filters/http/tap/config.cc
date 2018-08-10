#include "extensions/filters/http/tap/config.h"

#include <chrono>
#include <cstdint>
#include <string>

#include "envoy/config/filter/http/tap/v2alpha/tap.pb.validate.h"
#include "envoy/registry/registry.h"

#include "common/config/filter_json.h"

#include "extensions/filters/http/tap/tap_filter.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace TapFilter {

Http::FilterFactoryCb TapFilterFactory::createFilterFactoryFromProtoTyped(
    const envoy::config::filter::http::tap::v2alpha::Tap& proto_config,
    const std::string& stats_prefix, Server::Configuration::FactoryContext& context) {
  ASSERT(proto_config.has_max_request_bytes());
  ASSERT(proto_config.has_max_request_time());

  TapFilterConfigSharedPtr filter_config(
      new TapFilterConfig(proto_config, stats_prefix, context.scope()));
  return [filter_config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
    callbacks.addStreamDecoderFilter(std::make_shared<TapFilter>(filter_config));
  };
}

Http::FilterFactoryCb
TapFilterFactory::createFilterFactory(const Json::Object& json_config,
                                         const std::string& stats_prefix,
                                         Server::Configuration::FactoryContext& context) {
  envoy::config::filter::http::tap::v2alpha::Tap proto_config;
  Config::FilterJson::translateTapFilter(json_config, proto_config);
  return createFilterFactoryFromProtoTyped(proto_config, stats_prefix, context);
}

/**
 * Static registration for the tap filter. @see RegisterFactory.
 */
static Registry::RegisterFactory<TapFilterFactory,
                                 Server::Configuration::NamedHttpFilterConfigFactory>
    register_;

} // namespace TapFilter
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
