//
// Created by zhuye on 2026/1/23.
//

#ifndef MUC_CLEMENTINE_ARMOR_H
#define MUC_CLEMENTINE_ARMOR_H

#include <opencv2/opencv.hpp>
#include <utility>
#include <cmath>

struct LightBar {
    cv::RotatedRect box;
    float angle{};
    float length{};
    float thickness{};
    cv::Point2f center;

    LightBar() = default;

    LightBar(cv::RotatedRect b, float a, float l, float t)
        : box(std::move(b)), angle(a), length(l), thickness(t), center(b.center) {
    }

    LightBar(cv::RotatedRect&& b, float a, float l, float t)
        : box(std::move(b)), angle(a), length(l), thickness(t), center(b.center) {
    }
};

struct Armor {
public:
    // 静态常量定义
    static constexpr float maxArmorWidthHeightRatio = 3.5f;
    static constexpr float maxArmorWidthToLightBarLength = 4.0f;
    static constexpr float armorHeightToLightBarLength = 2.0f;
    static constexpr float maxLightBarLengthWidthRatio = 10.0f;
    static constexpr float lightBarMinAngle = 60.0f;
    static constexpr float lightBarMaxAngle = 120.0f;
    static constexpr float maxAngleBetweenLightBars = 40.0f;

    // 成员变量 - 按照正确的顺序声明
    LightBar leftBar;
    LightBar rightBar;
    cv::Point2f center;
    float width{};
    float height{};
    int id{};

    // 默认构造函数
    Armor() = default;

    // 构造函数 - 在函数体内完成复杂的