/*
 * MainWindow.h
 *
 *  Created on: 04 feb 2017
 *      Author: lorenzo
 */

#ifndef SRC_GUI_MAINWINDOW_H_
#define SRC_GUI_MAINWINDOW_H_

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

namespace cb {

class MainWindow: public QMainWindow {
Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	virtual ~MainWindow();

	void play();

private:
	Ui::MainWindow *_ui;
};

} /* namespace cb */

#endif /* SRC_GUI_MAINWINDOW_H_ */
