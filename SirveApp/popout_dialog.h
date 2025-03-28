#pragma once

#ifndef POPOUT_DIALOG_H
#define POPOUT_DIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QPointer>
#include <QWidget>


class PopoutDialog : public QDialog
{
	Q_OBJECT

public:
	PopoutDialog();

    QPointer<QGridLayout> mainLayout;

	void acquire(QWidget *widget);
};


#endif