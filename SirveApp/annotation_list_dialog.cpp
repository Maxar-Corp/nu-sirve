#include "annotation_list_dialog.h"
#include "annotation_edit_dialog.h"

AnnotationListDialog::AnnotationListDialog(std::vector<AnnotationInfo> &input_vector, VideoInfo details, QWidget *parent)
    : QDialog(parent, Qt::Window), data(input_vector), base_data(details)
{
    initialize_gui();

    base_data = details;
    repopulate_list();

    resize(400, 300);

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
    AnnotationEditDialog annotation_edit_dialog(data.back());
    connect(&annotation_edit_dialog, &AnnotationEditDialog::annotationChanged, this, &AnnotationListDialog::annotationListUpdated);

    auto response = annotation_edit_dialog.exec();

    // if action was cancelled or window closed, then remove the new annotation
    if (response == 0) {
        data.pop_back();
        emit annotationListUpdated();

        return;
    }

    repopulate_list();
    lst_annotations->setCurrentRow(data.size() - 1);


    emit updateAnnotationStencilText(lst_annotations->currentItem()->text());
    emit showAnnotationStencil();
}

void AnnotationListDialog::edit()
{
    // set user definable attributes

    int index = lst_annotations->currentRow();

    if (index >= 0) {
        // store old data in case user cancels operation
        AnnotationInfo old_data = data[index];

        // display new annotation screen
        AnnotationEditDialog annotation_edit_dialog(data[index]);
        connect(&annotation_edit_dialog, &AnnotationEditDialog::annotationChanged, this, &AnnotationListDialog::annotationListUpdated);

        auto response = annotation_edit_dialog.exec();

        // if action was cancelled or window closed, then restore previous annotation
        if (response == 0)
        {
            data[index] = old_data;

            emit annotationListUpdated();
        }

        repopulate_list();

        if (data.size() > 0) {
            //show_annotation(index);
            lst_annotations->setCurrentRow(index);
        }
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
        }
    }

}

void AnnotationListDialog::UpdateStencilPosition(QPoint location)
{
    QString output;

    int index = lst_annotations->currentRow();

    if (index >= 0) {

        AnnotationInfo d = data[index];

        output = "Annotation: " + d.text + "\n\n";
        output += "X Pixel: " + QString::number(location.x()) + "\t Y Pixel: " + QString::number(location.y()) + " \n\n";
        output += "Font Size: " + QString::number(12) + "\t Color: " + "red" + "\n\n";
        output += "Frame Start: " + QString::number(1) + "\t \n\nNum Frames: " + QString::number(624) + " \n";

        lbl_description->setText(output);
    }
}
