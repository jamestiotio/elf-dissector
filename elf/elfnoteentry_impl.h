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

#ifndef ELFNOTEENTRY_IMPL_H
#define ELFNOTEENTRY_IMPL_H

#include "elfnoteentry.h"

template <typename T>
class ElfNoteEntryImpl : public ElfNoteEntry
{
public:
    explicit ElfNoteEntryImpl(const ElfNoteSection* section, uint64_t offset) :
        ElfNoteEntry(section),
        m_note(reinterpret_cast<const T*>(section->rawData() + offset))
    {
    }

    uint64_t type() const final
    {
        return m_note->n_type;
    }

    uint64_t size() const final
    {
        return alignTo(nameSize(), sizeof(T::n_namesz)) + alignTo(descriptionSize(), sizeof(T::n_namesz)) + sizeof(T);
    }

    const char* name() const final
    {
        return reinterpret_cast<const char*>(m_note) + sizeof(T);
    }

protected:
    uint64_t nameSize() const final
    {
        return m_note->n_namesz;
    }

    uint64_t descriptionSize() const final
    {
        return m_note->n_descsz;
    }

private:
    static uint64_t alignTo(uint64_t value, uint64_t size)
    {
        const auto r = value % size;
        if (!r)
            return value;
        return value + size - r;
    }

    const T * const m_note;
};

#endif
