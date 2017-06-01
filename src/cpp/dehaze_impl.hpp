#pragma once

#include "dehaze.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

namespace dehaze {

class DehazeImpl: public dehaze::Dehaze {
    public:
        DehazeImpl();
        std::string dehaze(const std::string &uri, const std::string &media) override;
    private:
        uint32_t rows;
        uint32_t cols;
        uint32_t size;
        cv::Mat get_dark_channel(const cv::Mat &image, const uint32_t patch);
        cv::Vec3f get_atmospheric_light(float *const dark_channel_start, const int32_t offset, const uint32_t low, const uint32_t high, const uint32_t n, std::vector<std::pair<float *, float *>> &swap_stack);
        cv::Mat get_transmission(const cv::Mat &image, const cv::Mat &dark_channel, const cv::Vec3f &atomspheric_light);
        cv::Mat subsample(const cv::Mat &image, const uint32_t sampling_ratio);
        cv::Mat upsample(const cv::Mat &image, const uint32_t sampling_ratio);
        cv::Mat mean_filter(const cv::Mat &image, const uint32_t radius);
        cv::Mat guided_filter(const cv::Mat &guidance_image, const cv::Mat &source_image, const uint32_t radius, const double epsilon, const uint32_t sampling_ratio);
        cv::Mat get_fine_transmission(const cv::Mat &image, const cv::Mat &transmission);
        void recover_dehazed_image(cv::Mat &image, const cv::Mat &fine_transmission, const cv::Vec3f &atmospheric_light);
        cv::Mat get_dehazed_image(const cv::Mat &source_image);
        std::string trim_uri_protocal(const std::string &uri);
        std::string get_dehazed_uri(const std::string &uri);
        std::string save_dehazed_image(const cv::Mat &dehazed_image, const std::string &uri);
};

}
