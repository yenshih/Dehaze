#include "dehaze_impl.hpp"

namespace dehaze {

std::shared_ptr<Dehaze> Dehaze::create() {
    return std::make_shared<DehazeImpl>();
}

DehazeImpl::DehazeImpl() {}

cv::Mat DehazeImpl::min_filter(cv::Mat image, const int patch) {
    cv::Mat minified_image = cv::Mat(this->rows, this->cols, CV_32FC1);
    cv::Mat dark_channel = cv::Mat(this->rows, this->cols, CV_32FC1);
    float *data = image.ptr<float>(0);
    float *minified_data = minified_image.ptr<float>(0);
    for (int i = 0; i < this->size; i++) {
        const float blue = *data++;
        const float green = *data++;
        const float red = *data++;
        *minified_data++ = std::fmin(std::fmin(blue, green), red);
    }
    erode(minified_image, dark_channel, cv::Mat::ones(patch, patch, CV_32FC1));
    return dark_channel;
}

cv::Vec<float, 3> DehazeImpl::get_atmospheric_light(cv::Mat image, cv::Mat dark_channel) {
    cv::Vec<float, 3> a(0, 0, 0);
    const int n = .001 * this->size;
    cv::Mat linear_dark_channel = dark_channel.reshape(1, this->size);
    cv::Mat res = cv::Mat::ones(n, 1, CV_32FC3);
    for (int i = 0; i < n; i++) {
        float max = 0;
        int pos = 0;
        for (float *p = (float *)linear_dark_channel.datastart; p != (float *)linear_dark_channel.dataend; p++) {
            if (*p > max) {
                max = *p;
                pos = (p - (float *)linear_dark_channel.datastart);
                res.at<cv::Vec<float, 3>>(i, 0) = ((cv::Vec<float, 3> *)image.data)[pos];
            }
        }
        ((float *)linear_dark_channel.data)[pos] = 0;
    }
    for (int i = 0; i < n; i++) {
        a[0] += res.at<cv::Vec<float, 3>>(i, 0)[0];
        a[1] += res.at<cv::Vec<float, 3>>(i, 0)[1];
        a[2] += res.at<cv::Vec<float, 3>>(i, 0)[2];
    }
    a[0] /= n;
    a[1] /= n;
    a[2] /= n;
    return a;
}

cv::Mat DehazeImpl::get_transmission(cv::Mat dark_channel, double a) {
    cv::Mat transmission = cv::Mat(this->rows, this->cols, CV_32FC1);
    float *dark = dark_channel.ptr<float>(0);
    float *tran = transmission.ptr<float>(0);
    for (int i = 0; i < this->size; i++) {
        float data = 1 - .95 * (*dark++) / a;
        data <= .2 && (data = .5);
        data >= 1 && (data = 1.0);
        *tran++ = data;
    }
    return transmission;
}

cv::Mat DehazeImpl::get_dehazed_image(cv::Mat image, cv::Mat fine_transmission, cv::Vec<float, 3> a) {
    constexpr float t0 = .1;
    cv::Mat dehazed_image = cv::Mat(this->rows, this->cols, CV_32FC3);
    cv::Vec<float, 3> *data = image.ptr<cv::Vec<float, 3>>(0);
    float *tran = fine_transmission.ptr<float>(0);
    cv::Vec<float, 3> *dehazed_data = dehazed_image.ptr<cv::Vec<float, 3>>(0);
    for (int i = 0; i < this->size; i++, data++, tran++, dehazed_data++) {
        (*dehazed_data)[0] = (((*data)[0] - a[0]) / std::fmax(*tran, t0) + a[0]) * 255;
        (*dehazed_data)[1] = (((*data)[1] - a[1]) / std::fmax(*tran, t0) + a[1]) * 255;
        (*dehazed_data)[2] = (((*data)[2] - a[2]) / std::fmax(*tran, t0) + a[2]) * 255;
    }
    return dehazed_image;
}

std::string DehazeImpl::save_image(cv::Mat dehazed_image, const std::string uri) {
    std::vector<int> compression_params = { CV_IMWRITE_PNG_COMPRESSION, 9 };
    std::size_t delimiter = uri.find_last_of(".");
    std::string dehazed_uri = uri.substr(0, delimiter) + "_dehazed" + uri.substr(delimiter);
    cv::imwrite(dehazed_uri, dehazed_image, compression_params);
    return dehazed_uri;
}

std::string DehazeImpl::dehaze_image(const std::string & uri) {
    cv::Mat image;
    cv::imread(uri, 1).convertTo(image, CV_32FC3, 1.0 / 255);
    this->rows = image.rows;
    this->cols = image.cols;
    this->size = this->rows * this->cols;
    constexpr int patch = 15;
    cv::Mat dark_channel = this->min_filter(image, patch);
    cv::Vec<float, 3> a = this->get_atmospheric_light(image, dark_channel);
    cv::Mat transmission = this->get_transmission(dark_channel, (a[0] + a[1] + a[2]) / 3.0);
    cv::Mat fine_transmission;
    cv::ximgproc::guidedFilter(image, transmission, fine_transmission, 60, .0001, -1);
    cv::Mat dehazed_image = this->get_dehazed_image(image, fine_transmission, a);
    return this->save_image(dehazed_image, uri);
}

}
