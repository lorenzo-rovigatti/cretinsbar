/*
 * WaveForm.cpp
 *
 *  Created on: 02 apr 2017
 *      Author: lorenzo
 */

#include "WaveForm.h"

#include "../Engine.h"

namespace cb {

WaveForm::WaveForm(QWidget *parent) :
		QCustomPlot(parent), _scrollbar(nullptr), _pos_layer("position_layer"), _position(nullptr), _selection(nullptr) {
	MOVE_SEL_THRESHOLD = 10;
	SET_SEL_THRESHOLD = 10;
	_sel_moving_type = sel_moving_type::NO_MOVING;
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

void WaveForm::load_wave(Engine *engine) {
	const QByteArray *buffer = engine->data();
	qint64 length = buffer->length();
	clearGraphs();
	int bytes = engine->sample_size() / 8;
	long n_samples = length / bytes;
	int n_channels = engine->channel_count();
	qreal length_in_seconds = engine->duration();
	long increment = n_channels;
	long max_val = 2 << (engine->sample_size() - 2);
	long min_val = (engine->sample_type() == QAudioFormat::UnSignedInt) ? 0 : -max_val;
	long max_interval = max_val - min_val;

	const short *samples = reinterpret_cast<const short *>(buffer->data());

	// add to the plot a graph for each channel
	long n_samples_per_channel = n_samples / n_channels;
	QVector<qreal> x_data(n_samples_per_channel);
	QVector<qreal> y_data(n_samples_per_channel);
	for(int channel = 0; channel < n_channels; channel++) {
		int idx = 0;
		for(int i = channel; i < n_samples; i += increment, idx++) {
			x_data[idx] = idx / (qreal) engine->sample_rate();
			// shift each plot up
			y_data[idx] = (qreal) (samples[i] + channel*max_interval);
		}

		QCPGraph *graph = addGraph();
		graph->setPen(QPen(QColor("black")));
		graph->setData(x_data, y_data, true);
	}

	_scrollbar->setRange(0, length_in_seconds);
	xAxis->setRange(0, length_in_seconds);
	yAxis->setRange(min_val, min_val + max_interval*n_channels);

	replot();
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

	if(_sel_moving_type == sel_moving_type::NO_MOVING) {
		_position->setVisible(false);
		_selection->setVisible(false);
	}
	layer(_pos_layer)->replot();
}

void WaveForm::_on_mouse_move(QMouseEvent *event) {
	QString msg;
	int x_pixel = event->pos().x();
	qreal x_coord = xAxis->pixelToCoord(x_pixel);
	if(isEnabled()) {
		// transform the mouse position to x,y coordinates and show them in the status bar
		msg = QString("%1 s").arg(x_coord, 0, 'f', 2);
	}
	emit status_update(msg);

	int pixel_diff = event->pos().x() - _press_pos.x();
	bool left_pressed = event->buttons() & Qt::LeftButton;
	if(left_pressed) {
		// if the cursor has moved enough
		if(fabs(pixel_diff) > SET_SEL_THRESHOLD) {
			qreal left_pos, right_pos;

			switch(_sel_moving_type) {
			case sel_moving_type::MOVE_RIGHT:
				left_pos = _selection->topLeft->coords().x();
				right_pos = x_coord;
				break;
			case sel_moving_type::MOVE_LEFT:
				left_pos = x_coord;
				right_pos = _selection->bottomRight->coords().x();
				break;
			default:
			case sel_moving_type::NO_MOVING:
				if(pixel_diff > 0) {
					left_pos = xAxis->pixelToCoord(_press_pos.x());
					right_pos = x_coord;
				}
				else {
					left_pos = x_coord;
					right_pos = xAxis->pixelToCoord(_press_pos.x());
				}
				break;
			}

			// if the user dragged one of the two edges over the other we invert their
			// positions and also the type of selection movement
			if(left_pos > right_pos) {
				qreal tmp = left_pos;
				left_pos = right_pos;
				right_pos = tmp;
				if(_sel_moving_type == sel_moving_type::MOVE_RIGHT) _sel_moving_type = sel_moving_type::MOVE_LEFT;
				else if(_sel_moving_type == sel_moving_type::MOVE_LEFT) _sel_moving_type = sel_moving_type::MOVE_RIGHT;
			}
			// check boundaries
			if(left_pos < 0) left_pos = 0;
			if(right_pos > xAxis->range().upper) right_pos = xAxis->range().upper;

			_selection->topLeft->setCoords(left_pos, yAxis->range().upper);
			_selection->bottomRight->setCoords(right_pos, yAxis->range().lower);
			_selection->setVisible(true);
			layer(_pos_layer)->replot();
		}
	}
	// if no buttons were pressed we check whether the mouse is close to one of the edges
	// of the selection rectangle
	else {
		qreal left_x_pos = xAxis->coordToPixel(_selection->topLeft->coords().x());
		qreal right_x_pos = xAxis->coordToPixel(_selection->bottomRight->coords().x());
		// if the cursor is close to either edge, we change the cursor shape and remember
		// which edge is close to
		if(fabs(x_pixel - left_x_pos) < MOVE_SEL_THRESHOLD) {
			this->setCursor(QCursor(Qt::CursorShape::SplitHCursor));
			_sel_moving_type = sel_moving_type::MOVE_LEFT;
		}
		else if(fabs(x_pixel - right_x_pos) < MOVE_SEL_THRESHOLD) {
			this->setCursor(QCursor(Qt::CursorShape::SplitHCursor));
			_sel_moving_type = sel_moving_type::MOVE_RIGHT;
		}
		else {
			this->setCursor(Qt::ArrowCursor);
			_sel_moving_type = sel_moving_type::NO_MOVING;
		}
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
