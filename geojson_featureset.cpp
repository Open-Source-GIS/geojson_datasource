// mapnik
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>

#include <fstream>
#include <iostream>

#include "geojson_featureset.hpp"
#include "geojson_parser.hpp"

mapnik::transcoder* tr = new mapnik::transcoder("utf-8");

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
    state_bundle.pair[0] = NULL;
    state_bundle.pair[1] = NULL;

    // FIXME: manually free
    hand = yajl_alloc(
        &callbacks, NULL,
        &state_bundle);

    yajl_config(hand, yajl_allow_comments, 1);

    std::getline(in_, input_buffer_);

}

geojson_featureset::~geojson_featureset() { }

mapnik::feature_ptr geojson_featureset::next()
{

    std::clog << "geojson_featureset::next() on " << itt_ << "\n";

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
            std::clog << "geojson_featureset::next exiting at " << itt_ << "\n";
            return state_bundle.feature;
        }
        /*
        else if (parse_result == yajl_status_ok)
        {
            std::clog << "geojson_featureset::next status ok at " << itt_ << "\n";
            return mapnik::feature_ptr();
        }
        */
    }

    feature_id_++;

    // return state_bundle.feature;
    return mapnik::feature_ptr();

}
