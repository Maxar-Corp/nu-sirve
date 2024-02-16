#include <QCheckBox>
#include <QPushButton>

class QPushButtonWithId : public QPushButton
{
    Q_OBJECT

public:
    QPushButtonWithId(int id, const QString& text, QWidget *parent = nullptr);

signals:
    void clicked_with_id(int id);

private slots:
    void handle_button_click();

private:
    int id;
};

class QCheckBoxWithId : public QCheckBox
{
    Q_OBJECT

public:
    QCheckBoxWithId(int id, const QString& text, QWidget *parent = nullptr);

signals:
    void checked_with_id(int id);
    void unchecked_with_id(int id);

private slots:
    void handle_state_changed(int state);

private:
    int id;
};