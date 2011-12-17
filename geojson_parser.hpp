#ifndef GEOJSON_PARSER_HPP
#define GEOJSON_PARSER_HPP

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

int gj_null(void * ctx);
int gj_boolean(void * ctx, int);
int gj_number(void * ctx, const char*, size_t);
int gj_string(void *, const unsigned char*, size_t);
int gj_map_key(void *, const unsigned char*, size_t);
int gj_start_map(void *);
int gj_end_map(void *);
int gj_start_array(void * ctx);
int gj_end_array(void * ctx);

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

#endif // GEOJSON_PARSER_HPP
