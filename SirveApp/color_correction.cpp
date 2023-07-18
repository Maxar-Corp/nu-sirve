#include "color_correction.h"

Min_Max_Value::Min_Max_Value()
{
	min_value = 0;
	max_value = 1;
}

Min_Max_Value::~Min_Max_Value()
{
}

double Min_Max_Value::get_min()
{
	return min_value;
}


double Min_Max_Value::get_max()
{
	return max_value;
}


void Min_Max_Value::get_updated_color(arma::vec & input)
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

void Min_Max_Value::set_min(double value)
{
	if (value >= 0 & value <= max_value)
	{
		min_value = value;
	}
}

void Min_Max_Value::set_max(double value)
{
	if (value >= min_value & value <= 1.0)
	{
		max_value = value;
	}
}