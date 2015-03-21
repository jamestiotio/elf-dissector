/*
    Copyright (C) 2013-2014 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ui/mainwindow.h>

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char** argv)
{
    QCoreApplication::setApplicationName("ELF Dissector");
    QCoreApplication::setOrganizationName("KDAB");
    QCoreApplication::setOrganizationDomain("kdab.com");
    QCoreApplication::setApplicationVersion("0.0.1");

    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("elf", "ELF library to open", "<elf>");
    parser.process(app);

    MainWindow mainWindow;
    mainWindow.show();
    foreach (const QString &file, parser.positionalArguments())
        mainWindow.loadFile(file);

    return app.exec();
}