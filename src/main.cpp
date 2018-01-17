/*
 * main.cpp
 *
 *  Created on: 02 feb 2017
 *      Author: lorenzo
 */

#include <iostream>

#include "CretinsBar.h"

#include <QApplication>
#include <QDebug>
#include <QDateTime>
#include <QFileInfo>

/**
 * Logs the message msg, also adding the message type, timestamp and the filename, function and line which called the logger.
 *
 * @param type
 * @param context
 * @param msg
 */
void message_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
	static const char* typeStr[] = { "[   Debug]", "[ Warning]", "[Critical]", "[   Fatal]" };

	if(type <= QtFatalMsg) {
		QByteArray localMsg = msg.toLocal8Bit();
		QString filename = QFileInfo(context.file).fileName();

		QString contextString(QStringLiteral("(%1, %2, %3)").arg(filename)
		.arg(context.function)
		.arg(context.line));

		QString timeStr(QDateTime::currentDateTime().toString("dd-MM-yy HH:mm:ss:zzz"));

		std::cerr << timeStr.toLocal8Bit().constData() << " - " << typeStr[type] << " " << contextString.toLocal8Bit().constData() << " " << localMsg.constData() << std::endl;

		if(type == QtFatalMsg) abort();
	}
}

int main(int argc, char *argv[]) {
	qInstallMessageHandler(message_handler);

	cb::CretinsBar app(argc, argv);
	return app.exec();
}
