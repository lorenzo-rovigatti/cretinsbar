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

class QAudioFormat;

namespace cb {

class Engine;

class MainWindow: public QMainWindow {
Q_OBJECT

public:
	MainWindow(Engine *engine, QWidget *parent = 0);
	virtual ~MainWindow();

public slots:
	void format_changed(const QAudioFormat &new_format);

private:
	Engine *_engine;
	Ui::MainWindow *_ui;
};

} /* namespace cb */

#endif /* SRC_GUI_MAINWINDOW_H_ */
