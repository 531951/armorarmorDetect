//
// Created by zhuye on 2026/1/15.
//

#include "ArmorDetect.h"

void drawTetragon(cv::Mat& image, cv::Point2f* vertices, const cv::Scalar& color) {
    int thickness = (int)ceil(2e-3 * image.cols);
    for (int j = 0; j < 4; j++) {
        cv::line(image, vertices[j], vertices[(j + 1) % 4], color, thickness);
    }
    int radius = (int)ceil(1e-2 * image.cols);
}

std::tuple<cv::Mat, cv::Mat> ArmorDetect::findRedPreprocess(const cv::Mat& frame) const {
    std::vector<cv::Mat> channels; // BGR
    cv::split(frame, channels);

    cv::Mat highlightImg;
    cv::threshold(channels[1], highlightImg, this->params.highlightThresholdForRed, 255, cv::THRESH_BINARY);

    cv::Mat colorImg = channels[2] - channels[0];
    cv::threshold(colorImg, colorImg, this->params.colorThresholdForRed, 255, cv::THRESH_BINARY);
    cv::erode(colorImg, colorImg, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));

    return std::make_tuple(highlightImg, colorImg);
}

std::tuple<cv::Mat, cv::Mat> ArmorDetect::findBluePreprocess(const cv::Mat& frame) const {
    std::vector<cv::Mat> channels; // BGR
    cv::split(frame, channels);

    cv::Mat highlightImg;
    cv::threshold(channels[1], highlightImg, this->params.highlightThresholdForBlue, 255, cv::THRESH_BINARY);

    cv::Mat colorImg = channels[0] - channels[2];
    cv::threshold(colorImg, colorImg, this->params.colorThresholdForBlue, 255, cv::THRESH_BINARY);

    return std::make_tuple(highlightImg, colorImg);
}

std::vector<LightBar> ArmorDetect::findLightBars(const cv::Mat& highlight, const cv::Mat& color) const {
    ContoursVector colorContours;
    ContoursVector highlightContours;

    cv::findContours(color, colorContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::findContours(highlight, highlightContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::RotatedRect> colorBoundingRRects;

    for (const auto& c : colorContours) {
        colorBoundingRRects.push_back(cv::minAreaRect(c));
    }

    std::vector<LightBar> lightBars;

    for (auto&& h : highlightContours) {
        auto box = cv::minAreaRect(h);
        auto boxArea = box.size.width * box.size.height;

        if (boxArea < this->params.minLightBarArea) continue;

        auto width = box.size.width;
        auto height = box.size.height;
        auto angle = box.angle;

        if (height > width) { // light bar like: '//'
            angle = 90 - angle;
        }
        else { // light bar like: '\\'
            std::swap(height, width);
            angle = 180 - angle;
        }

        auto ratio = height / width;

        if (ratio > Armor::maxLightBarLengthWidthRatio) continue;
        if (angle < Armor::lightBarMinAngle || angle > Armor::lightBarMaxAngle) continue;

        bool inside = false;

        for (const auto& r : colorBoundingRRects) {
            std::vector<cv::Point2f> intersectingRegion;
            if (cv::rotatedRectangleIntersection(r, box, intersectingRegion) != cv::INTERSECT_NONE) {
                inside = true;
                break;
            }
        }

        if (inside) {
            lightBars.emplace_back(box, angle, height, width);
        }
    }

    return lightBars;
}

std::vector<Armor> ArmorDetect::lightBarsPairing(const std::vector<LightBar>& lightBars) const {
    if (lightBars.size() < 2) return std::vector<Armor>();

    std::vector<Armor> armors;

    for (size_t i = 0; i < lightBars.size() - 1; i++) {
        for (size_t j = i + 1; j < lightBars.size(); j++) {
            auto maxBarLength = std::max(lightBars[i].length, lightBars[j].length);

            if (abs(lightBars[i].angle - lightBars[j].angle) > Armor::maxAngleBetweenLightBars) continue;
            if (abs(lightBars[i].length - lightBars[j].length) / maxBarLength > 0.5f) continue;
            if (abs(lightBars[i].center.x - lightBars[j].center.x) / maxBarLength > Armor::maxArmorWidthToLightBarLength) continue;
            if (abs(lightBars[i].center.y - lightBars[j].center.y) > this->params.lightBarsCenterMaxDiffY) continue;

            armors.emplace_back(lightBars[i], lightBars[j]);
        }
    }
    return armors;
}

Armor ArmorDetect::choice(const std::vector<Armor>& armors) {
    if (armors.empty()) {
        return Armor(); // 返回默认构造的Armor
    }
    return armors[0];
}