// mapnik
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>

#include <fstream>
#include <iostream>

// yajl
#include "yajl/yajl_parse.h"

#include "geojson_featureset.hpp"

enum parser_state {
    parser_outside,
    parser_in_featurecollection,
    parser_in_feature,
    parser_in_coordinates,
    parser_in_coordinate_pair,
    parser_in_type
};

struct fm {
    mapnik::feature_ptr feature;
    double pair[2];
    parser_state state;
};

static int gj_start_map(void * ctx)
{
    return 1;
}

static int gj_map_key(void * ctx, const unsigned char* key, size_t t)
{
    std::string key_ = std::string((const char*) key, t);
    std::clog << key_ << "\n";
    if (key_ == "type")
    {
        ((fm *) ctx)->state = parser_in_type;
    }
    if (key_ == "coordinates")
    {
        ((fm *) ctx)->state = parser_in_coordinates;
    }
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
    else
    {
    }
    return 1;
}

static int gj_boolean(void * ctx, int)
{
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
      tr_(new mapnik::transcoder(encoding)) {


}

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
            if (parse_result != yajl_status_ok)
            {
                // unsigned char *str = yajl_get_error(hand, 0,  (const unsigned char*) s, strlen(s));
                throw mapnik::datasource_exception("GeoJSON Plugin: invalid GeoJSON detected");
            }
        }

        // increment the count so that we only return one feature
        ++feature_id_;

        /*
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
        */



        // return the feature!
        // return feature;
        return state_bundle.feature;
    }

    // otherwise return an empty feature
    return mapnik::feature_ptr();
}

