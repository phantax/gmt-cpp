#include "DecisionReader.h"
#include <math.h>
#include <stdexcept>
#include <limits>
#include <iostream>

using std::vector;


/*
 * ___________________________________________________________________________
 */
DecisionReader::DecisionReader(StreamReader& streamer) : streamer_(streamer) {
}


/*
 * ___________________________________________________________________________
 */
bool DecisionReader::readBoolUniform() {

	return this->readBit();
}


/*
 * ___________________________________________________________________________
 */
size_t DecisionReader::readUIntUniform(size_t max) {

	if (max == 0) {
		throw std::runtime_error("DecisionReader::readUIntUniform(...): "
				"'max' needs to be larger than zero");
	}

	// Determine bit width of requested integer
	size_t width = 1;
	while (width < std::numeric_limits<size_t>::digits
            && max > (((size_t)1) << width)) {
		width++;
	}

    // Going to become the random value ...
	size_t value;

	do {
        // Reset value
        value = 0;
        // Add full bytes ...
        size_t w = width;
        while (w >= 8) {
            value = value << 8;
            value = value | (0xFF & this->readByte());
            w -= 8;
        }
        // ... and fill remaining width with bits
        while (w >= 1) {
            value = value << 1;
            if (this->readBit()) {
                value = value | 0x1;
            }
            w -= 1;
        }
	} while (value >= max);

	return value;
}


/*
 * ___________________________________________________________________________
 */
size_t DecisionReader::readUIntExponential(size_t num, size_t denom) {

	size_t value = 0;

	while (this->readUIntUniform(denom) <= num) {
		++value;
	}

	return value;
}


/*
 * ___________________________________________________________________________
 */
size_t DecisionReader::readUIntCustom(const vector<size_t>& pd) {

    // The sum of scores
    size_t sum = 0;
    for (vector<size_t>::const_iterator it = pd.begin(); it != pd.end(); ++it) {
        sum += *it;
    }
    
    // Draw a number at random uniformly distributed between 0 and <sum>
    size_t val = this->readUIntUniform(sum);

    // Map to cumulative probability distribution
    size_t index = 0;
    for (vector<size_t>::const_iterator it = pd.begin(); it != pd.end(); ++it) {
        if (val < *it) {
            return index;
        } else {
            val -= *it;
            ++index;
        }
    }

    // Should never happen
    return 0;
}


/*
 * ___________________________________________________________________________
 */
size_t DecisionReader::readUIntCustom(const vector<double>& pd) {

    double maxint = 0.5 * std::numeric_limits<size_t>::max();

    double sum = 0.;
    double min = 0.;
    double max = 0.;

    for (vector<double>::const_iterator it = pd.begin(); it != pd.end(); ++it) {

        // Skip inacceptable values
        if (*it < 0.) {
            continue;
        }

        // Track min/max
        if (*it > max) {
            max = *it;
        }
        if ((*it > 0.) && (min == 0. || *it < min)) {
            min = *it;
        }

        // Track sum
        sum += *it;
    }

    // Check scaling 
    double d1 = log(sum) - log(min);
    double d2 = log(maxint) - d1;

    if ((d1 > (62. * log(2.))) || (d2 < (3. * log(2.)))) {
        // >>> Cannot map >>>
        std::cout << "d1 = " << d1 / log(2.) << std::endl;
        std::cout << "d2 = " << d2 / log(2.) << std::endl;
		throw std::runtime_error("DecisionReader::readUIntCustom(...): "
				"Unhandlable scales");
    }

    // The integer values mapped from double
    vector<size_t> pdint(pd.size());

    // The scale factor
    double f = maxint / sum;

    // Map to integer
    size_t index = 0;
    for (vector<double>::const_iterator it = pd.begin(); it != pd.end(); ++it) {
        pdint[index++] = f * (*it);
    }

    // Now apply readUIntCustom(...) based on mapped integer values
    return this->readUIntCustom(pdint);
}


/*
 * ___________________________________________________________________________
 */
DecisionReader::~DecisionReader() {
}

