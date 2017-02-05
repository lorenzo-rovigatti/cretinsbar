/*
 * CretinsBar.cpp
 *
 *  Created on: 04 feb 2017
 *      Author: lorenzo
 */

#include "CretinsBar.h"
#include "GUI/MainWindow.h"

namespace cb {

CretinsBar::CretinsBar(int &argc, char **argv): QApplication(argc, argv) {
	setOrganizationName("CretinsBar");
	setApplicationName("CretinsBar");
	setApplicationVersion("alpha");

	_window = new cb::MainWindow();
	_window->show();
	_window->play();
}

CretinsBar::~CretinsBar() {

}

} /* namespace cb */
