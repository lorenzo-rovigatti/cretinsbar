/*
 * WaveForm.cpp
 *
 *  Created on: 02 apr 2017
 *      Author: lorenzo
 */

#include "WaveForm.h"

namespace cb {

WaveForm::WaveForm(QWidget *parent) :
		QCustomPlot(parent), _scrollbar(nullptr), _pos_layer("position_layer"), _position(nullptr), _selection(nullptr) {

}

WaveForm::~WaveForm() {

}

void WaveForm::init(QScrollBar *scrollbar) {
	_scrollbar = scrollbar;

	setMaximumHeight(300);
	setInteractions(QCP::iRangeZoom);
	axisRect()->setRangeDrag(Qt::Horizontal);
	axisRect()->setRangeZoom(Qt::Horizontal);

	xAxis->setTickLength(0, 0);
	xAxis->setSubTicks(false);
	xAxis->setTickLabels(true);
	xAxis->setBasePen(Qt::NoPen);

	yAxis->setTickLabels(false);
	yAxis->setTicks(false);
	yAxis->grid()->setVisible(false);
	yAxis->setBasePen(Qt::NoPen);

	addLayer(_pos_layer, 0, QCustomPlot::limAbove);
	layer(_pos_layer)->setMode(QCPLayer::lmBuffered);

	_selection = new QCPItemRect(this);
	_selection->setBrush(QBrush(QColor(100, 100, 100, 100)));
	_selection->setPen(Qt::NoPen);
	_selection->setLayer(_pos_layer);
	_selection->setVisible(false);

	_position = new QCPItemStraightLine(this);
	_position->point1->setCoords(0, -1);
	_position->point2->setCoords(0, 1);
	QPen line_pen(QColor("red"));
	line_pen.setWidth(3);
	_position->setPen(line_pen);
	_position->setLayer(_pos_layer);

	// setup the scrollbar
	connect(_scrollbar, &QScrollBar::valueChanged, this, &WaveForm::_plot_scrollbar_changed);
	connect(xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(_x_axis_changed(QCPRange)));

	connect(this, &QCustomPlot::mouseMove, this, &WaveForm::_on_mouse_move);
	connect(this, &QCustomPlot::mousePress, this, &WaveForm::_on_mouse_press);
	connect(this, &QCustomPlot::mouseRelease, this, &WaveForm::_on_mouse_release);
}

pair_qreal WaveForm::selection_boundaries() {
	return _sel_boundaries;
}

void WaveForm::update_play_position(qint64 position) {
	qreal pos_in_sec = position / (qreal) 1000000.;
	_position->point1->setCoords(pos_in_sec, -1);
	_position->point2->setCoords(pos_in_sec, 1);
	_position->setVisible(true);
	layer(_pos_layer)->replot();
}

void WaveForm::_x_axis_changed(const QCPRange &range) {
	bool range_valid;
	qreal duration = graph()->data()->keyRange(range_valid).upper;
	if(range_valid) {
		// make sure that we do not zoom out too much
		if(range.lower < 0. || range.upper > duration) {
			QCPRange new_range = range.bounded(0, duration);
			xAxis->setRange(new_range);
		}
		else {
			// adjust the position of the scroll bar slider
			_scrollbar->setRange(0, qRound(duration - range.size()));
			_scrollbar->setValue(qRound(range.center()));
			// adjust the size of the scroll bar slider
			_scrollbar->setPageStep(qRound(range.size()));
		}
	}
}

void WaveForm::_plot_scrollbar_changed(int value) {
	// we don't want to replot twice if the user is dragging plot
	if(qAbs(xAxis->range().center() - value / 100.0) > 0.01) {
		xAxis->setRange(value, xAxis->range().size(), Qt::AlignCenter);
		replot();
	}
}

void WaveForm::_on_mouse_press(QMouseEvent *event) {
	_press_pos = event->pos();

	_position->setVisible(false);
	_selection->setVisible(false);
	layer(_pos_layer)->replot();
}

void WaveForm::_on_mouse_move(QMouseEvent *event) {
	QString msg;
	qreal x = xAxis->pixelToCoord(event->pos().x());
	if(isEnabled()) {
		// transform the mouse position to x,y coordinates and show them in the status bar
		msg = QString("%1 s").arg(x, 0, 'f', 2);
	}
	emit status_update(msg);

	int pixel_diff = event->pos().x() - _press_pos.x();
	bool left_pressed = event->buttons() & Qt::LeftButton;
	if(left_pressed && fabs(pixel_diff) > 10) {
		qreal left_pos, right_pos;
		if(pixel_diff > 0) {
			left_pos = xAxis->pixelToCoord(_press_pos.x());
			right_pos = x;
		}
		else {
			left_pos = x;
			right_pos = xAxis->pixelToCoord(_press_pos.x());
		}
		_selection->topLeft->setCoords(left_pos, yAxis->range().upper);
		_selection->bottomRight->setCoords(right_pos, yAxis->range().lower);
		_selection->setVisible(true);
		layer(_pos_layer)->replot();
	}
}

void WaveForm::_on_mouse_release(QMouseEvent *event) {
	if(_selection->visible()) {
		_sel_boundaries.first = _selection->topLeft->coords().x();
		_sel_boundaries.second = _selection->bottomRight->coords().x();
	}
	else {
		_sel_boundaries.first = xAxis->pixelToCoord(event->pos().x());
		_sel_boundaries.second = -1;
	}
}

void WaveForm::leaveEvent(QEvent *event) {
	emit status_update(QString(""));
}

} /* namespace cb */
