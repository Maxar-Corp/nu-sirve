/********************************************************************************
** Form generated from reading UI file 'QtGuiApplication1.ui'
**
** Created by: Qt User Interface Compiler version 5.12.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTGUIAPPLICATION1_H
#define UI_QTGUIAPPLICATION1_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtGuiApplication1Class
{
public:
    QAction *actionLoad_OSM;
    QWidget *centralWidget;
    QTabWidget *tabMenu;
    QWidget *tab;
    QFrame *line;
    QLabel *lbl_directory_path;
    QLabel *lbl_file_name;
    QWidget *gridLayoutWidget_2;
    QGridLayout *gridLayout;
    QLineEdit *txt_start_frame;
    QLabel *label_4;
    QLineEdit *txt_end_frame;
    QPushButton *btn_get_frames;
    QLabel *label_3;
    QLabel *lbl_max_frames;
    QFrame *line_3;
    QPushButton *btn_load_osm;
    QLabel *lbl_file_load;
    QFrame *line_4;
    QCheckBox *btn_use_epoch;
    QLineEdit *txt_epoch;
    QLabel *label_5;
    QLabel *label_6;
    QWidget *tab_2;
    QWidget *gridLayoutWidget;
    QGridLayout *grdLayout_Color_Control;
    QLabel *lbl_lift;
    QLabel *lbl_gamma;
    QLabel *lbl_gain;
    QSlider *slider_lift;
    QSlider *slider_gamma;
    QSlider *slider_gain;
    QLabel *lbl_lift_value;
    QLabel *lbl_gamma_value;
    QLabel *lbl_gain_value;
    QFrame *line_2;
    QFrame *frmColorCorrection;
    QLabel *lbl_color_correction;
    QPushButton *btn_reset_color_correction;
    QWidget *tab_5;
    QWidget *tab_6;
    QFrame *frmVideoPlayer;
    QSlider *sldrVideo;
    QPushButton *btn_play;
    QPushButton *btn_slow_back;
    QPushButton *btn_fast_forward;
    QFrame *frm_video;
    QLabel *lbl_fps;
    QPushButton *btn_prev_frame;
    QPushButton *btn_next_frame;
    QPushButton *btn_video_menu;
    QPushButton *btn_pause;
    QPushButton *btn_reverse;
    QPushButton *btn_frame_save;
    QPushButton *btn_frame_record;
    QTabWidget *tabPlots;
    QWidget *tab_3;
    QFrame *frm_histogram;
    QWidget *tab_4;
    QFrame *frm_plots;
    QComboBox *cmb_plot_yaxis;
    QCheckBox *chk_plot_full_data;
    QComboBox *cmb_plot_xaxis;
    QCheckBox *chk_plot_primary_data;
    QLabel *label;
    QLabel *label_2;
    QCheckBox *chk_plot_show_line;
    QPushButton *btn_save_plot;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuHelp;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *QtGuiApplication1Class)
    {
        if (QtGuiApplication1Class->objectName().isEmpty())
            QtGuiApplication1Class->setObjectName(QString::fromUtf8("QtGuiApplication1Class"));
        QtGuiApplication1Class->resize(1723, 847);
        actionLoad_OSM = new QAction(QtGuiApplication1Class);
        actionLoad_OSM->setObjectName(QString::fromUtf8("actionLoad_OSM"));
        centralWidget = new QWidget(QtGuiApplication1Class);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        tabMenu = new QTabWidget(centralWidget);
        tabMenu->setObjectName(QString::fromUtf8("tabMenu"));
        tabMenu->setGeometry(QRect(20, 30, 270, 746));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        line = new QFrame(tab);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(10, 120, 231, 20));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        lbl_directory_path = new QLabel(tab);
        lbl_directory_path->setObjectName(QString::fromUtf8("lbl_directory_path"));
        lbl_directory_path->setGeometry(QRect(10, 30, 231, 31));
        lbl_directory_path->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        lbl_directory_path->setWordWrap(true);
        lbl_file_name = new QLabel(tab);
        lbl_file_name->setObjectName(QString::fromUtf8("lbl_file_name"));
        lbl_file_name->setGeometry(QRect(10, 70, 231, 31));
        lbl_file_name->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        lbl_file_name->setWordWrap(true);
        gridLayoutWidget_2 = new QWidget(tab);
        gridLayoutWidget_2->setObjectName(QString::fromUtf8("gridLayoutWidget_2"));
        gridLayoutWidget_2->setGeometry(QRect(40, 150, 160, 91));
        gridLayout = new QGridLayout(gridLayoutWidget_2);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        txt_start_frame = new QLineEdit(gridLayoutWidget_2);
        txt_start_frame->setObjectName(QString::fromUtf8("txt_start_frame"));
        txt_start_frame->setEnabled(false);
        txt_start_frame->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(txt_start_frame, 1, 0, 1, 1);

        label_4 = new QLabel(gridLayoutWidget_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_4, 0, 1, 1, 1);

        txt_end_frame = new QLineEdit(gridLayoutWidget_2);
        txt_end_frame->setObjectName(QString::fromUtf8("txt_end_frame"));
        txt_end_frame->setEnabled(false);
        txt_end_frame->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(txt_end_frame, 1, 1, 1, 1);

        btn_get_frames = new QPushButton(gridLayoutWidget_2);
        btn_get_frames->setObjectName(QString::fromUtf8("btn_get_frames"));
        btn_get_frames->setEnabled(false);

        gridLayout->addWidget(btn_get_frames, 3, 0, 1, 2);

        label_3 = new QLabel(gridLayoutWidget_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_3, 0, 0, 1, 1);

        lbl_max_frames = new QLabel(gridLayoutWidget_2);
        lbl_max_frames->setObjectName(QString::fromUtf8("lbl_max_frames"));

        gridLayout->addWidget(lbl_max_frames, 2, 0, 1, 2);

        line_3 = new QFrame(tab);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setGeometry(QRect(10, 260, 231, 20));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);
        btn_load_osm = new QPushButton(tab);
        btn_load_osm->setObjectName(QString::fromUtf8("btn_load_osm"));
        btn_load_osm->setGeometry(QRect(160, 100, 81, 23));
        lbl_file_load = new QLabel(tab);
        lbl_file_load->setObjectName(QString::fromUtf8("lbl_file_load"));
        lbl_file_load->setGeometry(QRect(10, 400, 231, 141));
        lbl_file_load->setFrameShape(QFrame::Box);
        lbl_file_load->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        lbl_file_load->setWordWrap(true);
        line_4 = new QFrame(tab);
        line_4->setObjectName(QString::fromUtf8("line_4"));
        line_4->setGeometry(QRect(10, 370, 231, 20));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);
        btn_use_epoch = new QCheckBox(tab);
        btn_use_epoch->setObjectName(QString::fromUtf8("btn_use_epoch"));
        btn_use_epoch->setGeometry(QRect(20, 280, 131, 17));
        txt_epoch = new QLineEdit(tab);
        txt_epoch->setObjectName(QString::fromUtf8("txt_epoch"));
        txt_epoch->setEnabled(false);
        txt_epoch->setGeometry(QRect(20, 330, 211, 20));
        label_5 = new QLabel(tab);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setEnabled(false);
        label_5->setGeometry(QRect(20, 350, 211, 16));
        label_6 = new QLabel(tab);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setEnabled(false);
        label_6->setGeometry(QRect(20, 310, 47, 13));
        tabMenu->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        gridLayoutWidget = new QWidget(tab_2);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(10, 19, 231, 95));
        grdLayout_Color_Control = new QGridLayout(gridLayoutWidget);
        grdLayout_Color_Control->setSpacing(6);
        grdLayout_Color_Control->setContentsMargins(11, 11, 11, 11);
        grdLayout_Color_Control->setObjectName(QString::fromUtf8("grdLayout_Color_Control"));
        grdLayout_Color_Control->setContentsMargins(0, 0, 0, 0);
        lbl_lift = new QLabel(gridLayoutWidget);
        lbl_lift->setObjectName(QString::fromUtf8("lbl_lift"));
        lbl_lift->setSizeIncrement(QSize(0, 0));
        lbl_lift->setAlignment(Qt::AlignCenter);

        grdLayout_Color_Control->addWidget(lbl_lift, 0, 0, 1, 1);

        lbl_gamma = new QLabel(gridLayoutWidget);
        lbl_gamma->setObjectName(QString::fromUtf8("lbl_gamma"));
        lbl_gamma->setAlignment(Qt::AlignCenter);

        grdLayout_Color_Control->addWidget(lbl_gamma, 1, 0, 1, 1);

        lbl_gain = new QLabel(gridLayoutWidget);
        lbl_gain->setObjectName(QString::fromUtf8("lbl_gain"));
        lbl_gain->setAlignment(Qt::AlignCenter);

        grdLayout_Color_Control->addWidget(lbl_gain, 2, 0, 1, 1);

        slider_lift = new QSlider(gridLayoutWidget);
        slider_lift->setObjectName(QString::fromUtf8("slider_lift"));
        slider_lift->setEnabled(true);
        slider_lift->setMaximum(100);
        slider_lift->setValue(50);
        slider_lift->setOrientation(Qt::Horizontal);
        slider_lift->setTickPosition(QSlider::TicksAbove);
        slider_lift->setTickInterval(10);

        grdLayout_Color_Control->addWidget(slider_lift, 0, 1, 1, 1);

        slider_gamma = new QSlider(gridLayoutWidget);
        slider_gamma->setObjectName(QString::fromUtf8("slider_gamma"));
        slider_gamma->setEnabled(true);
        slider_gamma->setMinimum(10);
        slider_gamma->setMaximum(500);
        slider_gamma->setSingleStep(5);
        slider_gamma->setValue(100);
        slider_gamma->setOrientation(Qt::Horizontal);
        slider_gamma->setTickPosition(QSlider::TicksAbove);
        slider_gamma->setTickInterval(50);

        grdLayout_Color_Control->addWidget(slider_gamma, 1, 1, 1, 1);

        slider_gain = new QSlider(gridLayoutWidget);
        slider_gain->setObjectName(QString::fromUtf8("slider_gain"));
        slider_gain->setEnabled(true);
        slider_gain->setMinimum(50);
        slider_gain->setMaximum(150);
        slider_gain->setValue(100);
        slider_gain->setOrientation(Qt::Horizontal);
        slider_gain->setTickPosition(QSlider::TicksAbove);
        slider_gain->setTickInterval(10);

        grdLayout_Color_Control->addWidget(slider_gain, 2, 1, 1, 1);

        lbl_lift_value = new QLabel(gridLayoutWidget);
        lbl_lift_value->setObjectName(QString::fromUtf8("lbl_lift_value"));
        lbl_lift_value->setAlignment(Qt::AlignCenter);

        grdLayout_Color_Control->addWidget(lbl_lift_value, 0, 2, 1, 1);

        lbl_gamma_value = new QLabel(gridLayoutWidget);
        lbl_gamma_value->setObjectName(QString::fromUtf8("lbl_gamma_value"));
        lbl_gamma_value->setAlignment(Qt::AlignCenter);

        grdLayout_Color_Control->addWidget(lbl_gamma_value, 1, 2, 1, 1);

        lbl_gain_value = new QLabel(gridLayoutWidget);
        lbl_gain_value->setObjectName(QString::fromUtf8("lbl_gain_value"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lbl_gain_value->sizePolicy().hasHeightForWidth());
        lbl_gain_value->setSizePolicy(sizePolicy);
        lbl_gain_value->setAlignment(Qt::AlignCenter);

        grdLayout_Color_Control->addWidget(lbl_gain_value, 2, 2, 1, 1);

        line_2 = new QFrame(tab_2);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setGeometry(QRect(10, 160, 231, 20));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);
        frmColorCorrection = new QFrame(tab_2);
        frmColorCorrection->setObjectName(QString::fromUtf8("frmColorCorrection"));
        frmColorCorrection->setGeometry(QRect(5, 200, 250, 250));
        frmColorCorrection->setFrameShape(QFrame::Box);
        frmColorCorrection->setFrameShadow(QFrame::Raised);
        lbl_color_correction = new QLabel(tab_2);
        lbl_color_correction->setObjectName(QString::fromUtf8("lbl_color_correction"));
        lbl_color_correction->setGeometry(QRect(0, 460, 261, 20));
        lbl_color_correction->setAlignment(Qt::AlignCenter);
        btn_reset_color_correction = new QPushButton(tab_2);
        btn_reset_color_correction->setObjectName(QString::fromUtf8("btn_reset_color_correction"));
        btn_reset_color_correction->setGeometry(QRect(170, 120, 71, 21));
        tabMenu->addTab(tab_2, QString());
        tab_5 = new QWidget();
        tab_5->setObjectName(QString::fromUtf8("tab_5"));
        tabMenu->addTab(tab_5, QString());
        tab_6 = new QWidget();
        tab_6->setObjectName(QString::fromUtf8("tab_6"));
        tabMenu->addTab(tab_6, QString());
        frmVideoPlayer = new QFrame(centralWidget);
        frmVideoPlayer->setObjectName(QString::fromUtf8("frmVideoPlayer"));
        frmVideoPlayer->setGeometry(QRect(309, 50, 680, 600));
        frmVideoPlayer->setFrameShape(QFrame::Box);
        frmVideoPlayer->setFrameShadow(QFrame::Raised);
        sldrVideo = new QSlider(frmVideoPlayer);
        sldrVideo->setObjectName(QString::fromUtf8("sldrVideo"));
        sldrVideo->setGeometry(QRect(10, 530, 651, 22));
        sldrVideo->setSliderPosition(0);
        sldrVideo->setOrientation(Qt::Horizontal);
        sldrVideo->setTickPosition(QSlider::NoTicks);
        btn_play = new QPushButton(frmVideoPlayer);
        btn_play->setObjectName(QString::fromUtf8("btn_play"));
        btn_play->setGeometry(QRect(350, 560, 35, 30));
        btn_slow_back = new QPushButton(frmVideoPlayer);
        btn_slow_back->setObjectName(QString::fromUtf8("btn_slow_back"));
        btn_slow_back->setGeometry(QRect(590, 560, 35, 30));
        btn_fast_forward = new QPushButton(frmVideoPlayer);
        btn_fast_forward->setObjectName(QString::fromUtf8("btn_fast_forward"));
        btn_fast_forward->setGeometry(QRect(550, 560, 35, 30));
        frm_video = new QFrame(frmVideoPlayer);
        frm_video->setObjectName(QString::fromUtf8("frm_video"));
        frm_video->setGeometry(QRect(10, 10, 660, 500));
        frm_video->setFrameShape(QFrame::StyledPanel);
        frm_video->setFrameShadow(QFrame::Raised);
        lbl_fps = new QLabel(frmVideoPlayer);
        lbl_fps->setObjectName(QString::fromUtf8("lbl_fps"));
        lbl_fps->setGeometry(QRect(490, 570, 47, 13));
        btn_prev_frame = new QPushButton(frmVideoPlayer);
        btn_prev_frame->setObjectName(QString::fromUtf8("btn_prev_frame"));
        btn_prev_frame->setGeometry(QRect(230, 560, 35, 30));
        btn_next_frame = new QPushButton(frmVideoPlayer);
        btn_next_frame->setObjectName(QString::fromUtf8("btn_next_frame"));
        btn_next_frame->setGeometry(QRect(390, 560, 35, 30));
        btn_video_menu = new QPushButton(frmVideoPlayer);
        btn_video_menu->setObjectName(QString::fromUtf8("btn_video_menu"));
        btn_video_menu->setGeometry(QRect(630, 560, 41, 30));
        btn_pause = new QPushButton(frmVideoPlayer);
        btn_pause->setObjectName(QString::fromUtf8("btn_pause"));
        btn_pause->setGeometry(QRect(310, 560, 35, 30));
        btn_reverse = new QPushButton(frmVideoPlayer);
        btn_reverse->setObjectName(QString::fromUtf8("btn_reverse"));
        btn_reverse->setGeometry(QRect(270, 560, 35, 30));
        btn_frame_save = new QPushButton(frmVideoPlayer);
        btn_frame_save->setObjectName(QString::fromUtf8("btn_frame_save"));
        btn_frame_save->setEnabled(false);
        btn_frame_save->setGeometry(QRect(10, 560, 35, 30));
        btn_frame_record = new QPushButton(frmVideoPlayer);
        btn_frame_record->setObjectName(QString::fromUtf8("btn_frame_record"));
        btn_frame_record->setEnabled(false);
        btn_frame_record->setGeometry(QRect(50, 560, 35, 30));
        tabPlots = new QTabWidget(centralWidget);
        tabPlots->setObjectName(QString::fromUtf8("tabPlots"));
        tabPlots->setGeometry(QRect(1000, 50, 684, 620));
        tabPlots->setTabPosition(QTabWidget::South);
        tabPlots->setTabShape(QTabWidget::Rounded);
        tabPlots->setTabsClosable(false);
        tabPlots->setMovable(true);
        tabPlots->setTabBarAutoHide(false);
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        frm_histogram = new QFrame(tab_3);
        frm_histogram->setObjectName(QString::fromUtf8("frm_histogram"));
        frm_histogram->setGeometry(QRect(10, 10, 660, 500));
        frm_histogram->setFrameShape(QFrame::StyledPanel);
        frm_histogram->setFrameShadow(QFrame::Raised);
        tabPlots->addTab(tab_3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        frm_plots = new QFrame(tab_4);
        frm_plots->setObjectName(QString::fromUtf8("frm_plots"));
        frm_plots->setGeometry(QRect(10, 10, 660, 500));
        frm_plots->setFrameShape(QFrame::StyledPanel);
        frm_plots->setFrameShadow(QFrame::Raised);
        cmb_plot_yaxis = new QComboBox(tab_4);
        cmb_plot_yaxis->setObjectName(QString::fromUtf8("cmb_plot_yaxis"));
        cmb_plot_yaxis->setEnabled(false);
        cmb_plot_yaxis->setGeometry(QRect(160, 540, 101, 22));
        chk_plot_full_data = new QCheckBox(tab_4);
        chk_plot_full_data->setObjectName(QString::fromUtf8("chk_plot_full_data"));
        chk_plot_full_data->setEnabled(false);
        chk_plot_full_data->setGeometry(QRect(520, 520, 111, 21));
        chk_plot_full_data->setChecked(true);
        cmb_plot_xaxis = new QComboBox(tab_4);
        cmb_plot_xaxis->setObjectName(QString::fromUtf8("cmb_plot_xaxis"));
        cmb_plot_xaxis->setEnabled(false);
        cmb_plot_xaxis->setGeometry(QRect(10, 540, 131, 22));
        chk_plot_primary_data = new QCheckBox(tab_4);
        chk_plot_primary_data->setObjectName(QString::fromUtf8("chk_plot_primary_data"));
        chk_plot_primary_data->setEnabled(false);
        chk_plot_primary_data->setGeometry(QRect(520, 540, 131, 21));
        chk_plot_primary_data->setChecked(false);
        label = new QLabel(tab_4);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 520, 131, 20));
        label->setAlignment(Qt::AlignCenter);
        label_2 = new QLabel(tab_4);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(160, 520, 101, 20));
        label_2->setAlignment(Qt::AlignCenter);
        chk_plot_show_line = new QCheckBox(tab_4);
        chk_plot_show_line->setObjectName(QString::fromUtf8("chk_plot_show_line"));
        chk_plot_show_line->setEnabled(false);
        chk_plot_show_line->setGeometry(QRect(520, 560, 151, 21));
        chk_plot_show_line->setChecked(false);
        btn_save_plot = new QPushButton(tab_4);
        btn_save_plot->setObjectName(QString::fromUtf8("btn_save_plot"));
        btn_save_plot->setEnabled(false);
        btn_save_plot->setGeometry(QRect(460, 520, 41, 31));
        tabPlots->addTab(tab_4, QString());
        QtGuiApplication1Class->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(QtGuiApplication1Class);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1723, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        QtGuiApplication1Class->setMenuBar(menuBar);
        mainToolBar = new QToolBar(QtGuiApplication1Class);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        QtGuiApplication1Class->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(QtGuiApplication1Class);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        QtGuiApplication1Class->setStatusBar(statusBar);
        QWidget::setTabOrder(tabMenu, tabPlots);
        QWidget::setTabOrder(tabPlots, btn_slow_back);
        QWidget::setTabOrder(btn_slow_back, btn_play);
        QWidget::setTabOrder(btn_play, btn_fast_forward);
        QWidget::setTabOrder(btn_fast_forward, sldrVideo);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionLoad_OSM);

        retranslateUi(QtGuiApplication1Class);

        tabMenu->setCurrentIndex(0);
        tabPlots->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(QtGuiApplication1Class);
    } // setupUi

    void retranslateUi(QMainWindow *QtGuiApplication1Class)
    {
        QtGuiApplication1Class->setWindowTitle(QApplication::translate("QtGuiApplication1Class", "QtGuiApplication1", nullptr));
        actionLoad_OSM->setText(QApplication::translate("QtGuiApplication1Class", "Load OSM", nullptr));
        lbl_directory_path->setText(QApplication::translate("QtGuiApplication1Class", "Directory Path:", nullptr));
        lbl_file_name->setText(QApplication::translate("QtGuiApplication1Class", "File Name:", nullptr));
        txt_start_frame->setText(QApplication::translate("QtGuiApplication1Class", "0", nullptr));
        label_4->setText(QApplication::translate("QtGuiApplication1Class", "End Frame", nullptr));
        btn_get_frames->setText(QApplication::translate("QtGuiApplication1Class", "Load Frames", nullptr));
        label_3->setText(QApplication::translate("QtGuiApplication1Class", "Start Frame", nullptr));
        lbl_max_frames->setText(QApplication::translate("QtGuiApplication1Class", "Max Frames: ", nullptr));
        btn_load_osm->setText(QApplication::translate("QtGuiApplication1Class", "Load OSM File", nullptr));
        lbl_file_load->setText(QApplication::translate("QtGuiApplication1Class", "File Load Status:", nullptr));
        btn_use_epoch->setText(QApplication::translate("QtGuiApplication1Class", "Use epoch as t0", nullptr));
        label_5->setText(QApplication::translate("QtGuiApplication1Class", "Format is:    YYYY/MM/DD HH:MM:SS", nullptr));
        label_6->setText(QApplication::translate("QtGuiApplication1Class", "Epoch", nullptr));
        tabMenu->setTabText(tabMenu->indexOf(tab), QApplication::translate("QtGuiApplication1Class", "Import", nullptr));
        lbl_lift->setText(QApplication::translate("QtGuiApplication1Class", "Lift", nullptr));
        lbl_gamma->setText(QApplication::translate("QtGuiApplication1Class", "Gamma", nullptr));
        lbl_gain->setText(QApplication::translate("QtGuiApplication1Class", "Gain", nullptr));
        lbl_lift_value->setText(QApplication::translate("QtGuiApplication1Class", "0.0", nullptr));
        lbl_gamma_value->setText(QApplication::translate("QtGuiApplication1Class", "1.0", nullptr));
        lbl_gain_value->setText(QApplication::translate("QtGuiApplication1Class", "1.0", nullptr));
        lbl_color_correction->setText(QApplication::translate("QtGuiApplication1Class", "Output vs Input Value", nullptr));
        btn_reset_color_correction->setText(QApplication::translate("QtGuiApplication1Class", "Reset", nullptr));
        tabMenu->setTabText(tabMenu->indexOf(tab_2), QApplication::translate("QtGuiApplication1Class", "Color", nullptr));
        tabMenu->setTabText(tabMenu->indexOf(tab_5), QApplication::translate("QtGuiApplication1Class", "Processing", nullptr));
        tabMenu->setTabText(tabMenu->indexOf(tab_6), QApplication::translate("QtGuiApplication1Class", "Info", nullptr));
        btn_play->setText(QApplication::translate("QtGuiApplication1Class", "Pl", nullptr));
        btn_slow_back->setText(QApplication::translate("QtGuiApplication1Class", "SS", nullptr));
        btn_fast_forward->setText(QApplication::translate("QtGuiApplication1Class", "FF", nullptr));
        lbl_fps->setText(QString());
        btn_prev_frame->setText(QApplication::translate("QtGuiApplication1Class", "S", nullptr));
        btn_next_frame->setText(QApplication::translate("QtGuiApplication1Class", "F", nullptr));
        btn_video_menu->setText(QApplication::translate("QtGuiApplication1Class", "...", nullptr));
        btn_pause->setText(QApplication::translate("QtGuiApplication1Class", "Pa", nullptr));
        btn_reverse->setText(QApplication::translate("QtGuiApplication1Class", "Re", nullptr));
        btn_frame_save->setText(QApplication::translate("QtGuiApplication1Class", "Sa", nullptr));
        btn_frame_record->setText(QApplication::translate("QtGuiApplication1Class", "Rec", nullptr));
        tabPlots->setTabText(tabPlots->indexOf(tab_3), QApplication::translate("QtGuiApplication1Class", "Histogram", nullptr));
        chk_plot_full_data->setText(QApplication::translate("QtGuiApplication1Class", "Plot All Data", nullptr));
        chk_plot_primary_data->setText(QApplication::translate("QtGuiApplication1Class", "Plot Primary Data Only", nullptr));
        label->setText(QApplication::translate("QtGuiApplication1Class", "X - Axis", nullptr));
        label_2->setText(QApplication::translate("QtGuiApplication1Class", "Y - Axis", nullptr));
        chk_plot_show_line->setText(QApplication::translate("QtGuiApplication1Class", "Show Video Frame Location", nullptr));
        btn_save_plot->setText(QApplication::translate("QtGuiApplication1Class", "Sa", nullptr));
        tabPlots->setTabText(tabPlots->indexOf(tab_4), QApplication::translate("QtGuiApplication1Class", "Plots", nullptr));
        menuFile->setTitle(QApplication::translate("QtGuiApplication1Class", "File", nullptr));
        menuHelp->setTitle(QApplication::translate("QtGuiApplication1Class", "Help", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QtGuiApplication1Class: public Ui_QtGuiApplication1Class {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTGUIAPPLICATION1_H
