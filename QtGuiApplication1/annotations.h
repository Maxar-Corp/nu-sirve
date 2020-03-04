#pragma once

#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

#include "logging.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qlistwidget.h>
#include <qgridlayout.h>

#include "annotation_new.h"
#include "Data_Structures.h"


class Annotations : public QDialog
{
	Q_OBJECT

public:
	Annotations(QWidget *parent = nullptr);
	~Annotations();

private:

	std::vector<annotation_info> data;

	QLabel *lbl_annotations, *lbl_description;
	QListWidget *lst_annotations;
	QPushButton *btn_ok, *btn_edit, *btn_new, *btn_delete;

	void initialize_gui();
	void ok();
	void add();
	void edit();
	void delete_object();

};


#endif