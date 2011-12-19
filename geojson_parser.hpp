#ifndef GEOJSON_PARSER_HPP
#define GEOJSON_PARSER_HPP

#include <mapnik/feature_factory.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/geometry.hpp>

#include "yajl/yajl_parse.h"

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
    int done;
    std::string property_name;
    parser_state state;
};

#endif // GEOJSON_PARSER_HPP
