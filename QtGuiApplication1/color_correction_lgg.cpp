#include "color_correction_lgg.h"

Lift_Gamma_Gain::Lift_Gamma_Gain(double input_lift, double input_gamma, double input_gain)
{
	lift = input_lift;
	gamma = input_gamma;
	gain = input_gain;

	enhanced_dynamic_range = false;

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

void Lift_Gamma_Gain::get_updated_color(arma::vec & input, int max_value, double &frame_min, double &frame_max)
{
	//arma::vec updated_value;

	int rows = input.n_rows;
	int cols = input.n_cols;
	arma::mat ones(rows, cols, arma::fill::ones);

	input = input / max_value;
	input = input * gain - input * lift + ones * lift;

	double max_max = input.max();

	// Guarantees root of negative not being taken
	arma::uvec index = arma::find(input < 0);
	if (index.n_elem > 0)
		input.elem(index) = arma::zeros(index.n_elem);

	input = arma::pow(input, 1.0 / gamma);

	double min_frame_value, max_frame_value, min_frame_value_update, max_frame_value_update;
	min_frame_value = 0;
	max_frame_value = 1;

	if (enhanced_dynamic_range) {

		max_frame_value_update = (std::pow(1.0, gamma) - lift) / (gain - lift);
		min_frame_value_update = (std::pow(0.00000001, gamma) - lift) / (gain - lift);
		if (min_frame_value_update > 0)
			min_frame_value = min_frame_value_update;
		if (max_frame_value_update < 1)
			max_frame_value = max_frame_value_update;

		frame_min = min_frame_value;
		frame_max = max_frame_value;
	}
	else {
		frame_min = 0;
		frame_max = 1;
	}

	// Check limits
	index = arma::find(input > max_frame_value);
	if (index.n_elem > 0) {
		double check = input(index(0));
		input.elem(index) = arma::ones(index.n_elem) * max_frame_value;
	}
	index = arma::find(input < min_frame_value);
	if (index.n_elem > 0)
		input.elem(index) = arma::ones(index.n_elem) * min_frame_value;

	input = (input - min_frame_value) / (max_frame_value - min_frame_value);

	//return updated_value;
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

void Lift_Gamma_Gain::toggle_enhanced_range(bool enhanced_range)
{
	enhanced_dynamic_range = enhanced_range;
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

// ------------------------------------------------------------------------------------

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

bool Min_Max_Value::set_min(double value)
{
	if (value >= 0 & value <= max_value)
	{
		if (value != min_value) {
			min_value = value;
			emit update_min_max(min_value, max_value);
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool Min_Max_Value::set_max(double value)
{
	if (value >= min_value & value <= 1.0)
	{
		if (value != max_value) {
			max_value = value;
			emit update_min_max(min_value, max_value);
		}
		return true;
	}
	else
	{
		return false;
	}
}

void Min_Max_Value::get_min_slider_range(int & min_value, int & max_value)
{
	min_value = 0;
	max_value = 1000;
}

void Min_Max_Value::get_max_slider_range(int & min_value, int & max_value)
{
	min_value = 0;
	max_value = 1000;
}

int Min_Max_Value::get_ui_slider_value(double input)
{

	return input * 1000;
}

// ------------------------------------------------------------------------------------------

double Min_Max_Value::min_convert_slider_to_value(int value)
{
	return value / 1000.;
}


double Min_Max_Value::max_convert_slider_to_value(int value)
{
	return value / 1000.;
}




