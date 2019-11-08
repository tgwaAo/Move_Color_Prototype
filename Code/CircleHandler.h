#ifndef CIRCLEHANDLER_H
#define CIRCLEHANDLER_H

#include <opencv2/opencv.hpp>
#include <random>

/**
 * @class CircleHandler
 * @author me
 * @date 09/10/19
 * @file CircleHandler.h
 * @brief Class handles circles and enables hits of them (gameplay).
 */
class CircleHandler
{
public:
    /**
     * @brief Default constructor calling other constructor with 10 circles
     *        with a radius of ten. The time of each state is 3 seconds and
     *        initial state is dark (visible, but not active jet). The circles
     *        are blue and created in a window of size 480x640.
     */
    CircleHandler();

    /**
     * @brief Constructor initializing times of states and active state, color
     *        and number of circles and window of creation.
     * @param numCircles Number of used circles.
     * @param radius Radius of used circles.
     * @param circleTimeStates Time of each state each circle has.
     * @param color Color of active state (brightest).
     * @param width Width of possible positions in window.
     * @param height Height of possible positions in window.
     */
    CircleHandler(const uint8_t numCircles, const uint8_t radius_,
                  const std::vector<float> &circleTimeStates,
                  const cv::Scalar& color,
                  const uint16_t width = 640,
                  const uint16_t height = 480);

    /**
     * @brief Destructor of class
     */
    ~CircleHandler();

    /**
     * @brief Update states and drawings.
     * @param img Used image to draw circles.
     */
    void updateCircles(cv::Mat *img);

    /**
     * @brief Check hits of circles at position.
     * @param x Current x-value to check hit.
     * @param y Current y-value to check hit.
     * @return Number of hits.
     */
    uint8_t checkHit(const uint16_t &x, const uint16_t &y);

    /**
     * @class Circle
     * @author me
     * @date 09/10/19
     * @file CircleHandler.h
     * @brief Struct to use circles with a point for position, a timer to switch states
     *        and a current status (invisible, dark = not active, bright = active).
     */
    struct Circle {
        cv::Point point;
        clock_t timerStart;
        uint8_t status;
    };

    /**
     * @brief Set times of different states.
     * @param circleTimeStates Possible states of the circles
     */
    void setCircleTimeStates(const std::vector<float>& circleTimeStates);

    /**
     * @brief Set color of shown circles.
     * @param color Color of shown circles.
     */
    void setColor(const cv::Scalar& color);

    /**
     * @brief Set radius of shown circles.
     * @param radius Radius of shown circles.
     */
    void setRadius(const uint8_t radius);

    /**
     * @brief Get time of state at given index.
     * @param idx Index of time state.
     * @return 
     */
    std::vector<float> getCircleTimeStates() const;

    /**
     * @brief Get color of drawn circles.
     * @return Color of drawn circles.
     */
    cv::Scalar getColor() const;

    /**
     * @brief Get radius of drawn cirlces.
     * @return Radius of drawn circles.
     */
    uint8_t getRadius() const;

private:
    /**
     * @brief New position, state and new time of a circle.
     * @param idx Index of changed circle.
     */
    void newPos(uint8_t idx);

    uint8_t radius_;
    cv::Scalar color_;
    std::vector<float> circleTimeStates_;
    std::vector<Circle> allCircles;
    std::mt19937 eng;
    std::uniform_int_distribution<uint16_t> randomUpDownIdx;
    std::uniform_int_distribution<uint16_t> randomLeftRightIdx;
    
    const uint8_t SIZE_OF_TIME_STATES = 3;
};

#endif // CIRCLEHANDLER_H
