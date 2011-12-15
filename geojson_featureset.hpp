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
    // this constructor can have any arguments you need
    geojson_featureset(mapnik::box2d<double> const& box,
            std::string const& encoding,
            boost::shared_ptr<std::ifstream> const& in_);

    // desctructor
    virtual ~geojson_featureset();

    // mandatory: you must expose a next() method, called when rendering
    mapnik::feature_ptr next();

private:
    // members are up to you, but these are recommended
    mapnik::box2d<double> const& box_;
    mutable int feature_id_;
    boost::scoped_ptr<mapnik::transcoder> tr_;
};

#endif // HELLO_FEATURESET_HPP
