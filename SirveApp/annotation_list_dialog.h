#pragma once

#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

#include <qlabel.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qlistwidget.h>
#include <qgridlayout.h>

#include "annotation_edit_dialog.h"
#include "annotation_info.h"
#include "video_display.h"
#include "support/qthelpers.h"


class AnnotationListDialog : public QDialog
{
	Q_OBJECT

public:
	AnnotationListDialog(std::vector<AnnotationInfo> &input_vector, VideoInfo details, QWidget *parent = nullptr);
	~AnnotationListDialog();

	void ShowAnnotation(int index);
	void repopulate_list();

signals:
	void annotation_list_updated();

private:

	std::vector<AnnotationInfo> &data;
	VideoInfo base_data;

	QLabel *lbl_annotations, *lbl_description;
	QListWidget *lst_annotations;
	QPushButton *btn_ok, *btn_edit, *btn_new, *btn_delete;
	VideoDisplay *current_video;

	void initialize_gui();
	void ok();
	void add();
	void edit();
	void delete_object();

};


#endif