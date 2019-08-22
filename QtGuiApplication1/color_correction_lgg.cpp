#include "color_correction_lgg.h"

Lift_Gamma_Gain::Lift_Gamma_Gain(double input_lift, double input_gamma, double input_gain)
{
	lift = input_lift;
	gamma = input_gamma;
	gain = input_gain;

	max_lift = 1.0;
	min_lift = -1.0;
	
	max_gamma = 5.0;
	min_gamma = .001;
	
	max_gain = 2.0;
	min_gain = 0.0;
}

Lift_Gamma_Gain::~Lift_Gamma_Gain()
{
}

double Lift_Gamma_Gain::get_min_lift()
{
	return min_lift;
}

double Lift_Gamma_Gain::get_lift()
{
	return lift;
}

double Lift_Gamma_Gain::get_gamma()
{
	return gamma;
}

double Lift_Gamma_Gain::get_gain()
{
	return gain;
}

double Lift_Gamma_Gain::get_updated_color(int original_value, int max_value)
{
	double normalized_input, exponent_base, updated_value;
	max_value = max_value - 1;

	normalized_input = (double)original_value / max_value;
	exponent_base = normalized_input * gain - normalized_input * lift + lift;
	
	// Guarantees root of negative not being taken
	if (exponent_base < 0) {
		exponent_base = 0;
	}
	
	updated_value = std::pow(exponent_base, 1.0 / gamma) * max_value; 

	// Check limits
	if (updated_value >= max_value)
	{
		updated_value = max_value;
	}
	
	if (updated_value < 0)
	{
		updated_value = 0;
	}

	return updated_value;
}

arma::mat Lift_Gamma_Gain::get_updated_color(arma::mat input, int max_value)
{
	arma::mat normalized_input, exponent_base, updated_value;
	
	int rows = input.n_rows;
	int cols = input.n_cols;
	arma::mat ones(rows, cols, arma::fill::ones);
	
	normalized_input = input / max_value;

	exponent_base = normalized_input * gain - normalized_input * lift + ones * lift;
	
	// Guarantees root of negative not being taken
	arma::uvec index = arma::find(exponent_base < 0);
	if (index.n_elem > 0)
		exponent_base.elem(index) = arma::zeros(index.n_elem);

	updated_value = arma::pow(exponent_base, 1.0 / gamma) * max_value;

	// Check limits
	index = arma::find(updated_value > max_value);
	if (index.n_elem > 0)
		updated_value.elem(index) = arma::ones(index.n_elem) * max_value;

	index = arma::find(updated_value < 0);
	if (index.n_elem > 0)
		updated_value.elem(index) = arma::zeros(index.n_elem);
	
	return updated_value;
}

bool Lift_Gamma_Gain::set_lift(double value)
{
	if (value >= min_lift & value <= max_lift)
	{
		if (value != lift) {
			lift = value;
			emit update_lift_gamma_gain(lift, gamma, gain);
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool Lift_Gamma_Gain::set_gamma(double value)
{
	if (value >= min_gamma & value <= max_gamma)
	{
		if (value != gamma) {
			gamma = value;
			emit update_lift_gamma_gain(lift, gamma, gain);
		}
		return true;
	}
	else
		{
			return false;
		}
}

bool Lift_Gamma_Gain::set_gain(double value)
{
	if (value >= min_gain & value <= max_gain)
	{
		if (value != gain) {
			gain = value;
			emit update_lift_gamma_gain(lift, gamma, gain);
		}
		return true;
	}
	else
	{
		return false;
	}
}

// ------------------------------------------------------------------------------------------

void Lift_Gamma_Gain::get_lift_slider_range(int & min_value, int & max_value)
{
	int value_range = (max_lift - min_lift) * 100;

	min_value = 0;
	max_value = value_range;
}

void Lift_Gamma_Gain::get_gamma_slider_range(int & min_value, int & max_value)
{
	// Assumes gamma is above zero

	min_value = min_gamma * 1000;
	max_value = max_gamma * 1000;

}

void Lift_Gamma_Gain::get_gain_slider_range(int & min_value, int & max_value)
{
	// Assumes min_gain is zero or positive

	min_value = min_gain * 100;
	max_value = max_gain * 100;
}

double Lift_Gamma_Gain::lift_convert_slider_to_value(int value)
{	
	return (min_lift * 100. + value) / 100.;
}

double Lift_Gamma_Gain::gamma_convert_slider_to_value(int value)
{
	return value / 1000.;
}

double Lift_Gamma_Gain::gain_convert_slider_to_value(int value)
{
	return value / 100.;
}


