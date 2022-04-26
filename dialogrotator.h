#ifndef DIALOGROTATOR_H
#define DIALOGROTATOR_H

#include <QDialog>

namespace Ui {
class DialogRotator;
}

class DialogRotator : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRotator(QWidget *parent = nullptr);
    ~DialogRotator();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::DialogRotator *ui;
};

int printRotatorList(const struct rot_caps *rotCaps, void *data);

#endif // DIALOGROTATOR_H
