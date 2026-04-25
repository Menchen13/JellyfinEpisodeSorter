#include <opencv2/core/types.hpp>
#include <stdexcept>
#include <vector>

#include <cppcodec/base64_rfc4648.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include "ocr.hpp"

std::expected<OcrResult, std::string>
idToTitle(std::string_view url, std::string_view apiKey, std::string_view id,
          std::string_view googleApiKey) noexcept {

  return std::unexpected<std::string>("WELP?");
  // catch cv::Exception, runtime_error
}

cv::Mat getTitlecard(const std::string &streamUrl,
                     const unsigned int &targetSecond) {
  cv::VideoCapture video;
  video.setExceptionMode(true);

  // will throw if fails
  video.open(streamUrl.data());

  // set stream to targetMilliSecond
  video.set(cv::CAP_PROP_POS_MSEC, (targetSecond * 1000));

  cv::Mat frame;

  // will throw if fails
  video.read(frame);

  if (frame.empty()) {
    throw std::runtime_error(
        "OpenCV read titlecard successfully, but image is empty.");
  } else {
    return frame;
  }
}

// takes in a cv::Mat frame of the titlecard and returns it
// as a b64 string ready for ocr api
std::string processTitlecard(cv::Mat &frame) {
  std::vector<uchar> bytes;

  cv::resize(frame, frame, cv::Size(854, 480));;

  if (!cv::imencode(".jpg", frame, bytes)) {
    throw std::runtime_error("Failed imencode!");
  }
  std::string b64 = cppcodec::base64_rfc4648::encode(bytes);

  return b64;
}
