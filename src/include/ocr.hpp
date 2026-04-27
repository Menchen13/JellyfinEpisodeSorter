#pragma once
#include <expected>

#include <opencv2/core/mat.hpp>


#include "models.hpp"


// function to be called asyncrously in batches
// completes full JellyfinId to Titlecard Text with ocr and wraps errors/exceptions in pipeline in
// expected object
std::expected<OcrResult, std::string> idToTitle(const std::string &url, const std::string &apiKey, const std::string &id, const std::string &googleApiKey) noexcept;


// takes in a streamUrl to the episode and the targetSecond to Jump to
// returns a cv::Mat containing the frame with the Titlecard at targetSecond
cv::Mat getTitlecard(const std::string& streamUrl, const unsigned int& targetSecond);

// takes in titlecard frame and returns a base64 encoded string
// representing the image
// *WARINING* alters frame prarameter permanently
std::string processTitlecard(cv::Mat& frame);
