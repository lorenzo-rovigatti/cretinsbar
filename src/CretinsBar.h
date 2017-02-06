/*
 * CretinsBar.h
 *
 *  Created on: 04 feb 2017
 *      Author: lorenzo
 */

#ifndef SRC_CRETINSBAR_H_
#define SRC_CRETINSBAR_H_

#include <QApplication>

namespace cb {

class MainWindow;
class Engine;

class CretinsBar: public QApplication {
public:
	CretinsBar(int &argc, char **argv);
	virtual ~CretinsBar();

private:
	MainWindow *_window;
	Engine *_engine;
};

} /* namespace cb */

#endif /* SRC_CRETINSBAR_H_ */
