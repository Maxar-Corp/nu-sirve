#pragma once

#ifndef COLOR_CORRECTION_H
#define COLOR_CORRECTION_H

#include <armadillo>

namespace ColorCorrection
{
	void update_color(arma::vec& input, double min_value, double max_value);
};

#endif // COLOR_CORRECTION_H