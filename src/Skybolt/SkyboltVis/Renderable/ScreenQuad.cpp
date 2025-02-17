/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScreenQuad.h"
#include "OsgGeometryFactory.h"

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Projection>

namespace skybolt {
namespace vis {

ScreenQuad::ScreenQuad(osg::StateSet* stateSet, const BoundingBox2f& bounds)
{
	osg::Geometry* quad = createQuadWithUvs(bounds, QuadUpDirectionY);

	quad->setStateSet(stateSet);
	quad->setCullingActive(false);

	//create the geode 
	osg::Geode* geode = new osg::Geode(); 
	geode->addDrawable(quad);

	mTransform->addChild(quad);
}

} // namespace vis
} // namespace skybolt
