#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>

class QComboBoxWithId : public QComboBox
{
    Q_OBJECT

public:
    QComboBoxWithId(int id, QWidget *parent = nullptr);

signals:
    void currentIndexChangedWithId(int id, int index);

private slots:
    void HandleIndexChanged(int index);

private:
    int id;
};

class QPushButtonWithId : public QPushButton
{
    Q_OBJECT

public:
    QPushButtonWithId(int id, const QString& text, QWidget *parent = nullptr);

signals:
    void clickedWithId(int id);

private slots:
    void HandleButtonClick();

private:
    int id;
};

class QCheckBoxWithId : public QCheckBox
{
    Q_OBJECT

public:
    QCheckBoxWithId(int id, const QString& text, QWidget *parent = nullptr);

signals:
    void checkedWithId(int id);
    void uncheckedWithId(int id);

private slots:
    void HandleStateChanged(int state);

private:
    int id;
};
