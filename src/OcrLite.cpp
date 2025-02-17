#include "OcrLite.h"
#include "OcrUtils.h"
#include <stdarg.h> //windows&linux

OcrLite::OcrLite() {
}

OcrLite::~OcrLite() {
}

void OcrLite::setNumThread(int numOfThread) {
    dbNet.setNumThread(numOfThread);
    angleNet.setNumThread(numOfThread);
    crnnNet.setNumThread(numOfThread);
}

void OcrLite::initModels(const std::string &detPath, const std::string &clsPath,
                         const std::string &recPath, const std::string &keysPath) {
    dbNet.initModel(detPath);
    angleNet.initModel(clsPath);
    crnnNet.initModel(recPath, keysPath);
}

void OcrLite::Logger(const char *format, ...) {
    char *buffer = (char *) malloc(8192);
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    printf("%s", buffer);
    free(buffer);
}

cv::Mat makePadding(cv::Mat &src, const int padding) {
    if (padding <= 0) return src;
    cv::Scalar paddingScalar = {255, 255, 255};
    cv::Mat paddingSrc;
    cv::copyMakeBorder(src, paddingSrc, padding, padding, padding, padding, cv::BORDER_ISOLATED, paddingScalar);
    return paddingSrc;
}

void OcrLite::detect(const char *imgPath, const int padding, const int maxSideLen,
                     float boxScoreThresh, float boxThresh, float unClipRatio, bool doAngle, bool mostAngle) {
    cv::Mat bgrSrc = imread(imgPath, cv::IMREAD_COLOR); //default : BGR
    cv::Mat originSrc;
    cvtColor(bgrSrc, originSrc, cv::COLOR_BGR2RGB); // convert to RGB
    int originMaxSide = (std::max)(originSrc.cols, originSrc.rows);
    int resize;
    if (maxSideLen <= 0 || maxSideLen > originMaxSide) {
        resize = originMaxSide;
    } else {
        resize = maxSideLen;
    }
    resize += 2 * padding;
    cv::Rect paddingRect(padding, padding, originSrc.cols, originSrc.rows);
    cv::Mat paddingSrc = makePadding(originSrc, padding);
    ScaleParam scale = getScaleParam(paddingSrc, resize);
    detect(paddingSrc, paddingRect, scale,
           boxScoreThresh, boxThresh, unClipRatio, doAngle, mostAngle);
}

void OcrLite::detect(const cv::Mat &mat, int padding, int maxSideLen, float boxScoreThresh, float boxThresh,
                     float unClipRatio, bool doAngle, bool mostAngle) {
    cv::Mat originSrc;
    cvtColor(mat, originSrc, cv::COLOR_BGR2RGB); // convert to RGB
    int originMaxSide = (std::max)(originSrc.cols, originSrc.rows);
    int resize;
    if (maxSideLen <= 0 || maxSideLen > originMaxSide) {
        resize = originMaxSide;
    } else {
        resize = maxSideLen;
    }
    resize += 2 * padding;
    cv::Rect paddingRect(padding, padding, originSrc.cols, originSrc.rows);
    cv::Mat paddingSrc = makePadding(originSrc, padding);
    ScaleParam scale = getScaleParam(paddingSrc, resize);
    detect(paddingSrc, paddingRect, scale, boxScoreThresh, boxThresh,
           unClipRatio, doAngle, mostAngle);
}

std::vector<cv::Mat> OcrLite::getPartImages(cv::Mat &src, std::vector<TextBox> &textBoxes) {
    std::vector<cv::Mat> partImages;
    for (size_t i = 0; i < textBoxes.size(); ++i) {
        cv::Mat partImg = getRotateCropImage(src, textBoxes[i].boxPoint);
        partImages.emplace_back(partImg);
    }
    return partImages;
}

void OcrLite::detect(cv::Mat &src, cv::Rect &originRect, ScaleParam &scale,
                     float boxScoreThresh, float boxThresh, float unClipRatio, bool doAngle, bool mostAngle) {
    double startTime = getCurrentTime();
    cv::Mat textBoxPaddingImg = src.clone();
    std::vector<TextBox> textBoxes = dbNet.getTextBoxes(src, scale, boxScoreThresh, boxThresh, unClipRatio);
    std::vector<cv::Mat> partImages = getPartImages(src, textBoxes);
    std::vector<Angle> angles = angleNet.getAngles(partImages, doAngle, mostAngle);

    for (size_t i = 0; i < partImages.size(); ++i) {
        if (angles[i].index == 0) {
            partImages.at(i) = matRotateClockWise180(partImages[i]);
        }
    }

    std::vector<TextLine> textLines = crnnNet.getTextLines(partImages);
    for (size_t i = 0; i < textLines.size(); ++i) {
        Logger("%s\n", textLines[i].text.c_str());
        std::vector<cv::Point> boxPoint = std::vector<cv::Point>(4);
        int padding = originRect.x; //padding conversion
        boxPoint[0] = cv::Point(textBoxes[i].boxPoint[0].x - padding, textBoxes[i].boxPoint[0].y - padding);
        boxPoint[1] = cv::Point(textBoxes[i].boxPoint[1].x - padding, textBoxes[i].boxPoint[1].y - padding);
        boxPoint[2] = cv::Point(textBoxes[i].boxPoint[2].x - padding, textBoxes[i].boxPoint[2].y - padding);
        boxPoint[3] = cv::Point(textBoxes[i].boxPoint[3].x - padding, textBoxes[i].boxPoint[3].y - padding);
        Logger("%d,%d|%d,%d|%d,%d|%d,%d\n",
               boxPoint[0].x, boxPoint[0].y,
               boxPoint[1].x, boxPoint[1].y,
               boxPoint[2].x, boxPoint[2].y,
               boxPoint[3].x, boxPoint[3].y);
        Logger("---\n");
    }
    Logger("%f", getCurrentTime() - startTime);
}
