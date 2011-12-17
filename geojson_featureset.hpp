#ifndef GEOJSON_FEATURESET_HPP
#define GEOJSON_FEATURESET_HPP

#include <fstream>

// mapnik
#include <mapnik/datasource.hpp>
#include "yajl/yajl_parse.h"
#include "geojson_parser.hpp"

// boost
#include <boost/scoped_ptr.hpp> // needed for wrapping the transcoder

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
