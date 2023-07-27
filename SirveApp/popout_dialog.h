#pragma once

#ifndef POPOUT_DIALOG_H
#define POPOUT_DIALOG_H

#include <QDialog>
#include <QGridLayout>

#include "enhanced_label.h"
#include "clickable_chartview.h"
#include "plot_engineering_data.h"

class PopoutDialog : public QDialog
{
	Q_OBJECT

public:
	PopoutDialog(EnhancedLabel* label);
	PopoutDialog(Clickable_QChartView* chart_view);
	PopoutDialog(NewChartView* new_chart_view);
	~PopoutDialog();

    QGridLayout * mainLayout;
};


#endif