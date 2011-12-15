// mapnik
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>

// yajl
#include "yajl/yajl_parse.h"

#include "geojson_featureset.hpp"


static int gj_start_map(void * ctx)
{
    return 1;
}

static int gj_map_key(void * ctx, const unsigned char*, size_t t)
{
    return 1;
}

static int gj_end_map(void * ctx)
{
    return 1;
}

static int gj_null(void * ctx)
{
    return 1;
}

static int gj_number(void * ctx, const char* str, size_t t)
{
    std::cout << str << "\n";
    return 1;
}

static int gj_boolean(void * ctx, int)
{
    return 1;
}

static int gj_string(void * ctx, const unsigned char* str, size_t t)
{
    std::cout << str << "\n";
    return 1;
}

static int gj_start_array(void * ctx)
{
    return 1;
}

static int gj_end_array(void * ctx)
{
    return 1;
}

static yajl_callbacks callbacks = {
    gj_null,
    gj_boolean,
    NULL,
    NULL,
    gj_number,
    gj_string,
    gj_start_map,
    gj_map_key,
    gj_end_map,
    gj_start_array,
    gj_end_array
};

geojson_featureset::geojson_featureset(mapnik::box2d<double> const& box, std::string const& encoding)
    : box_(box),
      feature_id_(1),
      tr_(new mapnik::transcoder(encoding)) { }

geojson_featureset::~geojson_featureset() { }

mapnik::feature_ptr geojson_featureset::next()
{
    if (feature_id_ == 1)
    {
        // create a new feature
        mapnik::feature_ptr feature(mapnik::feature_factory::create(feature_id_));

        // increment the count so that we only return one feature
        ++feature_id_;

        // create an attribute pair of key:value
        UnicodeString ustr = tr_->transcode("geojson world!");
        boost::put(*feature,"key",ustr);

        // we need a geometry to display so just for fun here
        // we take the center of the bbox that was used to query
        // since we don't actually have any data to pull from...
        mapnik::coord2d center = box_.center();

        // create a new point geometry
        mapnik::geometry_type * pt = new mapnik::geometry_type(mapnik::Point);

        // we use path type geometries in Mapnik to fit nicely with AGG and Cairo
        // here we stick an x,y pair into the geometry using move_to()
        pt->move_to(center.x,center.y);

        // add the geometry to the feature
        feature->add_geometry(pt);

        // A feature usually will have just one geometry of a given type
        // but mapnik does support many geometries per feature of any type
        // so here we draw a line around the point
        mapnik::geometry_type * line = new mapnik::geometry_type(mapnik::LineString);
        line->move_to(box_.minx(),box_.miny());
        line->line_to(box_.minx(),box_.maxy());
        line->line_to(box_.maxx(),box_.maxy());
        line->line_to(box_.maxx(),box_.miny());
        line->line_to(box_.minx(),box_.miny());
        feature->add_geometry(line);

        // return the feature!
        return feature;
    }

    // otherwise return an empty feature
    return mapnik::feature_ptr();
}
