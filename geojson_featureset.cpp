// mapnik
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>

#include <fstream>
#include <iostream>

#include "yajl/yajl_parse.h"

#include "geojson_featureset.hpp"

mapnik::transcoder* tr = new mapnik::transcoder("utf-8");

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
        else if (key_ == "geometry")
        {
            ((fm *) ctx)->state = parser_in_geometry;
        }
        else if (key_ == "type")
        {
            ((fm *) ctx)->state = parser_in_type;
        }
        else if (key_ == "properties")
        {
            ((fm *) ctx)->state = parser_in_properties;
        }
        else if (key_ == "coordinates")
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
    else if (((fm *) ctx)->state == parser_in_feature)
    {
        ((fm *) ctx)->done = 1;
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

    if (((fm *) ctx)->state == parser_in_coordinate_pair)
    {
        if (((fm *) ctx)->pair[0] == 1000)
        {
            ((fm *) ctx)->pair[0] = x;
        }
        else if (((fm *) ctx)->pair[1] == 1000)
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
        bool valid_type = true;
        mapnik::geometry_type * pt;
        if (str_ == "Point")
        {
            pt = new mapnik::geometry_type(mapnik::Point);
        }
        else if (str_ == "LineString")
        {
            pt = new mapnik::geometry_type(mapnik::LineString);
        }
        else if (str_ == "Polygon")
        {
            pt = new mapnik::geometry_type(mapnik::Polygon);
        }
        else if (str_ == "MultiPoint")
        {
            pt = new mapnik::geometry_type(mapnik::MultiPoint);
        }
        else if (str_ == "MultiLineString")
        {
            pt = new mapnik::geometry_type(mapnik::MultiLineString);
        }
        else if (str_ == "MultiPolygon")
        {
            pt = new mapnik::geometry_type(mapnik::MultiPolygon);
        }
        else { valid_type = false; }

        if (valid_type)
        {
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
    int is_point = 0;
    if (((fm *) ctx)->feature->num_geometries() > 0)
    {
        mapnik::geometry_type & geom = ((fm *) ctx)->feature->get_geometry(0);
        if (geom.type() == mapnik::Point)
        {
            is_point = 1;
        }
    }
    if (((fm *) ctx)->state == parser_in_coordinates || is_point)
    {
        ((fm *) ctx)->state = parser_in_coordinate_pair;
    }
    return 1;
}

static int gj_end_array(void * ctx)
{
    int is_point = 0;
    if (((fm *) ctx)->feature->num_geometries() > 0)
    {
        mapnik::geometry_type & geom = ((fm *) ctx)->feature->get_geometry(0);
        if (geom.type() == mapnik::Point)
        {
            is_point = 1;
        }
    }

    if (((fm *) ctx)->state == parser_in_coordinate_pair)
    {
        ((fm *) ctx)->state = parser_in_coordinates;
        if (is_point)
        {
            ((fm *) ctx)->state = parser_outside;
            ((fm *) ctx)->feature->get_geometry(
                ((fm *) ctx)->feature->num_geometries() - 1).move_to(
                ((fm *) ctx)->pair[0],
                ((fm *) ctx)->pair[1]);
        }
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
      itt_(0),
      tr_(new mapnik::transcoder(encoding)) {

    std::ifstream in_(file_.c_str(), std::ios_base::in | std::ios_base::binary);

    if (!in_.is_open()) {
        throw mapnik::datasource_exception("GeoJSON Plugin: could not open: '" + file_ + "'");
    }

    in_.seekg(0, std::ios::end);
    file_length_ = in_.tellg();
    in_.seekg(0, std::ios::beg);

    state_bundle.state = parser_outside;
    state_bundle.done = 0;
    state_bundle.pair[0] = 1000;
    state_bundle.pair[1] = 1000;

    // FIXME: manually free
    hand = yajl_alloc(
        &callbacks, NULL,
        &state_bundle);

    yajl_config(hand, yajl_allow_comments, 1);

    std::getline(in_, input_buffer_);


    mapnik::feature_ptr feature(mapnik::feature_factory::create(feature_id_));

    state_bundle.done = 0;
    state_bundle.feature = feature;

    for (; itt_ < input_buffer_.length(); itt_++) {

        int parse_result;

        parse_result = yajl_parse(hand,
                (const unsigned char*) &input_buffer_[itt_],
                1);

        if (parse_result == yajl_status_error)
        {
            //char* s;
            //unsigned char *str = yajl_get_error(hand, 1,  (const unsigned char*) s, strlen(s));
            // throw mapnik::datasource_exception("GeoJSON Plugin: invalid GeoJSON detected:" +
            //             std::string((const char*) str));
            throw mapnik::datasource_exception("GeoJSON Plugin: invalid GeoJSON detected");
            // yajl_free_error(hand, str);
        }
        else if (state_bundle.done == 1)
        {
            features_.push_back(state_bundle.feature);
            mapnik::feature_ptr feature(mapnik::feature_factory::create(feature_id_));

            // reset
            state_bundle.pair[0] = 1000;
            state_bundle.pair[1] = 1000;
            state_bundle.done = 0;
            state_bundle.feature = feature;

        }

    }

}

geojson_featureset::~geojson_featureset() { }

mapnik::feature_ptr geojson_featureset::next()
{

    feature_id_++;

    if (feature_id_ <= features_.size())
    {
        return features_.at(feature_id_ - 1);
    }
    else
    {
        return mapnik::feature_ptr();
    }
}
