#ifndef ANNOTATIONLISTDIALOG_H
#define ANNOTATIONLISTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

#include "annotation_info.h"
#include "video_display.h"
#include "annotation_edit_dialog.h"
#include "abir_reader.h"

class AnnotationListDialog : public QDialog
{
    Q_OBJECT

public:
    AnnotationListDialog(std::vector<AnnotationInfo> &input_vector, VideoInfo details, ABPFileType file_type, QWidget *parent = nullptr);
    ~AnnotationListDialog();

    AnnotationEditDialog *annotation_edit_dialog;

    void ShowAnnotation(int index);
    void repopulate_list();

    QPointer<QPushButton> btn_ok;

protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    void annotationListUpdated();
    void hideAnnotationStencil();
    void positionChanged(QPoint location);
    void showAnnotationStencil();
    void updateAnnotationStencil(AnnotationInfo data);


public slots:

    void OnDialogRejected();
    void UpdateStencilPosition(QPoint position);
    void SetStencilLocation(QPoint location);

private:
    int nRows = 480;
    int nCols = 640;
    std::vector<AnnotationInfo> &data;
    VideoInfo base_data;
    AnnotationInfo old_data;

    QLabel *lbl_annotations, *lbl_description;
    QListWidget *lst_annotations;
    QPushButton *btn_edit, *btn_new, *btn_delete;
    // VideoDisplay *current_video;

    QString *myText;

    void initialize_gui();
    void ok();
    void add();
    void edit();
    void delete_object();
};

#endif // ANNOTATIONLISTDIALOG_H
