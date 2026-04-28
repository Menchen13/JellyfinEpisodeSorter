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

      // if this is the googleAPI it may throw RateLimitException
      std::string title = ocrCallback(base64);

      results.emplace_back(episodes.at(i).id, title);

      i++;

    } catch (const RateLimitException &e) {
      if (displayTimeEstimate) {
        std::println("{} after {} titlecards!", e.what(), (i + 1));
        const unsigned int cycles{
            static_cast<unsigned int>(std::ceil(episodes.size() / (i + 1)))};
        std::println(
            "If you are using the Free-Tier of the GeminiAPI this there is a "
            "hard rate-timit on it - for me it was 20 Requests per DAY!\n You "
            "can either let this programm run somewhere for a few weeks to "
            "sort a whole series or find somewhere to run the ollama container "
            "with the vision model...");

        // message should only be printed the first time rate limit is hit
        displayTimeEstimate = false;
      }

      // sleep for 1 min to try reset rate limit (wont work sadly)
      std::this_thread::sleep_for(std::chrono::seconds(60));
    }
  }
  return results;
}
