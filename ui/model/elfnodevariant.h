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

#ifndef ELFNODEVARIANT_H
#define ELFNODEVARIANT_H

struct ElfNodeVariant
{
    enum Type {
        Invalid,
        FileSet,
        File,
        Section,
        SymbolTableSection,
        SymbolTableEntry,
        DynamicSection,
        DynamicEntry,
        NoteSection,
        NoteEntry,
        RelocationSection,
        RelocationEntry,
        VersionDefinitionSection,
        VersionDefinitionEntry,
        VersionDefinitionAuxiliaryEntry,
        DwarfInfo,
        DwarfDie
    };

    template <typename T> T* value() const
    {
        return static_cast<T*>(payload);
    }

    void *payload = 0;
    Type type = Invalid;
};

#endif