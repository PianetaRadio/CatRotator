/**
 ** This file is part of the CatRotator project.
 ** Copyright 2022 Gianfranco Sordetti IZ8EWD <iz8ewd@pianetaradio.it>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/


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

    void on_checkBox_PreviSat_toggled(bool checked);

    void on_checkBox_WSJTX_toggled(bool checked);

    void on_checkBox_AirScout_toggled(bool checked);

    void on_checkBox_PreviSat_2_toggled(bool checked);

    void on_checkBox_WSJTX_2_toggled(bool checked);

    void on_checkBox_AirScout_2_toggled(bool checked);

    void on_checkBox_PreviSat_3_toggled(bool checked);

    void on_checkBox_WSJTX_3_toggled(bool checked);

    void on_checkBox_AirScout_3_toggled(bool checked);

private:
    Ui::DialogRotator *ui;
};

int printRotatorList(const struct rot_caps *rotCaps, void *data);
void msgActivateUdp();

#endif // DIALOGROTATOR_H
