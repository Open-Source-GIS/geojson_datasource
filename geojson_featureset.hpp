#ifndef HELLO_FEATURESET_HPP
#define HELLO_FEATURESET_HPP

#include <fstream>

// mapnik
#include <mapnik/datasource.hpp>

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
    boost::scoped_ptr<mapnik::transcoder> tr_;
};

#endif // HELLO_FEATURESET_HPP
