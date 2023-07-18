#pragma once

#ifndef COLOR_CORRECTION_LGG_H
#define COLOR_CORRECTION_LGG_H

#include <iostream>
#include <string>

#include <armadillo>

#include <QWidget>

class Lift_Gamma_Gain : public QWidget
{
	Q_OBJECT
public:

	Lift_Gamma_Gain(double input_lift = 0, double input_gamma = 1, double input_gain = 1);
	~Lift_Gamma_Gain();

	double get_min_lift();
	double get_lift();
	double get_gamma();
	double get_gain();
	double get_updated_color(int original_value, int max_value);
	void get_updated_color(arma::vec& input, int max_value, double &frame_min, double &frame_max);

	bool set_lift(double value);
	bool set_gamma(double value);
	bool set_gain(double value);

	void get_lift_slider_range(int &min_value, int &max_value);
	void get_gamma_slider_range(int &min_value, int &max_value);
	void get_gain_slider_range(int &min_value, int &max_value);

	double lift_convert_slider_to_value(int value);
	double gamma_convert_slider_to_value(int value);
	double gain_convert_slider_to_value(int value);

signals:
	void update_lift_gamma_gain(double lift, double gamma, double gain);

private:
	double lift, gamma, gain;
	double max_lift, min_lift, max_gamma, min_gamma, max_gain, min_gain;

};


class Min_Max_Value : public QWidget
{
	Q_OBJECT
public:

	Min_Max_Value();
	~Min_Max_Value();

	double get_min();
	double get_max();
	void get_updated_color(arma::vec& input);

	bool set_min(double value);
	bool set_max(double value);

	void get_min_slider_range(int &min_value, int &max_value);
	void get_max_slider_range(int &min_value, int &max_value);
	int get_ui_slider_value(double input);

	double min_convert_slider_to_value(int value);
	double max_convert_slider_to_value(int value);

signals:
	void update_min_max(double min, double max);


private:
	double min_value, max_value;

};



#endif // COLOR_CORRECTION_LGG_H