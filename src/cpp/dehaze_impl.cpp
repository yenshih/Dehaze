#include "dehaze_impl.hpp"

namespace dehaze {

std::shared_ptr<Dehaze> Dehaze::create() {
    return std::make_shared<DehazeImpl>();
}

DehazeImpl::DehazeImpl() {}

cv::Mat DehazeImpl::min_filter(const cv::Mat image, const int patch) {
    cv::Mat minified_image = cv::Mat(rows, cols, CV_32FC1);
    cv::Mat dark_channel = cv::Mat(rows, cols, CV_32FC1);
    const float *data = image.ptr<float>(0);
    float *minified_data = minified_image.ptr<float>(0);
    for (int i = 0; i < this->size; i++) {
        const float blue = *data++;
        const float green = *data++;
        const float red = *data++;
        *minified_data++ = std::fmin(std::fmin(blue, green), red);
    }
    cv::erode(minified_image, dark_channel, cv::Mat::ones(patch, patch, CV_32FC1));
    return dark_channel;
}

void DehazeImpl::sort_by_dark_channel(cv::Mat sorted_image, cv::Mat dark_channel, const int low, const int high, const int n) {
    float *const sorted_image_start = (float *)sorted_image.datastart;
    float *const dark_channel_start = (float *)dark_channel.datastart;
    const int offset = sorted_image_start - dark_channel_start;
    float *i = low + dark_channel_start;
    float *j = high + dark_channel_start;
    const float mid = *(((low + high) >> 1) + dark_channel_start);
    while (i <= j) {
        while (*i > mid) i++;
        while (*j < mid) j--;
        if (i <= j) {
            std::swap(*i, *j);
            std::swap(*(i + offset), *(j + offset));
            i++;
            j--;
        }
    }
    const int next_low = i - dark_channel_start;
    const int next_high = j - dark_channel_start;
    if (n < next_high - low + 1) {
        if (low < next_high) sort_by_dark_channel(sorted_image, dark_channel, low, next_high, n);
    } else {
        const int next_n = n - next_low + low;
        if (next_n > 0 && next_low < high) sort_by_dark_channel(sorted_image, dark_channel, next_low, high, next_n);
    }
}

cv::Vec<float, 3> DehazeImpl::get_atmospheric_light(const cv::Mat image, const cv::Mat dark_channel) {
    const int n = .001 * this->size;
    cv::Mat sorted_image = image.clone();
    this->sort_by_dark_channel(sorted_image, dark_channel, 0, size - 1, n);
    float atmospheric_blue_light = 0;
    float atmospheric_green_light = 0;
    float atmospheric_red_light = 0;
    const float *sorted_data = &(*sorted_image.ptr<cv::Vec<float, 3>>(0))[0];
    for (int i = 0; i < n; i++) {
        atmospheric_blue_light += *sorted_data++;
        atmospheric_green_light += *sorted_data++;
        atmospheric_red_light += *sorted_data++;
    }
    atmospheric_blue_light /= n;
    atmospheric_green_light /= n;
    atmospheric_red_light /= n;
    const cv::Vec<float, 3> atmospheric_light(atmospheric_blue_light, atmospheric_green_light, atmospheric_red_light);
    return atmospheric_light;
}

cv::Mat DehazeImpl::get_transmission(const cv::Mat dark_channel, const cv::Vec<float, 3> atomspheric_light) {
    float approx_atmospheric_light = (atomspheric_light[0] + atomspheric_light[1] + atomspheric_light[2]) / 3;
    cv::Mat transmission = cv::Mat(rows, cols, CV_32FC1);
    const float *dark = dark_channel.ptr<float>(0);
    float *tran = transmission.ptr<float>(0);
    for (int i = 0; i < this->size; i++) {
        float data = 1 - .95 * (*dark++) / approx_atmospheric_light;
        data <= .2 && (data = .5);
        data >= 1 && (data = 1.0);
        *tran++ = data;
    }
    return transmission;
}

void DehazeImpl::recover_dehazed_image(cv::Mat image, const cv::Mat fine_transmission, const cv::Vec<float, 3> atmospheric_light) {
    constexpr float t0 = .1;
    const float atmospheric_blue_light = atmospheric_light[0];
    const float atmospheric_green_light = atmospheric_light[1];
    const float atmospheric_red_light = atmospheric_light[2];
    float *data = &(*image.ptr<cv::Vec<float, 3>>(0))[0];
    const float *tran = fine_transmission.ptr<float>(0);
    for (int i = 0; i < this->size; i++) {
        float restricted_transmission = std::fmax(*tran++, t0);
        *data = ((*data - atmospheric_blue_light) / restricted_transmission + atmospheric_blue_light) * 255;
        data++;
        *data = ((*data - atmospheric_green_light) / restricted_transmission + atmospheric_green_light) * 255;
        data++;
        *data = ((*data - atmospheric_red_light) / restricted_transmission + atmospheric_red_light) * 255;
        data++;
    }
}

cv::Mat DehazeImpl::get_dehazed_image(cv::Mat source_image) {
    cv::Mat image;
    source_image.convertTo(image, CV_32FC3, 1.0 / 255);
    this->rows = image.rows;
    this->cols = image.cols;
    this->size = this->rows * this->cols;
    constexpr int patch = 15;
    const cv::Mat dark_channel = this->min_filter(image, patch);
    const cv::Vec<float, 3> atmospheric_light = get_atmospheric_light(image, dark_channel);
    const cv::Mat transmission = get_transmission(dark_channel, atmospheric_light);
    cv::Mat fine_transmission;
    cv::ximgproc::guidedFilter(image, transmission, fine_transmission, 60, .0001, -1);
    this->recover_dehazed_image(image, fine_transmission, atmospheric_light);
    return image;
}

std::string DehazeImpl::get_dehazed_uri(const std::string &uri) {
    std::size_t delimiter = uri.find_last_of(".");
    return uri.substr(0, delimiter) + "_dehazed" + uri.substr(delimiter);
}

std::string DehazeImpl::save_image(cv::Mat dehazed_image, const std::string &uri) {
    std::vector<int> compression_params = { CV_IMWRITE_PNG_COMPRESSION, 9 };
    std::string dehazed_uri = this->get_dehazed_uri(uri);
    cv::imwrite(dehazed_uri, dehazed_image, compression_params);
    return dehazed_uri;
}

std::string DehazeImpl::dehaze(const std::string &uri, const std::string &media) {
    if (media == "image") {
        cv::Mat source_image = cv::imread(uri, 1);
        cv::Mat dehazed_image = this->get_dehazed_image(source_image);
        return this->save_image(dehazed_image, uri);
    }
    return uri;
}

}
