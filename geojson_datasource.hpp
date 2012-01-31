#ifndef GEOJSON_DATASOURCE_HPP
#define GEOJSON_DATASOURCE_HPP

#include <fstream>

// mapnik
#include <mapnik/datasource.hpp>

class geojson_datasource : public mapnik::datasource
{
public:
    // constructor
    geojson_datasource(mapnik::parameters const& params, bool bind=true);
    virtual ~geojson_datasource ();
    mapnik::datasource::datasource_t type() const;
    static std::string name();
    mapnik::featureset_ptr features(mapnik::query const& q) const;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt) const;
    mapnik::box2d<double> envelope() const;
    mapnik::layer_descriptor get_descriptor() const;
    std::map<std::string, mapnik::parameters> get_statistics() const;
    boost::optional<mapnik::datasource::geometry_t> get_geometry_type() const;
    void bind() const;

private:
    // recommended naming convention of datasource members:
    // name_, type_, extent_, and desc_
    static const std::string name_;
    mapnik::datasource::datasource_t type_;
    mutable std::map<std::string, mapnik::parameters> statistics_;
    mutable mapnik::layer_descriptor desc_;
    mutable std::string file_;
    mutable mapnik::box2d<double> extent_;
};


#endif // FILE_DATASOURCE_HPP
