/*
 * WaveForm.h
 *
 *  Created on: 02 apr 2017
 *      Author: lorenzo
 */

#ifndef SRC_GUI_WAVEFORM_H_
#define SRC_GUI_WAVEFORM_H_

#include "qcustomplot/qcustomplot.h"

namespace cb {

using pair_qreal = QPair<qreal, qreal>;
class Engine;

class WaveForm: public QCustomPlot {
	Q_OBJECT;

public:
	WaveForm(QWidget *parent = 0);
	virtual ~WaveForm();

	void init(QScrollBar *scrollbar);
	pair_qreal selection_boundaries();
	void load_wave(Engine *engine);

public slots:
	void update_play_position(qint64 position);

signals:
	void status_update(QString);

private slots:
	void _x_axis_changed(const QCPRange &range);
	void _plot_scrollbar_changed(int value);
	void _on_mouse_press(QMouseEvent *event);
	void _on_mouse_move(QMouseEvent *event);
	void _on_mouse_release(QMouseEvent *event);

private:
	void leaveEvent(QEvent *event);

	QScrollBar *_scrollbar;

	const QString _pos_layer;
	QCPItemStraightLine *_position;
	QCPItemStraightLine *_beginning_position;
	QCPItemRect *_selection;

	QPoint _press_pos;
	pair_qreal _sel_boundaries;

	qreal MOVE_SEL_THRESHOLD;
	qreal SET_SEL_THRESHOLD;
	enum sel_moving_type {
		NO_MOVING,
		MOVE_LEFT,
		MOVE_RIGHT
	};
	int _sel_moving_type;
};

} /* namespace cb */

#endif /* SRC_GUI_WAVEFORM_H_ */
