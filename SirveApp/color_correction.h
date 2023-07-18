#pragma once

#ifndef COLOR_CORRECTION_H
#define COLOR_CORRECTION_H

#include <iostream>
#include <string>

#include <armadillo>

#include <QWidget>

class Min_Max_Value : public QWidget
{
	Q_OBJECT
public:

	Min_Max_Value();
	~Min_Max_Value();

	double get_min();
	double get_max();
	void get_updated_color(arma::vec& input);

	void set_min(double value);
	void set_max(double value);

private:
	double min_value, max_value;

};



#endif // COLOR_CORRECTION_H