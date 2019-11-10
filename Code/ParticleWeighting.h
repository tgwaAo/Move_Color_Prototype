// Modified MIT License
//
// Copyright (c) 2019 tgwaAo
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Modified part:
//
// THIS SOFTWARE DOES NOT CHECK YOUR SURROUNDINGS NOR DOES IT CONTROL YOUR
// MOVEMENT, SO IT IS UNDER YOUR OWN RESPONSIBILITY TO ENSURE NOBODY GETS HURT
// AND NOTHING GETS DAMAGED. PLAY CAREFULLY AND CHECK YOUR SURROUNDINGS BEFORE
// PLAYING.

#ifndef PARTICLE_WEIGHTING_H
#define PARTICLE_WEIGHTING_H


#include <math.h>
#include <cstdint>
#include <vector>


/**
 * @class ParticleWeighting
 * @author me
 * @date 09/10/19
 * @file ParticleWeighting.h
 * @brief Decide, whether a color is in this area or not via particles.
 */
class ParticleWeighting
{
public:
    /**
     * @brief Constructs a continuous matrix of particles in given area
     * and initializes necessary attributes.
     * @param numParticles_ Number of particles.
     * @param minWidth_ Where width starts.
     * @param maxWidth_ Where width ends.
     * @param minHeight_ Where height starts.
     * @param maxHeight_ Where height ends.
     * @param maxWeight_ Weight of a particle without error.
     * @param cols_ Number of horizontal pixels.
     * @param bound_ Minimal number of positive particles.
     * @param factors Factors multiplied by squared error.
     * @param hsvBest HSV values without error.
     */
    ParticleWeighting(
        const uint16_t &numParticles_,
        const double &minWidth_,
        const double &maxWidth_,
        const double &minHeight_,
        const double &maxHeight_,
        const uint16_t &maxWeight_,
        const uint16_t &cols_,
        const uint32_t bound_,
        const std::vector<double> &factors,
        const std::vector<uint16_t> &hsvBest);

    /**
     * @brief Calculate sum of weights.
     * @param pixelPtr Pointer pointing to current image.
     * @return Sum of weights of particles.
     */
    uint32_t calculateSumWeights(const uint8_t *pixelPtr);

    /**
     * @brief Decide, wheter area contains color or not.
     * @param pixelPtr Pointer pointing to current image.
     * @return Decision of containing color.
     */
    bool isColor(const uint8_t *pixelPtr);

    /**
     * @brief Update position of particles after changed size.
     */
    void update();

    /**
     * @brief Get values of a particle.
     * @param idx_ Index of wanted particle.
     * @param backX X position of particle.
     * @param backY Y position of particle.
     * @param backW Weight of particle.
     * @param alright Is alright, if idx_ is in valid range.
     */
    void getParticle(
        const uint16_t &idx_,
        uint16_t backX,
        uint16_t backY,
        uint16_t backW,
        bool alright) const;

    /**
     * @brief Set values of a particle.
     * @param idx_ Index of interesting particle.
     * @param change_x_ Next x position of particle.
     * @param change_y_ Next y position of particle.
     * @param change_w_ Next weight of particle.
     */
    void setParticle(
        const uint16_t &idx_,
        const uint16_t &changeX_,
        const uint16_t &changeY_,
        const uint16_t &changeW_);

    /**
     * @brief Get sum of weights.
     * @return Sum of weights.
     */
    uint32_t getSumWeights() const;

    /**
     * @brief Set sum of weights.
     * @param sum_weights New sum of weights.
     */
    void setSumWeights(const uint32_t& sumWeights);

    /**
     * @brief Get weight of no error.
     * @return Weight of no error.
     */
    uint16_t getMaxWeight() const;

    /**
     * @brief Set maximum possible weight.
     * @param value Maximum possible weight.
     */
    void setMaxWeight(const uint16_t &value);

    /**
     * @brief Get horizontal start of area.
     * @return Horizontal start of area.
     */
    uint16_t getMinWidth() const;

    /**
     * @brief Set horizontal start of area.
     * @param value Horizontal start of area.
     */
    void setMinWidth(const uint16_t &value);

    /**
     * @brief Get vertical start of area.
     * @return Vertical start of area.
     */
    uint16_t getMinHeight() const;

    /**
     * @brief Set vertical start of area.
     * @param value Vertical start of area.
     */
    void setMinHeight(const uint16_t &value);

    /**
     * @brief Get horizontal end of area.
     * @return Horizontal end of area.
     */
    uint16_t getMaxWidth() const;

    /**
     * @brief Set horizontal end of area.
     * @param value Horizontal end of area.
     */
    void setMaxWidth(const uint16_t &value);

    /**
     * @brief Get vertical end of area.
     * @return Vertical end of area.
     */
    uint16_t getMaxHeight() const;

    /**
     * @brief Set vertical end of area.
     * @param value Vertical end of area.
     */
    void setMaxHeight(const uint16_t &value);

    /**
     * @brief Get optimal hue.
     * @return Optimal value.
     */
    uint16_t getHue() const;

    /**
     * @brief Set optimal hue.
     * @param value Optimal hue.
     */
    void setHue(const uint16_t &value);

    /**
     * @brief Get optimal saturation.
     * @return Optimal saturation.
     */
    uint16_t getSat() const;

    /**
     * @brief Set optimal saturation.
     * @param value Optimal saturation.
     */
    void setSat(const uint16_t &value);

    /**
     * @brief Get optimal saturation.
     * @return Optimal saturation.
     */
    uint16_t getVal() const;

    /**
     * @brief Set optimal value
     * @param value
     */
    void setVal(const uint16_t &value);

    /**
     * @brief Get factor multiplied by squared hue error.
     * @return Factor multiplied by squared hue error.
     */
    double getFactorH() const;

    /**
     * @brief Set factor multiplied by squared hue error.
     * @param value Factor multiplied by squared hue error.
     */
    void setFactorH(double value);

    /**
     * @brief Get factor multiplied by squared saturation error.
     * @return Factor multiplied by squared saturation error.
     */
    double getFactorS() const;

    /**
     * @brief Set factor multiplied by squared saturation error.
     * @param value Factor multiplied by squared saturation error.
     */
    void setFactorS(double value);

    /**
     * @brief Get factor multiplied by squared value error.
     * @return Factor multiplied by squared value error.
     */
    double getFactorV() const;

    /**
     * @brief Set factor multiplied by squared value error.
     * @param value Factor multiplied by squared value error.
     */
    void setFactorV(double value);

    /**
     * @brief Get vector of optimal hsv values.
     * @return Vector of optimal hsv values.
     */
    std::vector<uint16_t> getHsv() const;

    /**
     * @brief Set optimal hsv values.
     * @param hsv Optimal hsv values.
     * @return Returns true, if input vector has a length of 3.
     */
    bool setHsv(const std::vector<uint16_t> &hsv);

    /**
     * @brief Get factors of squared errors of hsv values.
     * @return Factors of squared errors of hsv values.
     */
    std::vector<double> getFactors() const;

    /**
     * @brief Set factors of squared errors of hsv values.
     * @param factors Factors of squared errors of hsv values.
     * @return Returns true, if input vector has a length of 3.
     */
    bool setFactors(const std::vector<double> &factors);

private:
    const uint8_t HSV_CHANNELS = 3;

    std::vector<uint16_t> particlesX;
    std::vector<uint16_t> particlesY;
    std::vector<uint16_t> particlesW;
    uint32_t bound;
    uint32_t sumWeights;
    uint16_t maxWeight;
    uint16_t minWidth;
    uint16_t minHeight;
    uint16_t maxWidth;
    uint16_t maxHeight;
    uint16_t cols;
    uint16_t hue;
    uint16_t sat;
    uint16_t val;
    double factorH;
    double factorS;
    double factorV;
};

#endif  // PARTICLE_WEIGHTING_H
