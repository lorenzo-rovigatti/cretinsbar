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
class QCPRange;
class QCPItemStraightLine;
class QCPItemRect;

namespace cb {

class Engine;
class WaveForm;

class MainWindow: public QMainWindow {
Q_OBJECT

public:
	MainWindow(Engine *engine, QWidget *parent = 0);
	virtual ~MainWindow();

	void load_in_engine(QString filename);

public slots:

private slots:
	void _on_open();
	void _toggle_play(bool s);
	void _stop();

	void _plot_on_mouse_release(QMouseEvent *event);

	void _engine_playing();
	void _engine_paused();
	void _engine_stopped();
	void _engine_at_end();

	void _on_slider_change();

private:
	const QString _pos_layer;
	Engine *_engine;
	Ui::MainWindow *_ui;

	QPoint _press_pos;
	WaveForm *_plot;
	void _init_plot();
	void _set_all_enabled(bool state);
};

} /* namespace cb */

#endif /* SRC_GUI_MAINWINDOW_H_ */
