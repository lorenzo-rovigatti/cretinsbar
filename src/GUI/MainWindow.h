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
class QCustomPlot;
class QCPItemStraightLine;

namespace cb {

class Engine;

class MainWindow: public QMainWindow {
Q_OBJECT

public:
	MainWindow(Engine *engine, QWidget *parent = 0);
	virtual ~MainWindow();

public slots:
	void play_position_changed(qint64 position);
	void on_mouse_move(QMouseEvent *event);
	void erase_statusbar(QMouseEvent *event);

private slots:
	void _open();
	void _toggle_play(bool s);
	void _stop();
	void _seek(QMouseEvent *event);

	void _engine_playing();
	void _engine_paused();
	void _engine_stopped();

private:
	const QString _pos_layer;
	Engine *_engine;
	Ui::MainWindow *_ui;

	QCustomPlot *_plot;
	QCPItemStraightLine *_position;
	void _init_plot();
};

} /* namespace cb */

#endif /* SRC_GUI_MAINWINDOW_H_ */
