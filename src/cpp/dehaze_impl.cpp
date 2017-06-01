#include "dehaze_impl.hpp"

namespace dehaze {

std::shared_ptr<Dehaze> Dehaze::create() {
    return std::make_shared<DehazeImpl>();
}

DehazeImpl::DehazeImpl() {}

cv::Mat DehazeImpl::get_dark_channel(const cv::Mat &image, const uint32_t patch) {
    cv::Mat minified_image = cv::Mat(this->rows, this->cols, CV_32FC1);
    const float *data = image.ptr<float>();
    float *minified_data = minified_image.ptr<float>();
    for (uint32_t i = 0; i < this->size; i++) {
        const float blue = *data++;
        const float green = *data++;
        const float red = *data++;
        *minified_data++ = std::fmin(std::fmin(blue, green), red);
    }
    cv::Mat dark_channel;
    erode(minified_image, dark_channel, cv::Mat::ones(patch, patch, CV_32FC1));
    return dark_channel;
}

cv::Vec3f DehazeImpl::get_atmospheric_light(float *const dark_channel_start, const int32_t offset, const uint32_t low, const uint32_t high, const uint32_t n, std::vector<std::pair<float *, float *>> &swap_stack) {
    float *i = low + dark_channel_start;
    float *j = high + dark_channel_start;
    const float pivot = *(((low + high) >> 1) + dark_channel_start);
    while (i <= j) {
        while (*i > pivot) i++;
        while (*j < pivot) j--;
        if (i <= j) {
            std::swap(*i, *j);
            std::swap(*(i + offset), *(j + offset));
            swap_stack.push_back(std::make_pair(i++, j--));
        }
    }
    const uint32_t next_low = i - dark_channel_start;
    const uint32_t next_high = j - dark_channel_start;
    const int32_t next_n = n - next_low + low;
    if (n < next_high - low + 1) {
        return get_atmospheric_light(dark_channel_start, offset, low, next_high, n, swap_stack);
    } else if (next_n > 0) {
        return get_atmospheric_light(dark_channel_start, offset, next_low, high, next_n, swap_stack);
    } else {
        float atmospheric_blue_light = 0;
        float atmospheric_green_light = 0;
        float atmospheric_red_light = 0;
        float max_gray_data = 0;
        const float *data = dark_channel_start + offset;
        for (uint32_t i = 0; i < n; i++) {
            const float blue = *data++;
            const float green = *data++;
            const float red = *data++;
            const float gray_data = (299 * red + 587 * green + 114 * blue) / 1000;
            if (gray_data > max_gray_data) {
                max_gray_data = gray_data;
                atmospheric_blue_light = blue;
                atmospheric_green_light = green;
                atmospheric_red_light = red;
            }
        }
        const cv::Vec3f atmospheric_light(atmospheric_blue_light, atmospheric_green_light, atmospheric_red_light);
        for (size_t k = swap_stack.size() - 1; k > 0; k--) {
            std::pair<float *, float *> swap_pair = swap_stack[k];
            float *i = swap_pair.first;
            float *j = swap_pair.second;
            std::swap(*i, *j);
            std::swap(*(i + offset), *(j + offset));
        }
        return atmospheric_light;
    }
}

cv::Mat DehazeImpl::get_transmission(const cv::Mat &image, const cv::Mat &dark_channel, const cv::Vec3f &atmospheric_light) {
    const auto sqr = [](float x)->float { return x * x; };
    float approx_atmospheric_light = (atmospheric_light[0] + atmospheric_light[1] + atmospheric_light[2]) / 3;
    const float variance = (sqr(atmospheric_light[0] - approx_atmospheric_light) + sqr(atmospheric_light[1] - approx_atmospheric_light) + sqr(atmospheric_light[2] - approx_atmospheric_light)) / 3;
    cv::Mat transmission(this->rows, this->cols, CV_32FC1);
    const float *dark = dark_channel.ptr<float>();
    constexpr float variance_threshold = .001;
    constexpr float diff_threshold = .4;
    if (variance > variance_threshold) {
        cv::Mat temp_image(this->rows, this->cols, CV_32FC3);
        const float *data = image.ptr<float>();
        float *temp_data = temp_image.ptr<float>();
        for (uint32_t i = 0; i < this->size; i++) {
            *temp_data++ = *data++ / atmospheric_light[0];
            *temp_data++ = *data++ / atmospheric_light[1];
            *temp_data++ = *data++ / atmospheric_light[2];
        }
        transmission = get_dark_channel(temp_image, 15);
        float *tran = transmission.ptr<float>();
        for (uint32_t i = 0; i < this->size; i++) {
            float diff = std::fabs(atmospheric_light[i % 3] - *dark++);
            float data = 1 - .95 * (*tran);
            diff < diff_threshold && (data *= diff_threshold / diff);
            data > 1.0 && (data = 1.0);
            *tran++ = data;
        }
    } else {
        float *tran = transmission.ptr<float>();
        for (uint32_t i = 0; i < this->size; i++) {
            const float diff = std::fabs(approx_atmospheric_light - *dark);
            float data = 1 - .95 * (*dark++) / approx_atmospheric_light;
            diff < diff_threshold && (data *= diff_threshold / diff);
            data > 1.0 && (data = 1.0);
            *tran++ = data;
        }
    }
    return transmission;
}

cv::Mat DehazeImpl::subsample(const cv::Mat &image, const uint32_t sampling_ratio) {
    cv::Mat sub_image;
    cv::resize(image, sub_image, cv::Size(round(this->cols / sampling_ratio), round(this->rows / sampling_ratio)));
    return sub_image;
}

cv::Mat DehazeImpl::upsample(const cv::Mat &image, const uint32_t sampling_ratio) {
    cv::Mat up_image;
    cv::resize(image, up_image, cv::Size(this->cols, this->rows));
    return up_image;
}

cv::Mat DehazeImpl::mean_filter(const cv::Mat &image, const uint32_t radius) {
    cv::Mat mean_filtered_image;
    cv::boxFilter(image, mean_filtered_image, CV_32FC1, cv::Size(radius, radius));
    return mean_filtered_image;
}

cv::Mat DehazeImpl::guided_filter(const cv::Mat &guidance_image, const cv::Mat &source_image, const uint32_t radius, const double epsilon, const uint32_t sampling_ratio) {
    cv::Mat sub_guidance_image = subsample(guidance_image, sampling_ratio);
    cv::Mat sub_source_image = subsample(source_image, sampling_ratio);
    const uint32_t sub_radius = round(radius / sampling_ratio);
    cv::Mat mean_guidance_image = mean_filter(sub_guidance_image, sub_radius);
    cv::Mat mean_source_image = mean_filter(sub_source_image, sub_radius);
    cv::Mat a = (mean_filter(sub_guidance_image.mul(sub_source_image), sub_radius) - mean_guidance_image.mul(mean_source_image))
        / (mean_filter(sub_guidance_image.mul(sub_guidance_image), sub_radius) - mean_guidance_image.mul(mean_guidance_image) + epsilon);
    cv::Mat b = mean_source_image - a.mul(mean_guidance_image);
    return upsample(mean_filter(a, sub_radius), sampling_ratio).mul(guidance_image) + upsample(mean_filter(b, sub_radius), sampling_ratio);
}

cv::Mat DehazeImpl::get_fine_transmission(const cv::Mat &image, const cv::Mat &transmission) {
    cv::Mat gray_image;
    cvtColor(image, gray_image, CV_BGR2GRAY);
    return guided_filter(gray_image, transmission, 60, .0001, 5);
}

void DehazeImpl::recover_dehazed_image(cv::Mat &image, const cv::Mat &fine_transmission, const cv::Vec3f &atmospheric_light) {
    constexpr float t0 = .1;
    const float atmospheric_blue_light = atmospheric_light[0];
    const float atmospheric_green_light = atmospheric_light[1];
    const float atmospheric_red_light = atmospheric_light[2];
    float *data = image.ptr<float>();
    const float *tran = fine_transmission.ptr<float>();
    for (uint32_t i = 0; i < this->size; i++) {
        float restricted_transmission = std::fmax(*tran++, t0);
        *data = ((*data - atmospheric_blue_light) / restricted_transmission + atmospheric_blue_light) * 255;
        data++;
        *data = ((*data - atmospheric_green_light) / restricted_transmission + atmospheric_green_light) * 255;
        data++;
        *data = ((*data - atmospheric_red_light) / restricted_transmission + atmospheric_red_light) * 255;
        data++;
    }
}

cv::Mat DehazeImpl::get_dehazed_image(const cv::Mat &source_image) {
    cv::Mat image;
    source_image.convertTo(image, CV_32FC3, 1.0 / 255);
    this->rows = image.rows;
    this->cols = image.cols;
    this->size = this->rows * this->cols;
    constexpr uint32_t patch = 15;
    cv::Mat dark_channel = get_dark_channel(image, patch);
    float *dark_channel_start = dark_channel.ptr<float>();
    const int32_t offset = image.ptr<float>() - dark_channel_start;
    std::vector<std::pair<float *, float *>> swap_stack;
    const cv::Vec3f atmospheric_light = get_atmospheric_light(dark_channel_start, offset, 0, this->size - 1, .001 * this->size, swap_stack);
    const cv::Mat transmission = get_transmission(image, dark_channel, atmospheric_light);
    const cv::Mat fine_transmission = get_fine_transmission(image, transmission);
    recover_dehazed_image(image, fine_transmission, atmospheric_light);
    return image;
}

std::string DehazeImpl::trim_uri_protocal(const std::string &uri) {
    return uri.find_first_of("file://") == std::string::npos ? uri : uri.substr(7);
}

std::string DehazeImpl::get_dehazed_uri(const std::string &uri) {
    std::size_t delimiter = uri.find_last_of(".");
    return uri.substr(0, delimiter) + "_dehazed" + uri.substr(delimiter);
}

std::string DehazeImpl::save_dehazed_image(const cv::Mat &dehazed_image, const std::string &uri) {
    std::vector<int32_t> compression_params = { CV_IMWRITE_PNG_COMPRESSION, 9 };
    std::string dehazed_uri = this->get_dehazed_uri(uri);
    cv::imwrite(this->trim_uri_protocal(dehazed_uri), dehazed_image, compression_params);
    return dehazed_uri;
}

std::string DehazeImpl::dehaze(const std::string &uri, const std::string &media) {
    if (media == "image") {
        cv::Mat source_image = cv::imread(this->trim_uri_protocal(uri), 1);
        cv::Mat dehazed_image = this->get_dehazed_image(source_image);
        return this->save_dehazed_image(dehazed_image, uri);
    }
    return uri;
}

}
