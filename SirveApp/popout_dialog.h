#pragma once

#ifndef POPOUT_DIALOG_H
#define POPOUT_DIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QWidget>

class PopoutDialog : public QDialog
{
	Q_OBJECT

public:
	PopoutDialog(QWidget *widget);
	~PopoutDialog();

    QGridLayout * mainLayout;
};


#endif