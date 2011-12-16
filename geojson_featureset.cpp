// mapnik
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>

#include <fstream>
#include <iostream>

// yajl
#include "yajl/yajl_parse.h"

#include "geojson_featureset.hpp"

mapnik::transcoder* tr = new mapnik::transcoder("utf-8");

enum parser_state {
    parser_outside,
    parser_in_featurecollection,
    parser_in_features,
    parser_in_feature,
    parser_in_geometry,
    parser_in_coordinates,
    parser_in_properties,
    parser_in_coordinate_pair,
    parser_in_type
};

struct fm {
    mapnik::feature_ptr feature;
    double pair[2];
    std::string property_name;
    parser_state state;
};

static int gj_start_map(void * ctx)
{
    return 1;
}

static int gj_map_key(void * ctx, const unsigned char* key, size_t t)
{
    std::string key_ = std::string((const char*) key, t);
    if (((fm *) ctx)->state == parser_in_properties)
    {
        ((fm *) ctx)->property_name = key_;
    }
    else
    {
        if (key_ == "features")
        {
            ((fm *) ctx)->state = parser_in_features;
        }
        if (key_ == "geometry")
        {
            ((fm *) ctx)->state = parser_in_geometry;
        }
        if (key_ == "type")
        {
            ((fm *) ctx)->state = parser_in_type;
        }
        if (key_ == "properties")
        {
            ((fm *) ctx)->state = parser_in_properties;
        }
        if (key_ == "coordinates")
        {
            ((fm *) ctx)->state = parser_in_coordinates;
        }
    }
    return 1;
}

static int gj_end_map(void * ctx)
{
    if (((fm *) ctx)->state == parser_in_properties ||
        ((fm *) ctx)->state == parser_in_geometry)
    {
        ((fm *) ctx)->state = parser_in_feature;
    }
    if (((fm *) ctx)->state == parser_in_feature)
    {
        return 0;
    }
    return 1;
}

static int gj_null(void * ctx)
{
    if (((fm *) ctx)->state == parser_in_properties)
    {
        boost::put(*((fm *) ctx)->feature, ((fm *) ctx)->property_name, mapnik::value_null());
    }
    return 1;
}

static int gj_boolean(void * ctx, int x)
{
    if (((fm *) ctx)->state == parser_in_properties)
    {
        boost::put(*((fm *) ctx)->feature, ((fm *) ctx)->property_name, x);
    }
    return 1;
}

static int gj_number(void * ctx, const char* str, size_t t)
{
    std::string str_ = std::string((const char*) str, t);
    double x = boost::lexical_cast<double>(str_);

    if (((fm *) ctx)->state == parser_in_coordinates)
    {
        std::cout << x << "\n";
        if (((fm *) ctx)->pair[0] == NULL)
        {
            ((fm *) ctx)->pair[0] = x;
        }
        else if (((fm *) ctx)->pair[1] == NULL)
        {
            ((fm *) ctx)->pair[1] = x;
        }
    }
    if (((fm *) ctx)->state == parser_in_properties)
    {
        boost::put(*((fm *) ctx)->feature, ((fm *) ctx)->property_name, x);
    }
    if (((fm *) ctx)->state == parser_in_coordinates)
    {
    }
    return 1;
}

static int gj_string(void * ctx, const unsigned char* str, size_t t)
{
    std::string str_ = std::string((const char*) str, t);
    if (((fm *) ctx)->state == parser_in_type)
    {
        if (str_ == "Point")
        {
            mapnik::geometry_type * pt = new mapnik::geometry_type(mapnik::Point);
            ((fm *) ctx)->feature->add_geometry(pt);
        }
        if (str_ == "LineString")
        {
            mapnik::geometry_type * pt = new mapnik::geometry_type(mapnik::LineString);
            ((fm *) ctx)->feature->add_geometry(pt);
        }
        if (str_ == "Polygon")
        {
            mapnik::geometry_type * pt = new mapnik::geometry_type(mapnik::Polygon);
            ((fm *) ctx)->feature->add_geometry(pt);
        }
    }
    if (((fm *) ctx)->state == parser_in_properties)
    {
        UnicodeString ustr = tr->transcode(str_.c_str());
        boost::put(*((fm *) ctx)->feature, ((fm *) ctx)->property_name, ustr);
    }
    return 1;
}

static int gj_start_array(void * ctx)
{
    if (((fm *) ctx)->state == parser_in_coordinates)
    {
        ((fm *) ctx)->state = parser_in_coordinate_pair;
    }
    return 1;
}

static int gj_end_array(void * ctx)
{
    if (((fm *) ctx)->state == parser_in_coordinate_pair)
    {
        ((fm *) ctx)->state = parser_in_coordinates;
    }
    if (((fm *) ctx)->state == parser_in_coordinates)
    {
        ((fm *) ctx)->state = parser_outside;
        ((fm *) ctx)->feature->get_geometry(
            ((fm *) ctx)->feature->num_geometries() - 1).move_to(
            ((fm *) ctx)->pair[0],
            ((fm *) ctx)->pair[1]);
    }
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

geojson_featureset::geojson_featureset(
    mapnik::box2d<double> const& box,
    std::string const& encoding,
    std::string const& file)
    : box_(box),
      feature_id_(1),
      file_length_(0),
      file_(file),
      tr_(new mapnik::transcoder(encoding)) { }

geojson_featureset::~geojson_featureset() { }

mapnik::feature_ptr geojson_featureset::next()
{
    if (feature_id_ == 1)
    {
        std::ifstream in_(file_.c_str(), std::ios_base::in | std::ios_base::binary);
        in_.seekg(0, std::ios::end);
        file_length_ = in_.tellg();
        if (!in_.is_open()) {
            throw mapnik::datasource_exception("GeoJSON Plugin: could not open: '" + file_ + "'");
        }
        in_.seekg(0, std::ios::beg);
        // create a new feature
        mapnik::feature_ptr feature(mapnik::feature_factory::create(feature_id_));

        fm state_bundle;
        state_bundle.feature = feature;
        state_bundle.state = parser_outside;
        state_bundle.pair[0] = NULL;
        state_bundle.pair[1] = NULL;

        yajl_handle hand = yajl_alloc(
            &callbacks, NULL,
            &state_bundle);

        yajl_config(hand, yajl_allow_comments, 1);
        
        std::string input_line;

        while (std::getline(in_, input_line)) {
            int parse_result = yajl_parse(hand,
                    (const unsigned char*) input_line.c_str(),
                    input_line.length());

            char* s;
            if (parse_result == yajl_status_error)
            {
                // unsigned char *str = yajl_get_error(hand, 0,  (const unsigned char*) s, strlen(s));
                throw mapnik::datasource_exception("GeoJSON Plugin: invalid GeoJSON detected");
            }
            else if (parse_result == yajl_status_client_canceled)
            {
                return state_bundle.feature;
            }
            else if (parse_result == yajl_status_ok)
            {
                return state_bundle.feature;
            }
        }

        // return the feature!
        // return feature;
        // return mapnik::feature_ptr();
        return state_bundle.feature;
    }

    // otherwise return an empty feature
    return mapnik::feature_ptr();
}

