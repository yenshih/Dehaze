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
        cv::Mat min_filter(const cv::Mat image, const int patch);
        void sort_by_dark_channel(cv::Mat sorted_image, cv::Mat dark_channel, const int low, const int high, const int n);
        cv::Vec<float, 3> get_atmospheric_light(const cv::Mat image, const cv::Mat dark_channel);
        cv::Mat get_transmission(const cv::Mat dark_channel, const cv::Vec<float, 3> atomspheric_light);
        void recover_dehazed_image(cv::Mat image, const cv::Mat fine_transmission, const cv::Vec<float, 3> atmospheric_light);
        cv::Mat get_dehazed_image(cv::Mat source_image);
        std::string get_dehazed_uri(const std::string &uri);
        std::string save_image(cv::Mat dehazed_image, const std::string &uri);
};

}
