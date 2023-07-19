#include "color_correction.h"

Min_Max_Value::Min_Max_Value()
{
}

Min_Max_Value::~Min_Max_Value()
{
}

void Min_Max_Value::get_updated_color(arma::vec & input, double min_value, double max_value)
{
	// Normalize the values
	double range = max_value - min_value;
	input = input - min_value;
	input = input / range;

	// Replace anything below the minimum value and above the maximum value
	arma::uvec below_min_value = arma::find(input < 0);
	arma::uvec above_max_value = arma::find(input > 1);

	if (below_min_value.n_elem > 0) {
		input(below_min_value) = arma::zeros(below_min_value.n_elem);
	}

	if (above_max_value.n_elem > 0)
	{
		input(above_max_value) = arma::ones(above_max_value.n_elem);
	}
}