#pragma once
#include <expected>
#include <functional>

#include <opencv2/core/mat.hpp>


#include "models.hpp"

using ocrProvider = std::function<std::string(const std::string&)>;

// custom exception type for geminiAPI ratelimit
struct RateLimitException : public std::runtime_error {
    RateLimitException() : std::runtime_error("API Rate Limit Exceeded (HTTP 429/503)") {}
};

// function to be called asyncrously in batches
// completes full JellyfinId to Titlecard Text with ocr and wraps errors/exceptions in pipeline in
// expected object
std::expected<OcrResult, std::string> idToTitle(const std::string &url, const std::string &apiKey, const std::string &id, const std::string &googleApiKey) noexcept;

std::vector<OcrResult> idToTitlePipeline(const std::vector<Episode> &episodes, const std::string &jellyfinUrl, const unsigned int &targetSecond, ocrProvider ocrCallback);


// takes in a streamUrl to the episode and the targetSecond to Jump to
// returns a cv::Mat containing the frame with the Titlecard at targetSecond
cv::Mat getTitlecard(const std::string& streamUrl, const unsigned int& targetSecond);

// takes in titlecard frame and returns a base64 encoded string
// representing the image
// *WARNING* alters frame parameter permanently
std::string processTitlecard(cv::Mat& frame);
