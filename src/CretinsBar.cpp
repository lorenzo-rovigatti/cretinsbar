/*
 * CretinsBar.cpp
 *
 *  Created on: 04 feb 2017
 *      Author: lorenzo
 */

#include "CretinsBar.h"
#include "Engine.h"
#include "GUI/MainWindow.h"

namespace cb {

CretinsBar::CretinsBar(int &argc, char **argv)
	: QApplication(argc, argv)
	, _window(new MainWindow())
	, _engine(new Engine(this)) {
	setOrganizationName("CretinsBar");
	setApplicationName("CretinsBar");
	setApplicationVersion("alpha");

	_window->show();
	_engine->start_playback();
}

CretinsBar::~CretinsBar() {

}

} /* namespace cb */
