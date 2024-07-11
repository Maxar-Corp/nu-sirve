#ifndef ANNOTATIONLISTDIALOG_H
#define ANNOTATIONLISTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include "annotation_info.h"
#include "video_display.h"

class AnnotationListDialog : public QDialog
{
    Q_OBJECT

public:
    AnnotationListDialog(std::vector<AnnotationInfo> &input_vector, VideoInfo details, QWidget *parent = nullptr);
    ~AnnotationListDialog();

    void ShowAnnotation(int index);
    void repopulate_list();

signals:
    void annotationListUpdated();
    void showAnnotationStencil();
    void hideAnnotationStencil();
    void updateAnnotationStencil(AnnotationInfo data);
    void locationChanged(QPoint location);

public slots:

    void UpdateStencilPosition(QPoint location);

private:
    std::vector<AnnotationInfo> &data;
    VideoInfo base_data;

    QLabel *lbl_annotations, *lbl_description;
    QListWidget *lst_annotations;
    QPushButton *btn_ok, *btn_edit, *btn_new, *btn_delete;
    VideoDisplay *current_video;

    QString *myText;

    void initialize_gui();
    void ok();
    void add();
    void edit();
    void delete_object();
};

#endif // ANNOTATIONLISTDIALOG_H
