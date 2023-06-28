#pragma once

#ifndef POPOUT_DIALOG_H
#define POPOUT_DIALOG_H

#include <QDialog>
#include <QGridLayout>

#include "enhanced_label.h"

class PopoutDialog : public QDialog
{
	Q_OBJECT

public:
	PopoutDialog(EnhancedLabel* label);
	~PopoutDialog();

    QGridLayout * mainLayout;
};


#endif