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
#include "video_class.h"


class Annotations : public QDialog
{
	Q_OBJECT

public:
	Annotations(std::vector<annotation_info> &input_vector, video_info details, Video *input_video, QWidget *parent = nullptr);
	~Annotations();

	void show_annotation(int index);
	void repopulate_list();

private:

	std::vector<annotation_info> &data;
	video_info base_data;

	QLabel *lbl_annotations, *lbl_description;
	QListWidget *lst_annotations;
	QPushButton *btn_ok, *btn_edit, *btn_new, *btn_delete;
	Video *current_video;

	void initialize_gui();
	void ok();
	void add();
	void edit();
	void delete_object();

};


#endif