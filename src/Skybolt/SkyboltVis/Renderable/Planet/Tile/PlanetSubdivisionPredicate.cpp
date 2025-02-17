/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlanetSubdivisionPredicate.h"
#include "HeightMap.h"
#include "SkyboltVis/OsgGeocentric.h"
#include "SkyboltVis/OsgMathHelpers.h"
#include "SkyboltVis/Renderable/Planet/Tile/TileSource/TileSource.h"

#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;

namespace skybolt {
namespace vis {

static bool hasAnyChildren(const std::vector<TileSourcePtr>& tileSources, const QuadTreeTileKey& key)
{
	for (const auto& tileSource : tileSources)
	{
		if (tileSource->hasAnyChildren(key))
		{
			return true;
		}
	}
	return false;
}

bool PlanetSubdivisionPredicate::operator()(const Box2d& bounds, const QuadTreeTileKey& key)
{
	if (!hasAnyChildren(tileSources, key))
	{
		return false;
	}

	Box2d latLonBounds(math::vec2SwapComponents(bounds.minimum), math::vec2SwapComponents(bounds.maximum));

	osg::Vec2d latLon = nearestPointInSolidBox(observerLatLon, latLonBounds);

	osg::Vec3d observerPosition = llaToGeocentric(observerLatLon, std::max(1.0, observerAltitude), planetRadius);

	// Assume mean tile elevation is close to sea level. TODO: improve assumtion. We should use min and max tile bounds if available.
	osg::Vec3d tileNearestPointAtSeaLevel = llaToGeocentric(latLon, 0, planetRadius);
	double distanceToTileNearestPoint = (tileNearestPointAtSeaLevel - observerPosition).length();

	osg::Vec3d tileNearestPointAtLowestAltitude = llaToGeocentric(latLon, 0, planetRadius + getHeightmapMinAltitude());

	osg::Vec3d directionFromTileNearestPointAtLowestAltitudeToObserver = (observerPosition - tileNearestPointAtLowestAltitude);
	directionFromTileNearestPointAtLowestAltitudeToObserver.normalize();

	tileNearestPointAtLowestAltitude.normalize();
	float cosElevation = directionFromTileNearestPointAtLowestAltitudeToObserver * tileNearestPointAtLowestAltitude;
	bool visible = (cosElevation > 0.0f);

	if (visible)
	{
		double tileSize = planetRadius / std::pow(2, key.level);
		double projectedSize = tileSize / std::max(0.01, distanceToTileNearestPoint);
		return projectedSize > glm::mix(0.4f, 0.1f, cosElevation); // TODO: tune
	}

	return false;
}

osg::Vec2d PlanetSubdivisionPredicate::nearestPointInSolidBox(const osg::Vec2d& point, const Box2d& bounds) const
{
	// Handle longitude wrap around
	osg::Vec2d wrappedPoint = point;
	double centerLon = bounds.center().y();

	double dist = std::abs(point.y() - centerLon);
	double candidateDist = std::abs(point.y() - math::twoPiD() - centerLon);
	if (candidateDist < dist)
	{
		wrappedPoint.y() -= math::twoPiD();
		dist = candidateDist;
	}
	candidateDist = std::abs(point.y() + math::twoPiD() - centerLon);
	if (candidateDist < dist)
	{
		wrappedPoint.y() += math::twoPiD();
	}

	// Now find nearest point
	return osg::Vec2d(math::clamp(wrappedPoint.x(), bounds.minimum.x(), bounds.maximum.x()),
		math::clamp(wrappedPoint.y(), bounds.minimum.y(), bounds.maximum.y()));
}

} // namespace vis
} // namespace skybolt
