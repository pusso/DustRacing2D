// This file belongs to the "MiniCore" game engine.
// Copyright (C) 2010 Jussi Lind <jussi.lind@iki.fi>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA  02110-1301, USA.
//

#include "mcobjectgrid.hh"
#include "mcphysicscomponent.hh"
#include "mcshape.hh"

#include <algorithm>

MCObjectGrid::MCObjectGrid(float x1, float y1, float x2, float y2, float leafMaxW, float leafMaxH)
: m_bbox(x1, y1, x2, y2)
, m_leafMaxW(leafMaxW)
, m_leafMaxH(leafMaxH)
, m_horSize((x2 - x1) / m_leafMaxW)
, m_verSize((y2 - y1) / m_leafMaxH)
, m_helpHor(static_cast<float>(m_horSize) / (x2 - x1))
, m_helpVer(static_cast<float>(m_verSize) / (y2 - y1))
{
    build();
}

MCObjectGrid::~MCObjectGrid()
{
    for (GridCell * cell : m_matrix)
    {
        delete cell;
    }
}

void MCObjectGrid::setIndexRange(const MCBBox<float> & bbox)
{
    auto temp = static_cast<int>(bbox.x1() * m_helpHor);
    if (temp >= static_cast<int>(m_horSize)) temp = m_horSize - 1;
    else if (temp < 0) temp = 0;
    m_i0 = static_cast<size_t>(temp);

    temp = static_cast<int>(bbox.x2() * m_helpHor);
    if (temp >= static_cast<int>(m_horSize)) temp = m_horSize - 1;
    else if (temp < 0) temp = 0;
    m_i1 = static_cast<size_t>(temp);

    temp = static_cast<int>(bbox.y1() * m_helpVer);
    if (temp >= static_cast<int>(m_verSize)) temp = m_verSize - 1;
    else if (temp < 0) temp = 0;
    m_j0 = static_cast<size_t>(temp);

    temp = static_cast<int>(bbox.y2() * m_helpVer);
    if (temp >= static_cast<int>(m_verSize)) temp = m_verSize - 1;
    else if (temp < 0) temp = 0;
    m_j1 = static_cast<size_t>(temp);
}

void MCObjectGrid::insert(MCObject & object)
{
    if (!object.shape())
    {
        return;
    }

    setIndexRange(object.shape()->bbox());
    object.cacheIndexRange(m_i0, m_i1, m_j0, m_j1);

    for (size_t j = m_j0; j <= m_j1; j++)
    {
        for (size_t i = m_i0; i <= m_i1; i++)
        {
            auto && cell = m_matrix[j * m_horSize + i];
            cell->m_objects.insert(&object);
            m_dirtyCellCache.insert(cell);
        }
    }
}

bool MCObjectGrid::remove(MCObject & object)
{
    if (!object.shape())
    {
        return false;
    }

    bool removed = false;
    object.restoreIndexRange(&m_i0, &m_i1, &m_j0, &m_j1);

    for (size_t j = m_j0; j <= m_j1; j++)
    {
        for (size_t i = m_i0; i <= m_i1; i++)
        {
            const auto cell = m_matrix[j * m_horSize + i];
            const auto iter = cell->m_objects.find(&object);
            if (iter != cell->m_objects.end())
            {
                cell->m_objects.erase(iter);
                removed = true;

                if (!cell->m_objects.size())
                {
                    m_dirtyCellCache.erase(cell);
                }
            }
        }
    }
    return removed;
}

void MCObjectGrid::removeAll()
{
    for (size_t j = 0; j < m_verSize; j++)
    {
        for (size_t i = 0; i < m_horSize; i++)
        {
            m_matrix[j * m_horSize + i]->m_objects.clear();
        }
    }

    m_dirtyCellCache.clear();
}

void MCObjectGrid::build()
{
    m_dirtyCellCache.clear();

    for (auto cell : m_matrix)
    {
        delete cell;
    }

    m_matrix.clear();

    for (size_t j = 0; j < m_verSize; j++)
    {
        for (size_t i = 0; i < m_horSize; i++)
        {
            m_matrix.push_back(new GridCell);
        }
    }
}

const MCObjectGrid::CollisionVector & MCObjectGrid::getPossibleCollisions()
{
    static MCObjectGrid::CollisionVector collisions;
    collisions.clear();

    // Optimization: ignore collisions between sleeping objects.
    // Note that stationary objects are also sleeping objects.

    auto cellIter = m_dirtyCellCache.begin();
    while (cellIter != m_dirtyCellCache.end())
    {
        bool hadCollisions = false;
        auto & objects = (*cellIter)->m_objects;

        const auto end = objects.end();
        for (auto && objIter1 = objects.begin(); objIter1 != end; objIter1++)
        {
            auto * obj1 = *objIter1;
            for (auto && objIter2 = std::next(objIter1); objIter2 != end; objIter2++)
            {
                auto * obj2 = *objIter2; // Note that ob1 != obj2 always holds
                if (&obj1->parent() != obj2 &&
                    &obj2->parent() != obj1 &&
                    (!obj1->physicsComponent().isSleeping() || !obj2->physicsComponent().isSleeping()) &&
                    (obj1->isPhysicsObject() || obj1->isTriggerObject()) && !obj1->bypassCollisions() &&
                    (obj2->isPhysicsObject() || obj2->isTriggerObject()) && !obj2->bypassCollisions() &&
                    obj1->physicsComponent().neverCollideWithTag() != obj2->physicsComponent().collisionTag() &&
                    obj2->physicsComponent().neverCollideWithTag() != obj1->physicsComponent().collisionTag() &&
                    (obj1->collisionLayer() == obj2->collisionLayer() || obj1->collisionLayer() == -1 || obj2->collisionLayer() == -1) &&
                    obj1->shape()->likelyIntersects(*obj2->shape().get()))
                {
                    collisions.push_back({obj1, obj2});
                    hadCollisions = true;
                }
            }
        }

        if (!hadCollisions)
        {
            cellIter = m_dirtyCellCache.erase(cellIter);
        }
        else
        {
            cellIter++;
        }
    }

    return collisions;
}

const MCObjectGrid::ObjectSet & MCObjectGrid::getObjectsWithinDistance(const MCVector2dF & p, float d)
{
    return getObjectsWithinDistance(p.i(), p.j(), d);
}

const MCObjectGrid::ObjectSet & MCObjectGrid::getObjectsWithinDistance(float x, float y, float d)
{
    return getObjectsWithinBBox(MCBBox<float>(x - d, y - d, x + d, y + d));
}

const MCObjectGrid::ObjectSet & MCObjectGrid::getObjectsWithinBBox(const MCBBox<float> & bbox)
{
    setIndexRange(bbox);

    static ObjectSet resultObjs;
    resultObjs.clear();

    for (size_t j = m_j0; j <= m_j1; j++)
    {
        for (size_t i = m_i0; i <= m_i1; i++)
        {
            for (auto && obj : m_matrix[j * m_horSize + i]->m_objects)
            {
                if (obj->shape()->view())
                {
                    if (bbox.intersects(obj->shape()->view()->bbox().translated(MCVector2dF(obj->location()))))
                    {
                        resultObjs.insert(obj);
                    }
                }
            }
        }
    }

    return resultObjs;
}

const MCBBox<float> & MCObjectGrid::bbox() const
{
    return m_bbox;
}
