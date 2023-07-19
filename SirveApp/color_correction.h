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
	void get_updated_color(arma::vec& input, double min_value, double max_value);
};

#endif // COLOR_CORRECTION_H