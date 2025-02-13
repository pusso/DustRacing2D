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

#include "mccircleshape.hh"
#include "mcobject.hh"

MCCircleShape::MCCircleShape(MCShapeViewPtr view, float radius)
    : MCShape(view)
{
    setRadius(radius);
}

bool MCCircleShape::contains(const MCVector2dF & p) const
{
    return (MCVector2d<float>(location()) - p).lengthFast() <= radius();
}

float MCCircleShape::interpenetrationDepth(const MCCircleShape & p, MCVector2dF & contactNormal) const
{
    contactNormal = MCVector2dF(p.location() - location()).normalizedFast();
    const float depth = radius() + p.radius() - MCVector2dF(p.location() - location()).lengthFast();
    return depth;
}

float MCCircleShape::interpenetrationDepth(const MCSegment<float> & p, MCVector2dF & contactNormal) const
{
    contactNormal = this->contactNormal(p);
    const float depth = radius() - (MCVector2dF(location()) - p.vertex0).lengthFast();
    return depth;
}

MCVector2dF MCCircleShape::contactNormal(const MCSegment<float> & p) const
{
    return (p.vertex0 - MCVector2dF(location())).normalizedFast();
}

MCShape::Type MCCircleShape::type() const
{
    return MCShape::Type::Circle;
}

MCBBox<float> MCCircleShape::bbox() const
{
    return MCBBox<float>(
        MCVector2dF(location()) - MCVector2dF(radius(), radius()), radius() * 2, radius() * 2);
}

MCCircleShape::~MCCircleShape()
{}
