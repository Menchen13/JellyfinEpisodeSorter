#include <format>
#include <stdexcept>
#include <thread>
#include <vector>

#include <cppcodec/base64_rfc4648.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include "ocr.hpp"

std::expected<OcrResult, std::string>
idToTitle(const std::string &url, const std::string &apiKey,
          const std::string &id, const std::string &googleApiKey) noexcept {

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
// destorys passed in frame!!!
std::string processTitlecard(cv::Mat &frame) {
  std::vector<uchar> bytes;

  cv::resize(frame, frame, cv::Size(854, 480));

  if (!cv::imencode(".jpg", frame, bytes)) {
    throw std::runtime_error("Failed imencode!");
  }
  std::string b64 = cppcodec::base64_rfc4648::encode(bytes);

  return b64;
}

std::vector<OcrResult> idToTitlePipeline(const std::vector<Episode> &episodes,
                                         const std::string &jellyfinUrl,
                                         const unsigned int &targetSecond,
                                         ocrProvider ocrCallback) {
  std::vector<OcrResult> results;
  results.reserve(episodes.size());

  bool displayTimeEstimate = true;

  for (size_t i = 0; i < episodes.size();
       /*nothing here body of loop increments conditionally*/) {
    try {
      std::string streamUrl = std::format("{}/Videos/{}/stream?static=true",
                                          jellyfinUrl, episodes.at(i).id);

      cv::Mat frame = getTitlecard(streamUrl, targetSecond);

      std::string base64 = processTitlecard(frame);

      // if this is the googleAPI it may throw <RATELIMIT Exception> TODO
      std::string title = ocrCallback(base64);

      results.emplace_back(episodes.at(i).id, title);

      i++;

    } catch (const RateLimitException &e) {
      if (displayTimeEstimate) {
        std::println("{} after {} titlecards!", e.what(), (i + 1));
        const unsigned int cycles{
            static_cast<unsigned int>(std::ceil(episodes.size() / (i + 1)))};
        std::println("With {} total titlecards this is gonna take {} "
                     "minutes!\nIf you want it to go faster consider local-LLM "
                     "approach or paying for a better geminiAPI Service-Tier.",
                     episodes.size(), cycles);

        // message should only be printed the first time rate limit is hit
        displayTimeEstimate = false;
      }

      // sleep for 1 min to reset rate limit
      std::this_thread::sleep_for(std::chrono::seconds(60));
    }
  }
  return results;
}
