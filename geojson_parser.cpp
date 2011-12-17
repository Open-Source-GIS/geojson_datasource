#include "yajl/yajl_parse.h"
#include <iostream>

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
        std::clog << "marking as in feature\n";
        ((fm *) ctx)->state = parser_in_feature;
    }
    else if (((fm *) ctx)->state == parser_in_feature)
    {
        std::clog << "marking as done\n";
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
    else if (((fm *) ctx)->state == parser_in_coordinates)
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
