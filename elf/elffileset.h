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

#ifndef ELFFILESET_H
#define ELFFILESET_H

#include "elffile.h"

#include <QObject>
#include <QStringList>

/** A set of ELF files. */
class ElfFileSet : public QObject
{
    Q_OBJECT
public:
    explicit ElfFileSet(QObject* parent = 0);
    ~ElfFileSet();

    int size() const;
    void addFile(const QString &fileName);

    ElfFile::Ptr file(int index) const;

    void topologicalSort();
private:
    QString findLibrary(const QString &name) const;

    QVector<ElfFile::Ptr> m_files;
    QStringList m_searchPaths;
};

#endif // ELFFILESET_H
