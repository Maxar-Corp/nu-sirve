/********************************************************************************
** Form generated from reading UI file 'new_location.ui'
**
** Created by: Qt User Interface Compiler version 5.12.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NEW_LOCATION_H
#define UI_NEW_LOCATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Dialog_Add_Location
{
public:
    QWidget *layoutWidget;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *btn_create;
    QPushButton *cancelButton;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLineEdit *txt_name;
    QLineEdit *txt_description;
    QLineEdit *txt_latitude;
    QLineEdit *txt_longitude;
    QLineEdit *txt_altitude;

    void setupUi(QDialog *Dialog_Add_Location)
    {
        if (Dialog_Add_Location->objectName().isEmpty())
            Dialog_Add_Location->setObjectName(QString::fromUtf8("Dialog_Add_Location"));
        Dialog_Add_Location->resize(395, 248);
        layoutWidget = new QWidget(Dialog_Add_Location);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 200, 351, 33));
        hboxLayout = new QHBoxLayout(layoutWidget);
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        btn_create = new QPushButton(layoutWidget);
        btn_create->setObjectName(QString::fromUtf8("btn_create"));

        hboxLayout->addWidget(btn_create);

        cancelButton = new QPushButton(layoutWidget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        hboxLayout->addWidget(cancelButton);

        label = new QLabel(Dialog_Add_Location);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 30, 71, 16));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_2 = new QLabel(Dialog_Add_Location);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 70, 71, 16));
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_3 = new QLabel(Dialog_Add_Location);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(80, 140, 81, 20));
        label_3->setAlignment(Qt::AlignCenter);
        label_4 = new QLabel(Dialog_Add_Location);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(180, 140, 91, 20));
        label_4->setAlignment(Qt::AlignCenter);
        label_5 = new QLabel(Dialog_Add_Location);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(300, 140, 61, 16));
        label_5->setAlignment(Qt::AlignCenter);
        txt_name = new QLineEdit(Dialog_Add_Location);
        txt_name->setObjectName(QString::fromUtf8("txt_name"));
        txt_name->setGeometry(QRect(90, 30, 113, 20));
        txt_description = new QLineEdit(Dialog_Add_Location);
        txt_description->setObjectName(QString::fromUtf8("txt_description"));
        txt_description->setGeometry(QRect(90, 70, 271, 51));
        txt_description->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        txt_latitude = new QLineEdit(Dialog_Add_Location);
        txt_latitude->setObjectName(QString::fromUtf8("txt_latitude"));
        txt_latitude->setGeometry(QRect(90, 160, 60, 20));
        txt_longitude = new QLineEdit(Dialog_Add_Location);
        txt_longitude->setObjectName(QString::fromUtf8("txt_longitude"));
        txt_longitude->setGeometry(QRect(195, 160, 60, 20));
        txt_altitude = new QLineEdit(Dialog_Add_Location);
        txt_altitude->setObjectName(QString::fromUtf8("txt_altitude"));
        txt_altitude->setGeometry(QRect(300, 160, 60, 20));

        retranslateUi(Dialog_Add_Location);
        QObject::connect(cancelButton, SIGNAL(clicked()), Dialog_Add_Location, SLOT(reject()));

        QMetaObject::connectSlotsByName(Dialog_Add_Location);
    } // setupUi

    void retranslateUi(QDialog *Dialog_Add_Location)
    {
        Dialog_Add_Location->setWindowTitle(QApplication::translate("Dialog_Add_Location", "Add New Location", nullptr));
        btn_create->setText(QApplication::translate("Dialog_Add_Location", "Create Sensor", nullptr));
        cancelButton->setText(QApplication::translate("Dialog_Add_Location", "Cancel", nullptr));
        label->setText(QApplication::translate("Dialog_Add_Location", "Name:", nullptr));
        label_2->setText(QApplication::translate("Dialog_Add_Location", "Description:", nullptr));
        label_3->setText(QApplication::translate("Dialog_Add_Location", "Latitude(deg)", nullptr));
        label_4->setText(QApplication::translate("Dialog_Add_Location", "Longitude(deg)", nullptr));
        label_5->setText(QApplication::translate("Dialog_Add_Location", "Altitude(m)", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Dialog_Add_Location: public Ui_Dialog_Add_Location {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NEW_LOCATION_H
