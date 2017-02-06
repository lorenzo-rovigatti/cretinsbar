/*
 * MainWindow.cpp
 *
 *  Created on: 04 feb 2017
 *      Author: lorenzo
 */

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "../SoundUtils/SoundUtils.h"
#include "../Utilities/WavAnalyser.h"

namespace cb {

MainWindow::MainWindow(QWidget *parent):
	QMainWindow(parent),
	_ui(new Ui::MainWindow) {
	_ui->setupUi(this);
}

MainWindow::~MainWindow() {

}

} /* namespace cb */
