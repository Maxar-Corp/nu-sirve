#include <QtTest>

class TestExample: public QObject {
    Q_OBJECT
public:
    TestExample();
private slots:
    void testOne();
};