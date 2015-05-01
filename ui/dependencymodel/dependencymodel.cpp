/*
    Copyright (C) 2015 Volker Krause <vkrause@kde.org>

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

#include "dependencymodel.h"

#include <elf/elffileset.h>
#include <elf/elffile.h>
#include <elf/elfdynamicsection.h>

#include <QDebug>
#include <QIcon>

#include <cassert>
#include <memory>

DependencyModel::DependencyModel(QObject* parent): QAbstractItemModel(parent)
{
}

DependencyModel::~DependencyModel()
{
}

void DependencyModel::setFileSet(ElfFileSet* fileSet)
{
    if (m_fileSet == fileSet)
        return;

    beginResetModel();
    const auto l = [](DependencyModel* m) { m->endResetModel(); };
    const auto endReset = std::unique_ptr<DependencyModel, decltype(l)>(this, l);

    m_fileIndex.clear();
    m_childMap.clear();
    m_parentMap.clear();
    m_uniqueIndex = 0;

    m_fileSet = fileSet;
    if (!fileSet || fileSet->size() == 0)
        return;

    // build up indexes to make the tree building more efficient
    for (int i = 0; i < fileSet->size(); ++i) {
        const auto file = fileSet->file(i);
        const auto soName = file->dynamicSection()->soName();
        if (!soName.isEmpty())
            m_fileIndex.insert(soName, i);
    }

    // setup root
    m_parentMap.resize(1);
    m_parentMap[0] = 0;
    m_childMap.resize(1);

    // TODO identify top-level files (for now we just assume it's the first one)

    // fill the first level
    ++m_uniqueIndex;
    assert((uint32_t)m_parentMap.size() == m_uniqueIndex);
    assert((uint32_t)m_childMap.size() == m_uniqueIndex);

    m_parentMap.push_back(0);
    m_childMap[0].push_back(makeId(m_uniqueIndex, 0));
    m_childMap.push_back({});
}

QVariant DependencyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_fileSet)
        return {};

    const auto file = fileIndex(index.internalId());

    // TODO show number of symbols used for parent/child pair in second column
    switch (role) {
        case Qt::DisplayRole:
        {
            if (file != InvalidFile)
                return m_fileSet->file(file)->displayName(); // TODO just file name / SONAME
            const auto parentIdx = parent(index);
            const auto parentFile = fileIndex(parentIdx.internalId());
            return m_fileSet->file(parentFile)->dynamicSection()->neededLibraries().at(index.row());
        }
        case Qt::DecorationRole:
            if (file == InvalidFile)
                return QIcon::fromTheme("dialog-error");
            if (hasCycle(index))
                return QIcon::fromTheme("dialog-warning");
            break;
        case Qt::ToolTipRole:
            if (hasCycle(index))
                return tr("Cyclic dependency!");
            if (file != InvalidFile)
                return m_fileSet->file(file)->displayName(); // TODO full path
            else
                return tr("Dependency not found!");
    }

    return {};
}

int DependencyModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

int DependencyModel::rowCount(const QModelIndex& parent) const
{
    if (!m_fileSet)
        return 0;

    const auto node = nodeId(parent.internalId());
    assert(node < (uint32_t)m_childMap.size());

    {
        // this reference will become invalid as soon as we modify m_childMap below!
        const auto &children = m_childMap.at(node);
        if (!children.isEmpty())
            return children.size();
    }

    const auto file = fileIndex(parent.internalId());
    if (file == InvalidFile || hasCycle(parent))
        return 0;

    const auto needed = m_fileSet->file(file)->dynamicSection()->neededLibraries();
    if (needed.isEmpty())
        return 0;

    for (const auto need : needed) {
        const uint64_t childNode = makeId(++m_uniqueIndex, fileIndex(need));
        m_parentMap.push_back(parent.internalId());
        m_childMap.push_back({});
        m_childMap[node].push_back(childNode);
        assert((uint32_t)m_parentMap.size() == m_uniqueIndex + 1);
        assert((uint32_t)m_childMap.size() == m_uniqueIndex + 1);
    }

    return m_childMap.at(node).size();
}

QModelIndex DependencyModel::parent(const QModelIndex& child) const
{
    if (!m_fileSet || !child.isValid())
        return {};

    const auto node = nodeId(child.internalId());
    assert((uint32_t)m_parentMap.size() > node);

    const auto parentId = m_parentMap.at(node);
    const auto parentNode = nodeId(parentId);
    if (parentNode == 0)
        return {};

    assert((uint32_t)m_parentMap.size() > parentNode);
    const auto grandParentNode = nodeId(m_parentMap.at(parentNode));

    assert((uint32_t)m_childMap.size() > grandParentNode);
    const auto &children = m_childMap.at(grandParentNode);
    const auto row = children.indexOf(parentId);
    assert(row >= 0);
    return createIndex(row, 0, parentId);
}

QModelIndex DependencyModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!m_fileSet || !hasIndex(row, column, parent))
        return {};

    const auto node = nodeId(parent.internalId());
    assert(node < (uint32_t)m_childMap.size());

    const auto &children = m_childMap.at(node);
    if (row >= children.size())
        return {};
    assert(row < children.size());

    return createIndex(row, column, children.at(row));
}

uint64_t DependencyModel::makeId(uint32_t id, int32_t fileIndex) const
{
    uint64_t f = (uint32_t)fileIndex;
    return id | f << 32;
}

int32_t DependencyModel::fileIndex(uint64_t qmiId) const
{
    return qmiId >> 32;
}

int32_t DependencyModel::fileIndex(const QByteArray& needed) const
{
    const auto it = m_fileIndex.constFind(needed);
    if (it != m_fileIndex.cend())
        return it.value();
    return InvalidFile;
}

uint32_t DependencyModel::nodeId(uint64_t qmiId) const
{
    return qmiId;
}

bool DependencyModel::hasCycle(const QModelIndex& index) const
{
    const auto file = fileIndex(index.internalId());

    QModelIndex parentIndex(index);
    forever {
        parentIndex = parent(parentIndex);
        if (!parentIndex.isValid())
            return false;
        const auto parentFile = fileIndex(parentIndex.internalId());
        if (parentFile == file)
            return true;
    }
    Q_UNREACHABLE();
}
