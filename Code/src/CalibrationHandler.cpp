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

/**
* CalibrationHandler.cpp
*
* Take picture after a short time,
* drag and drop over it and calibrate.
*/

#include "CalibrationHandler.h"

CalibrationHandler::CalibrationHandler(
    std::string title,
    uint16_t distanceText2Border,
    uint8_t font,
    float textScale,
    cv::Scalar textColor,
    uint8_t textThickness)
    : title(title),
      distanceText2Border(distanceText2Border),
      font(font),
      textScale(textScale),
      textColor(textColor),
      textThickness(textThickness) {
    hsvPtr = 0;

    negDist = 2;
    posDist = 1;

    badColor = cv::Scalar(0, 0, 255);
    goodColor = cv::Scalar(0, 255, 0);
}

CalibrationHandler::~CalibrationHandler() {}

bool CalibrationHandler::calibrate(
    cv::Mat *const img,
    std::vector<double> *const hsvColor,
    std::vector<double> *const factorsColor) {
    /***************************************************************
     * Let user select positives.
     * ************************************************************/
    cv::setMouseCallback(title, clickAndCrop, this);

    if (!drawRectangle("Select ONLY good color", img))
        return false;

    std::vector<cv::Point> truePositiveSquare = squarePoints;
    squarePoints.pop_back();
    squarePoints.pop_back();

    /******************************************************************
     * Let user select non-negatives (more than all positives).
     * ***************************************************************/
    if (!drawRectangle("Select MORE than good color!", img))
        return false;

    /*************************************************************
     * Calculate sizes for preallocation
     * **********************************************************/
    img->copyTo(imgCopy);
    cv::putText(
        imgCopy,
        "Calibrating ...",
        cv::Point(
            distanceText2Border,
            imgCopy.rows - distanceText2Border),
        font,
        textScale,
        textColor,
        textThickness);

    uint16_t smallestX, biggestX, smallestY, biggestY;

    findMinMaxXY(squarePoints, &smallestX, &biggestX, &smallestY, &biggestY);

    // Above square
    uint16_t numX = ceil(imgCopy.cols/ static_cast<float>(negDist));
    uint16_t numY = ceil(smallestY / static_cast<float>(negDist));
    uint64_t numAll = numX * numY;

    // Below square
    numY = ceil((imgCopy.rows - biggestY) / static_cast<float>(negDist));
    numAll += numX * numY;

    // Left next to square
    numX = ceil(smallestX / static_cast<float>(negDist));
    numY = ceil((biggestY - smallestY) / static_cast<float>(negDist));
    numAll += numX * numY;

    // Right next to square
    numX = ceil((imgCopy.cols - biggestX) / static_cast<float>(negDist));
    numAll += numX * numY;

    // Only good values
    uint16_t goodSmallestX, goodSmallestY, goodBiggestX, goodBiggestY;
    findMinMaxXY(
        truePositiveSquare,
        &goodSmallestX,
        &goodBiggestX,
        &goodSmallestY,
        &goodBiggestY);

    numX = ceil((goodBiggestX - goodSmallestX) / static_cast<float>(posDist));
    numY = ceil((goodBiggestY - goodSmallestY) / static_cast<float>(posDist));
    uint32_t numAllGoodValues = numX * numY;
    uint16_t mult = numAll / (numAllGoodValues);
    numAll += mult * numAllGoodValues;

    /**************************************************************
     * Fill matrices with wrong colors.
     * ***********************************************************/
    hsvValues = Matrix8u(numAll, 3);

    uint64_t counter = 0;
    img->copyTo(imgCopy);
    cv::Mat hsv;
    cv::cvtColor(imgCopy, hsv, cv::COLOR_BGR2HSV);
    hsvPtr = reinterpret_cast<uint8_t*>(hsv.data);

    for (int col = 0; col < imgCopy.cols; col += negDist) {
        // Above square
        for (int row = 0; row < smallestY; row += negDist) {
            fillMatricesWithBadValues(imgCopy.cols, row, col, counter);
            ++counter;
        }

        // Below square
        for (int row = biggestY; row < imgCopy.rows; row += negDist) {
            fillMatricesWithBadValues(imgCopy.cols, row, col, counter);
            ++counter;
        }
    }

    // Left next to square
    for (int row = smallestY; row < biggestY; row += negDist) {
        for (int col = 0; col < smallestX; col += negDist) {
            fillMatricesWithBadValues(imgCopy.cols, row, col, counter);
            ++counter;
        }

        // Right next to square
        for (int col = biggestX; col < imgCopy.cols; col += negDist) {
            fillMatricesWithBadValues(imgCopy.cols, row, col, counter);
            ++counter;
        }
    }

    /********************************************************
     * Collect positive values and create histograms.
     * *****************************************************/
    setStartIdxPositives(counter);
    results = Eigen::VectorXd::Zero(numAll);
    Eigen::VectorXi hueHist = Eigen::VectorXi::Zero(256);
    Eigen::VectorXi satHist = Eigen::VectorXi::Zero(256);
    Eigen::VectorXi valHist = Eigen::VectorXi::Zero(256);

    for (uint32_t col = goodSmallestX; col < goodBiggestX; col += posDist) {
        for (uint32_t row = goodSmallestY; row < goodBiggestY; row += posDist) {
            line(imgCopy, cv::Point(col, row), cv::Point(col, row), goodColor);

            // Fill histograms.
            ++hueHist(
                hsvPtr[
                    row * imgCopy.cols * HSV_CHANNELS
                    + col * HSV_CHANNELS]);

            ++satHist(
                hsvPtr[
                    row * imgCopy.cols * HSV_CHANNELS
                    + col * HSV_CHANNELS
                    + 1]);

            ++valHist(
                hsvPtr[
                    row * imgCopy.cols * HSV_CHANNELS
                    + col * HSV_CHANNELS
                    + 2]);

            for (uint16_t m = 0; m < mult; ++m) {
                hsvValues(counter, 0) = hsvPtr[
                                            row * imgCopy.cols * HSV_CHANNELS
                                            + col * HSV_CHANNELS];
                hsvValues(counter, 1) = hsvPtr[
                                            row * imgCopy.cols * HSV_CHANNELS
                                            + col * HSV_CHANNELS
                                            + 1];
                hsvValues(counter, 2) = hsvPtr[
                                            row * imgCopy.cols * HSV_CHANNELS
                                            + col * HSV_CHANNELS
                                            + 2];

                results(counter) = 0.95;
                ++counter;
            }
        }
    }

    std::ofstream hsvValuesFile;
    hsvValuesFile.open("hsvValues.txt");
    hsvValuesFile << hsvValues << std::endl;
    hsvValuesFile.close();

    cv::imshow(title, imgCopy);
    int16_t key = cv::waitKey(0);

    if (key == KEY_ESC) {
        return false;
    }

    cv::putText(
        imgCopy,
        "Calculating ...",
        cv::Point(distanceText2Border, imgCopy.rows - distanceText2Border),
        font,
        textScale,
        textColor,
        textThickness);
    cv::imshow(title, imgCopy);
    cv::waitKey(WAIT_TIME);

    /**********************************************************
     * Find optimal values using histograms.
     * *******************************************************/
    int maxHue = 0;
    int maxSat = 0;
    int maxVal = 0;

    for (int i = 0; i < hueHist.size(); ++i) {
        if (maxHue < hueHist(i)) {
            optimalValuesIntern(0) = i;
            maxHue = hueHist(i);
        }

        if (maxSat < satHist(i)) {
            optimalValuesIntern(1) = i;
            maxSat = satHist(i);
        }

        if (maxVal < valHist(i)) {
            optimalValuesIntern(2) = i;
            maxVal = valHist(i);
        }
    }

    std::cout
            << "optimal start values in hsv = "
            << optimalValuesIntern(0)
            << " "
            << optimalValuesIntern(1)
            << " "
            << optimalValuesIntern(2)
            << std::endl;

    /*************************************************************
     * Start calibration for factors (tolerances) of colors
     * **********************************************************/
    std::cout << "starting calibration" << std::endl;

    calculate();
    std::cout
            << "factors for bright = "
            << std::endl
            << errorFactorsIntern(0)
            << " "
            << errorFactorsIntern(1)
            << " "
            << errorFactorsIntern(2)
            << std::endl;

    /***************************************************************
     * Visualisation of bright color search and optional save.
     * ************************************************************/
    img->copyTo(imgCopy);
    bool acceptValues =  visualizeResult(img);

    if (acceptValues) {
        (*hsvColor)[0] = optimalValuesIntern(0);
        (*hsvColor)[1] = optimalValuesIntern(1);
        (*hsvColor)[2] = optimalValuesIntern(2);
        (*factorsColor)[0] = errorFactorsIntern[0];
        (*factorsColor)[1] = errorFactorsIntern[1];
        (*factorsColor)[2] = errorFactorsIntern[2];
    }

    *hsvPtr = 0;
    squarePoints.pop_back();
    squarePoints.pop_back();
    squarePoints.pop_back();

    return acceptValues;
}

void CalibrationHandler::findMinMaxXY(
    const std::vector<cv::Point> &square,
    uint16_t *const smallestX,
    uint16_t *const biggestX,
    uint16_t *const smallestY,
    uint16_t *const biggestY) {
    if (square[0].x < square[1].x) {
        *smallestX = square[0].x;
        *biggestX = square[1].x;
    } else {
        *smallestX = square[1].x;
        *biggestX = square[0].x;
    }

    if (square[0].y < square[1].y) {
        *smallestY = square[0].y;
        *biggestY = square[1].y;
    } else {
        *smallestY = square[1].y;
        *biggestY = square[0].y;
    }
}

double CalibrationHandler::getProbability(const double squaredError) {
    return (1 / (1 + squaredError) );
}

double CalibrationHandler::getPrediction(
    const uint16_t &row,
    const uint16_t &col) {
    if (hsvPtr != 0) {
        int16_t errHue = optimalValuesIntern(0) - hsvPtr[
                      row * imgCopy.cols * HSV_CHANNELS
                      + col * HSV_CHANNELS];
        int16_t errSat = optimalValuesIntern(1) - hsvPtr[
                      row * imgCopy.cols * HSV_CHANNELS
                      + col * HSV_CHANNELS
                      + 1];
        int16_t errVal = optimalValuesIntern(2) - hsvPtr[
                      row * imgCopy.cols * HSV_CHANNELS
                      + col * HSV_CHANNELS
                      + 2];

        return getProbability(
                   pow(errHue, 2) * errorFactorsIntern(0)
                   + pow(errSat, 2) * errorFactorsIntern(1)
                   + pow(errVal, 2) * errorFactorsIntern(2));
    } else {
        return -1;
    }
}

void CalibrationHandler::calculate() {
    Eigen::IOFormat ShortFormat(4);
    setMaxIterationFactors(50);
    setMinErrorFactors(80);  // false positives
    setMinCorrectionFactors(1e-2);

    errorFactorsIntern = {
        START_VALUE_FACTORS,
        START_VALUE_FACTORS,
        START_VALUE_FACTORS
    };

    if (!calculateFactors()) {
        std::cout << "could not calculate factors" << std::endl;
    }
}

bool CalibrationHandler::calculateFactors() {
    Eigen::IOFormat ShortFormat(4);

    double errHue;
    double errSat;
    double errVal;

    Eigen::Vector3d dx(3);

    Eigen::MatrixXd A(hsvValues.rows(), hsvValues.cols());
    Eigen::BDCSVD<Eigen::MatrixXd> svdSolver(hsvValues.rows(), hsvValues.cols(),
        Eigen::ComputeThinU|Eigen::ComputeThinV);

    double sumDenominator;
    double squaredSumDenominator;

    uint16_t iterations = 0;

    while (iterations < getMaxIterationFactors()) {
        for (int i = 0; i < hsvValues.rows(); ++i) {
            // derivative(1/(1+(u*(x-a)^2+v*(y-b)^2+w*(z-c)^2)), u)
            // = -1 * (x-a)^2 / (u*(x-a)^2 + v*(y-b)^2 + w*(z-c)^2 + 1)^2
            errHue = optimalValuesIntern(0) - hsvValues(i, 0);  // x-a
            errSat = optimalValuesIntern(1) - hsvValues(i, 1);  // y-b
            errVal = optimalValuesIntern(2) - hsvValues(i, 2);  // z-c

            sumDenominator = 1
                             + pow(errHue, 2) * errorFactorsIntern(0)
                             + pow(errSat, 2) * errorFactorsIntern(1)
                             + pow(errVal, 2) * errorFactorsIntern(2);
            squaredSumDenominator = pow(sumDenominator, 2);

            A(i, 0) = -pow(errHue, 2) / squaredSumDenominator;
            A(i, 1) = -pow(errSat, 2) / squaredSumDenominator;
            A(i, 2) = -pow(errVal, 2) / squaredSumDenominator;
        }

        svdSolver.compute(A);

        if (svdSolver.info() != Eigen::Success) {
            std::cout << "could not compute solver" << std::endl;
            return false;
        }

        std::ofstream dxFile;
        dxFile.open("dx.txt");
        dxFile << dx << std::endl;
        dxFile.close();

        dx = svdSolver.solve(results);

        if (svdSolver.info() != Eigen::Success) {
            std::cout << "could not solve computation" << std::endl;
            return false;
        }

        errorFactorsIntern += dx;

        if (dx(0) < 0) dx(0) *= -1;
        if (dx(1) < 0) dx(1) *= -1;
        if (dx(2) < 0) dx(2) *= -1;

        ++iterations;

        if (dx(0) < getMinCorrectionFactors()
                && dx(1) < getMinCorrectionFactors()
                && dx(2) < getMinCorrectionFactors()) {
            if (getMinErrorFactors() != -1) {
                if (getFalsePositives(getStartIdxPositives()) < getMinErrorFactors()) {
                    break;
                }
            } else {
                break;
            }
        }
    }

    std::cout
            << "used iterations for calculation of error factors = "
            << unsigned(iterations)
            << std::endl;
    return true;
}


bool CalibrationHandler::visualizeResult(cv::Mat *const img) {
    for (uint16_t col = 0; col < imgCopy.cols; col += negDist) {
        // Above
        for (uint16_t row = 0; row < squarePoints[0].y; row += negDist) {
            addPointInImg(row, col);
        }

        // Below
        for (int row = squarePoints[1].y; row < imgCopy.rows; row += negDist) {
            addPointInImg(row, col);
        }
    }

    // Left
    for (int row = squarePoints[0].y; row < squarePoints[1].y; row += negDist) {
        for (int col = 0; col < squarePoints[0].x; col += negDist) {
            addPointInImg(row, col);
        }

        // Right
        for (int col = squarePoints[1].x; col < imgCopy.cols; col += negDist) {
            addPointInImg(row, col);
        }
    }

    // Good and more values
    double prediction;

    for (
        int col = squarePoints[0].x;
        col < squarePoints[1].x;
        col += posDist) {
        for (
            int row = squarePoints[0].y;
            row < squarePoints[1].y;
            row += posDist) {
            prediction = getPrediction(row, col);

            if (prediction < 0.5)
                line(
                    imgCopy,
                    cv::Point(col, row),
                    cv::Point(col, row),
                    badColor);
            else
                line(
                    imgCopy,
                    cv::Point(col, row),
                    cv::Point(col, row),
                    goodColor);
        }
    }

    cv::imshow(title, imgCopy);

    int key = cv::waitKey(0);

    if (key == KEY_D) {
        cv::imwrite("debug_image.jpg", *img);
        std::ofstream saveFile;
        saveFile.open("debug_hsv.txt");

        saveFile << unsigned(optimalValuesIntern(0)) << std::endl;
        saveFile << unsigned(optimalValuesIntern(1)) << std::endl;
        saveFile << unsigned(optimalValuesIntern(2)) << std::endl;
        saveFile << errorFactorsIntern(0) << std::endl;
        saveFile << errorFactorsIntern(1) << std::endl;
        saveFile << errorFactorsIntern(2) << std::endl;

        saveFile.close();
    } else if (key == KEY_S) {
        return true;
    } else if (key == KEY_ESC) {
        return false;
    }
    return false;
}

uint64_t CalibrationHandler::getFalsePositives(const uint64_t &startGoodValues) {
    uint64_t falsePositives = 0;
    double prediction;
    int16_t errHue;
    int16_t errSat;
    int16_t errVal;

    for (uint64_t i = 0; i < startGoodValues; ++i) {
        errHue = optimalValuesIntern(0) - hsvValues(i, 0);
        errSat = optimalValuesIntern(1) - hsvValues(i, 1);
        errVal = optimalValuesIntern(2) - hsvValues(i, 2);
        prediction = getProbability(
             pow(errHue, 2) * errorFactorsIntern(0)
             + pow(errSat, 2) * errorFactorsIntern(1)
             + pow(errVal, 2) * errorFactorsIntern(2));

        if (prediction >= 0.5) {
            ++falsePositives;
        }
    }

    return falsePositives;
}

void CalibrationHandler::fillMatricesWithBadValues(
    const uint16_t &hsvCols,
    const uint16_t &row,
    const uint16_t &col,
    const uint64_t &pos) {
    hsvValues(pos, 0) = hsvPtr[
                            row * hsvCols * HSV_CHANNELS
                            + col * HSV_CHANNELS];
    hsvValues(pos, 1) = hsvPtr[
                            row * hsvCols * HSV_CHANNELS
                            + col * HSV_CHANNELS
                            + 1];
    hsvValues(pos, 2) = hsvPtr[
                            row * hsvCols * HSV_CHANNELS
                            + col * HSV_CHANNELS
                            + 2];

    cv::line(
        imgCopy,
        cv::Point(col, row),
        cv::Point(col, row),
        badColor);
}

void CalibrationHandler::addPointInImg(const uint16_t row, const uint16_t col) {
    double prediction = getPrediction(row, col);

    if (prediction < 0.5)
        line(imgCopy, cv::Point(col, row), cv::Point(col, row), badColor);
    else
        line(imgCopy, cv::Point(col, row), cv::Point(col, row), goodColor);
}

void CalibrationHandler::clickAndCrop(
    int event,
    int x,
    int y,
    int flags,
    void *userdata) {
    if (userdata != 0) {
        CalibrationHandler* handler =
            reinterpret_cast<CalibrationHandler*>(userdata);
        handler->clickAndCrop(event, x, y);
    }
}

void CalibrationHandler::clickAndCrop(int event, int x, int y) {
    if (event == cv::EVENT_LBUTTONDOWN && squarePoints.size() < 2) {
        squarePoints.push_back(cv::Point(x, y));
        squarePoints.push_back(cv::Point(x, y));
        drawActive = true;
    } else if (event == cv::EVENT_LBUTTONUP) {
        drawActive = false;
    }

    if (event == cv::EVENT_MOUSEMOVE && drawActive) {
        if (squarePoints.size() == 2) {
            squarePoints[1] = cv::Point(x, y);
        }
    }
}

void CalibrationHandler::setFont(const uint8_t font) {
    this->font = font;
}

void CalibrationHandler::setMaxIterationFactors(const uint16_t maxIterationFactors) {
    this->maxIterationFactors = maxIterationFactors;
}

void CalibrationHandler::setMinErrorFactors(const double minErrorFactors) {
    this->minErrorFactors = minErrorFactors;
}

void CalibrationHandler::setMinCorrectionFactors(const double minCorrectionFactors) {
    this->minCorrectionFactors = minCorrectionFactors;
}

void CalibrationHandler::setStartIdxPositives(const uint64_t startIdxPositives) {
    this->startIdxPositives = startIdxPositives;
}

void CalibrationHandler::setNegDist(const uint8_t negDist) {
    this->negDist = negDist;
}

void CalibrationHandler::setPosDist(const uint8_t posDist) {
    this->posDist = posDist;
}

void CalibrationHandler::setTextColor(const cv::Scalar textColor) {
    this->textColor = textColor;
}

void CalibrationHandler::setTextScale(float textScale) {
    this->textScale = textScale;
}

void CalibrationHandler::setTextThickness(const uint8_t textThickness) {
    this->textThickness = textThickness;
}

void CalibrationHandler::setTitle(const std::string &title) {
    this->title = title;
}

uint8_t CalibrationHandler::getFont() const {
    return font;
}

uint16_t CalibrationHandler::getMaxIterationFactors() const {
    return maxIterationFactors;
}

double CalibrationHandler::getMinErrorFactors() const {
    return minErrorFactors;
}

double CalibrationHandler::getMinCorrectionFactors() const {
    return minCorrectionFactors;
}

uint64_t CalibrationHandler::getStartIdxPositives() const {
    return startIdxPositives;
}

uint8_t CalibrationHandler::getNegDist() const {
    return negDist;
}

uint8_t CalibrationHandler::getPosDist() const {
    return posDist;
}

cv::Scalar CalibrationHandler::getTextColor() const {
    return textColor;
}

float CalibrationHandler::getTextScale() const {
    return textScale;
}

uint8_t CalibrationHandler::getTextThickness() const {
    return textThickness;
}

std::string CalibrationHandler::getTitle() const {
    return title;
}

bool CalibrationHandler::drawRectangle(
        std::string description, cv::Mat *const img) {
    img->copyTo(imgCopy);
    cv::putText(
        imgCopy,
        description,
        cv::Point(distanceText2Border, imgCopy.rows-distanceText2Border),
        font,
        textScale,
        textColor,
        textThickness);

    int key;

    while (true) {
        cv::imshow(title, imgCopy);

        // Draw rectangle.
        if (squarePoints.size() == POINTS_OF_RECTANGLE) {
            img->copyTo(imgCopy);
            cv::putText(
                imgCopy,
                description,
                cv::Point(
                    distanceText2Border,
                    imgCopy.rows - distanceText2Border),
                font,
                textScale,
                textColor,
                textThickness);
            cv::rectangle(
                imgCopy,
                squarePoints[0],
                squarePoints[1],
                cv::Scalar(0, 0, 0),
                2);
        }

        key = cv::waitKey(WAIT_TIME);

        if (key == KEY_R) {
            for (int i = 0; i < POINTS_OF_RECTANGLE; ++i) {
                squarePoints.pop_back();
            }
            img->copyTo(imgCopy);
            cv::putText(
                imgCopy,
                "Select ONLY good color",
                cv::Point(
                    distanceText2Border,
                    imgCopy.rows - distanceText2Border),
                font,
                textScale,
                textColor,
                textThickness);
        } else if (key == KEY_ACCEPT) {
            return true;
        } else if (key == KEY_ESC) {
            return false;
        }
    }
}
