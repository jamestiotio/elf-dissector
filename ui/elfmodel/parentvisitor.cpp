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

#include "parentvisitor.h"
#include "elfmodel.h"

#include <elf/elffileset.h>
#include <elf/elfgnusymbolversiondefinitionauxiliaryentry.h>
#include <elf/elfgnusymbolversionrequirement.h>
#include <dwarf/dwarfcudie.h>

#include <QDebug>

#include <elf.h>

#include <cassert>

ParentVisitor::ParentVisitor(const ElfModel* model) : m_model(model)
{
}

ParentVisitor::type ParentVisitor::doVisit(ElfFile* file, int) const
{
    int row = 0;
    for (; row < m_model->fileSet()->size(); ++row) {
        if (m_model->fileSet()->file(row) == file)
            break;
    }
    return makeParent(m_model->fileSet(), row);
}

ParentVisitor::type ParentVisitor::doVisit(ElfSection* section, int) const
{
    return makeParent(section->file(), section->header()->sectionIndex());
}

ParentVisitor::type ParentVisitor::doVisit(ElfGNUSymbolVersionDefinition* verDef, int) const
{
    int row = -1;
    for (uint i = 0; i < verDef->section()->entryCount(); ++i) {
        if (verDef->section()->definition(i) == verDef) {
            row = i;
            break;
        }
    }
    assert(row >= 0);
    return makeParent(verDef->section(), row);
}

ParentVisitor::type ParentVisitor::doVisit(ElfGNUSymbolVersionRequirement *verNeed, int) const
{
    int row = -1;
    for (uint i = 0; i < verNeed->section()->entryCount(); ++i) {
        if (verNeed->section()->requirement(i) == verNeed) {
            row = i;
            break;
        }
    }
    assert(row >= 0);
    return makeParent(verNeed->section(), row);
}

ParentVisitor::type ParentVisitor::doVisit(ElfSymbolTableEntry *symbol, int) const
{
    int row = -1;
    for (uint i = 0; i < symbol->symbolTable()->size(); ++i) {
        if (symbol->symbolTable()->entry(i) == symbol) {
            row = i;
            break;
        }
    }
    assert(row >= 0);
    return makeParent(const_cast<ElfSymbolTableSection*>(symbol->symbolTable()), row);
}

ParentVisitor::type ParentVisitor::doVisit(DwarfInfo* info, int) const
{
    return makeParent(info->elfFile(), info->elfFile()->indexOfSection(".debug_info"));
}

ParentVisitor::type ParentVisitor::doVisit(DwarfDie* die, int) const
{
    if (die->parentDie()) {
        return makeParent(die->parentDie(), die->parentDie()->children().indexOf(die));
    }
    return makeParent(die->dwarfInfo(), die->dwarfInfo()->compilationUnits().indexOf(static_cast<DwarfCuDie*>(die)));
}

ParentVisitor::type ParentVisitor::makeParent(void* payload, int row) const
{
    return qMakePair<void*, int>(payload, row);
}
