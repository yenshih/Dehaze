#pragma once

#include "dehaze.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/ximgproc.hpp>

namespace dehaze {

class DehazeImpl: public dehaze::Dehaze {
    public:
        DehazeImpl();
        std::string dehaze(const std::string &uri, const std::string &media);
    private:
        int rows;
        int cols;
        int size;
        cv::Mat min_filter(cv::Mat image, const int &patch);
        cv::Vec<float, 3> get_atmospheric_light(cv::Mat image, cv::Mat dark_channel);
        cv::Mat get_transmission(cv::Mat dark_channel, const double &a);
        cv::Mat get_dehazed_image(cv::Mat image, cv::Mat fine_transmission, cv::Vec<float, 3> a);
        std::string save_image(cv::Mat dehazed_image, const std::string &uri);
};

}
