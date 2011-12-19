#ifndef GEOJSON_FEATURESET_HPP
#define GEOJSON_FEATURESET_HPP

#include <fstream>

// mapnik
#include <mapnik/datasource.hpp>
#include "yajl/yajl_parse.h"

// boost
#include <boost/scoped_ptr.hpp> // needed for wrapping the transcoder

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

// extend the mapnik::Featureset defined in include/mapnik/datasource.hpp
class geojson_featureset : public mapnik::Featureset
{
public:
    geojson_featureset(mapnik::box2d<double> const& box,
            std::string const& encoding,
            std::string const& file);
    virtual ~geojson_featureset();
    mapnik::feature_ptr next();
private:
    mapnik::box2d<double> const& box_;
    mutable int feature_id_;
    mutable int file_length_;

    mutable std::ifstream in_;
    mutable std::string file_;

    // parsing related
    mutable std::string input_buffer_;
    int itt_;
    yajl_handle hand;
    fm state_bundle;

    boost::scoped_ptr<mapnik::transcoder> tr_;
};

#endif // HELLO_FEATURESET_HPP
