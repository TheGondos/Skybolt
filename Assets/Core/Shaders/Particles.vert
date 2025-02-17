/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 440 core
#include "AtmosphericScatteringWithClouds.h"
#include "DepthPrecision.h"

#pragma import_defines ( CAST_SHADOWS )

in vec4 osg_Vertex;
in vec4 osg_MultiTexCoord0;
out vec2 texCoord;
out float alpha;
out float logZ;
out vec3 positionRelCamera;
out AtmosphericScattering scattering;

uniform mat4 osg_ModelViewProjectionMatrix;
uniform vec3 cameraPosition;
uniform vec3 cameraUpDirection;
uniform vec3 cameraRightDirection;
uniform vec3 lightDirection;

uniform sampler2D cloudSampler;

vec2 rotate(vec2 v, float a)
{
	float s = sin(a);
	float c = cos(a);
	mat2 m = mat2(c, -s, s, c);
	return m * v;
}

void main()
{
	float x = ((gl_VertexID + 1) % 4) > 1 ? 1.0f : 0.0f;
	float y = (gl_VertexID % 4) / 2;
	
	texCoord = vec2(x, y);
	
	vec4 pos = osg_Vertex;
	vec2 offset = (texCoord - 0.5) * osg_MultiTexCoord0.xx * 2.0;
	float rotationAngle = osg_MultiTexCoord0.z;
	offset = rotate(offset, rotationAngle);
	pos.xyz += offset.x * cameraRightDirection + offset.y * cameraUpDirection;
	
	alpha = osg_MultiTexCoord0.y;

	gl_Position = osg_ModelViewProjectionMatrix * pos;
	
#ifdef CAST_SHADOWS
	return;
#endif
	gl_Position.z = logarithmicZ_vertexShader(gl_Position.z, gl_Position.w, logZ);
	
	
	vec3 positionWS = osg_Vertex.xyz; // assume particles are in world coordinates
	positionRelCamera = positionWS.xyz - cameraPosition;
	
	// Atmospheric scattering
	vec3 positionRelPlanet = positionWS.xyz - planetCenter;
	vec3 cameraPositionRelPlanet = cameraPosition - planetCenter;
	scattering = calcAtmosphericScattering(cameraPositionRelPlanet, positionRelPlanet, lightDirection, cloudSampler);
}
