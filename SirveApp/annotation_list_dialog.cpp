#include "annotation_list_dialog.h"
#include "annotation_edit_dialog.h"

AnnotationListDialog::AnnotationListDialog(std::vector<AnnotationInfo> &input_vector, VideoInfo details, QWidget *parent)
    : QDialog(parent, Qt::Window), data(input_vector), base_data(details)
{
    initialize_gui();

    base_data = details;
    repopulate_list();

    connect(btn_ok, &QPushButton::pressed, this, &AnnotationListDialog::ok);
    connect(btn_new, &QPushButton::pressed, this, &AnnotationListDialog::add);
    connect(btn_edit, &QPushButton::pressed, this, &AnnotationListDialog::edit);
    connect(btn_delete, &QPushButton::pressed, this, &AnnotationListDialog::delete_object);

    connect(lst_annotations, &QListWidget::currentRowChanged, this, &AnnotationListDialog::ShowAnnotation);
}

AnnotationListDialog::~AnnotationListDialog() {

    delete lbl_annotations;
    delete lbl_description;

    delete btn_ok;
    delete btn_edit;
    delete btn_new;
    delete btn_delete;

    delete lst_annotations;
}

void AnnotationListDialog::ShowAnnotation(int index)
{

    QString output;

    if (index >= 0) {
        AnnotationInfo d = data[index];

        output = "Annotation: " + d.text + "\n\n";
        output += "X Pixel: " + QString::number(d.x_pixel) + "\t Y Pixel: " + QString::number(d.y_pixel) + " \n\n";
        output += "Font Size: " + QString::number(d.font_size) + "\t Color: " + d.color + "\n\n";
        output += "Frame Start: " + QString::number(d.frame_start) + "\t \n\nNum Frames: " + QString::number(d.num_frames) + " \n";
    }
    else {
        output = "";
    }

    lbl_description->setText(output);
}

void AnnotationListDialog::repopulate_list()
{
    lst_annotations->clear();

    for (int i = 0; i < data.size(); i++)
    {
        lst_annotations->addItem(data[i].text);
    }
}

void AnnotationListDialog::initialize_gui()
{

    // define objects
    lbl_annotations = new QLabel(tr("AnnotationListDialog"));
    lst_annotations = new QListWidget();
    lbl_description = new QLabel(tr(""));

    btn_ok = new QPushButton(tr("OK"));
    btn_edit = new QPushButton(tr("Edit"));
    btn_new = new QPushButton(tr("New"));
    btn_delete = new QPushButton(tr("Delete"));

    // set gridlayout
    QGridLayout *mainLayout = new QGridLayout;
    //mainLayout->addWidget(lbl_annotations, 0, 0, 1, 4, Qt::AlignLeft);
    mainLayout->addWidget(lst_annotations, 0, 0, 4, 1);
    mainLayout->addWidget(lbl_description, 0, 1, 4, 1);
    mainLayout->addWidget(btn_ok, 0, 2);
    mainLayout->addWidget(btn_new, 1, 2);
    mainLayout->addWidget(btn_edit, 2, 2);
    mainLayout->addWidget(btn_delete, 3, 2);

    // set grid characterestics
    //mainLayout->setRowStretch(0, 0);
    mainLayout->setRowStretch(0, 100);
    mainLayout->setRowStretch(1, 100);
    mainLayout->setRowStretch(2, 100);
    mainLayout->setRowStretch(3, 100);

    mainLayout->setColumnMinimumWidth(0, 100);
    mainLayout->setColumnMinimumWidth(1, 150);

    mainLayout->setColumnStretch(0, 100);
    mainLayout->setColumnStretch(1, 100);
    mainLayout->setColumnStretch(2, 0);

    setLayout(mainLayout);
    setWindowTitle(tr("AnnotationListDialog"));
}

void AnnotationListDialog::ok()
{
    done(1);
    this->deleteLater();

    emit hideAnnotationStencil();
}

void AnnotationListDialog::add()
{
    // set user definable attributes
    AnnotationInfo new_data;
    new_data.color = "red";
    new_data.font_size = 8;
    new_data.x_pixel = 50;
    new_data.y_pixel = 50;
    new_data.frame_start = base_data.min_frame;
    new_data.num_frames = base_data.max_frame - base_data.min_frame + 1;
    new_data.text = "Add Text";

    // set attributes for checking data
    new_data.min_frame = base_data.min_frame;
    new_data.max_frame = base_data.max_frame;
    new_data.x_min_position = 1;
    new_data.x_max_position = base_data.x_pixels;
    new_data.y_min_position = 1;
    new_data.y_max_position = base_data.y_pixels;

    data.push_back(new_data);

    // display new annotation screen
    annotation_edit_dialog = new AnnotationEditDialog(data.back());
    connect(annotation_edit_dialog, &AnnotationEditDialog::annotationChanged, this, &AnnotationListDialog::annotationListUpdated);


    connect(this, &QDialog::rejected, this, &AnnotationListDialog::onDialogRejected);

    auto response = annotation_edit_dialog->exec();

    // if action was cancelled or window closed, then remove the new annotation
    if (response == 0) {
        data.pop_back();
        emit annotationListUpdated();

        return;
    }

    repopulate_list();
    lst_annotations->setCurrentRow(data.size() - 1);

    emit updateAnnotationStencil(data[data.size()-1]);
    emit showAnnotationStencil();

    btn_ok->setDisabled(true);
}

void AnnotationListDialog::edit()
{
    // set user definable attributes

    int index = lst_annotations->currentRow();

    if (index >= 0) {
        // store old data in case user cancels operation
        old_data = data[index];

        annotation_edit_dialog = new AnnotationEditDialog(data[index]);
        annotation_edit_dialog->setWindowTitle("Edit Annotation");
        connect(annotation_edit_dialog, &AnnotationEditDialog::annotationChanged, this, &AnnotationListDialog::annotationListUpdated);

        auto response = annotation_edit_dialog->exec();

        // if action was cancelled or window closed, then restore previous annotation
        if (response == 0)
        {
            data[index] = old_data;

            emit annotationListUpdated();
            emit hideAnnotationStencil();
        }

        repopulate_list();

        if (data.size() > 0) {
            lst_annotations->setCurrentRow(index);
        }

        emit updateAnnotationStencil(data[index]);
        emit showAnnotationStencil();
    }
}

void AnnotationListDialog::delete_object()
{
    int index = lst_annotations->currentRow();

    if (index >= 0)
    {
        auto response = QtHelpers::LaunchYesNoMessageBox("Delete Annotation", "Are you sure you want to delete this annotation?");

        // if yes, delete annotation
        if (response == QMessageBox::Yes) {
            data.erase(data.begin() + index);

            repopulate_list();
            lst_annotations->setCurrentRow(-1);
            emit annotationListUpdated();
            emit hideAnnotationStencil();
        }
    }
}

void AnnotationListDialog::UpdateStencilPosition(QPoint position)
{
    QString output;

    int index = lst_annotations->currentRow();

    if (index >= 0) {

        AnnotationInfo d = data[index];

        output = "Annotation: " + d.text + "\n\n";
        output += "X Pixel: " + QString::number(position.x()) + "\t Y Pixel: " + QString::number(position.y()) + " \n\n";
        output += "Font Size: " + QString::number(d.font_size) + "\t Color: " + d.color + "\n\n";
        output += "Frame Start: " + QString::number(d.frame_start) + "\t \n\nNum Frames: " + QString::number(d.num_frames) + " \n";

        lbl_description->setText(output);

        emit positionChanged(position);

        btn_ok->setEnabled(true);
    }
}

void AnnotationListDialog::SetStencilLocation(QPoint location)
{
    auto response = QtHelpers::LaunchYesNoMessageBox("Locate Annotation", "Locate your new annotation here?", true);

    int index = lst_annotations->currentRow();

    // if yes, set stencil location
    if (response == QMessageBox::Yes) {

        //int index = lst_annotations->currentRow();

        data[index].x_pixel = location.x();
        data[index].y_pixel = location.y();

        emit hideAnnotationStencil();
        emit annotationListUpdated();
    } else if (response == QMessageBox::Cancel)
    {
        // If adding an annotation, clean out newly-entered data:
        if (this->windowTitle().contains("Add"))
        {
            data.pop_back();
            lst_annotations->takeItem(lst_annotations->count()-1);
        } else
        {
            data[index] = old_data;
        }
        emit hideAnnotationStencil();
    }
}

void AnnotationListDialog::onDialogRejected() {
    emit hideAnnotationStencil();
}
