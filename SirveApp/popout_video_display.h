#pragma once

#ifndef POPOUT_VIDEO_DISPLAY_H
#define POPOUT_VIDEO_DISPLAY_H

#include <QDialog>
#include <QGridLayout>

#include "enhanced_label.h"

class PopoutVideoDisplay : public QDialog
{
	Q_OBJECT

public:
	PopoutVideoDisplay(EnhancedLabel* label);
	~PopoutVideoDisplay();

    QGridLayout * mainLayout;
};


#endif