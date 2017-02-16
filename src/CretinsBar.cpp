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

CretinsBar::CretinsBar(int &argc, char **argv) :
		QApplication(argc, argv), _engine(new Engine(this)), _window(new MainWindow(_engine)) {
	setOrganizationName("CretinsBar");
	setApplicationName("CretinsBar");
	setApplicationVersion("alpha");

	_window->show();
}

CretinsBar::~CretinsBar() {

}

} /* namespace cb */
