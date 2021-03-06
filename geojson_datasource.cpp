// file plugin
#include "geojson_datasource.hpp"
#include "geojson_featureset.hpp"

#include <fstream>
#include <iostream>

// boost
#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>

// mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/projection.hpp>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(geojson_datasource)

geojson_datasource::geojson_datasource(parameters const& params, bool bind)
: datasource(params),
    type_(datasource::Vector),
    desc_(*params_.get<std::string>("type"),
        *params_.get<std::string>("encoding","utf-8")),
    file_(*params_.get<std::string>("file","")),
    extent_()
{
    if (file_.empty()) throw mapnik::datasource_exception("GeoJSON Plugin: missing <file> parameter");
    if (bind)
    {
        this->bind();
    }
}

// FIXME: implement
void geojson_datasource::bind() const
{
    if (is_bound_) return;

    extent_.init(-180,-90,180,90);

    is_bound_ = true;
}

geojson_datasource::~geojson_datasource() { }

// This name must match the plugin filename, eg 'geojson.input'
std::string const geojson_datasource::name_="geojson";

std::string geojson_datasource::name()
{
    return name_;
}


boost::optional<mapnik::datasource::geometry_t> geojson_datasource::get_geometry_type() const {
    return boost::optional<mapnik::datasource::geometry_t>();
}

mapnik::datasource::datasource_t geojson_datasource::type() const {
    return type_;
}

std::map<std::string, mapnik::parameters> geojson_datasource::get_statistics() const {
    return statistics_;
}

// FIXME: implement
mapnik::box2d<double> geojson_datasource::envelope() const
{
    if (!is_bound_) bind();

    return extent_;
}

mapnik::layer_descriptor geojson_datasource::get_descriptor() const
{
    if (!is_bound_) bind();

    return desc_;
}

mapnik::featureset_ptr geojson_datasource::features(mapnik::query const& q) const
{
    if (!is_bound_) bind();

    // if the query box intersects our world extent then query for features
    if (extent_.intersects(q.get_bbox()))
    {
        return boost::make_shared<geojson_featureset>(q.get_bbox(),
                desc_.get_encoding(),
                file_);
    }

    // otherwise return an empty featureset pointer
    return mapnik::featureset_ptr();
}

// FIXME: implement
mapnik::featureset_ptr geojson_datasource::features_at_point(mapnik::coord2d const& pt) const
{
    if (!is_bound_) bind();
    return mapnik::featureset_ptr();
}
