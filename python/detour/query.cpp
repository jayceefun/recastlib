/*
 * @summary: detour navmesh query
 * @date: 2013-03-26
 * @author: zl
 */

#include "config.h"
#include "query.h"
#include "dtmath.h"
#include "common.h"

using boost::python::override;
using boost::python::error_already_set;


inline float* typecast(dtVec3& v) {
	return &(v.x);
}


dtQueryFilterWraper::dtQueryFilterWraper(): dtQueryFilter()
{
}

dtQueryFilterWraper::~dtQueryFilterWraper()
{
}

bool dtQueryFilterWraper::passFilter(
		const dtPolyRef ref, const dtMeshTile *tile, const dtPoly *poly) const
{
	override func = this->get_override("passFilter");
	if (func)
	{
		try
		{
			return func(ref, boost::ref(tile), boost::ref(poly));
		}
		catch (const error_already_set&)
		{
			if (PyErr_Occurred())
				PyErr_Print();

			return false;
		}
	}
	return dtQueryFilter::passFilter(ref, tile, poly);
}

float dtQueryFilterWraper::getCost(
		const float *pa, const float *pb,
		const dtPolyRef prevRef, const dtMeshTile *prevTile, const dtPoly *prevPoly,
		const dtPolyRef curRef, const dtMeshTile *curTile, const dtPoly *curPoly,
		const dtPolyRef nextRef, const dtMeshTile *nextTile, const dtPoly *nextPoly) const
{
	override func = this->get_override("getCost");
	if (func)
	{
		dtVec3 vpa(pa);
		dtVec3 vpb(pb);
		try
		{
			return func(vpa, vpb,
					prevRef, boost::ref(prevTile), boost::ref(prevPoly),
					curRef, boost::ref(curTile), boost::ref(curPoly),
					nextRef, boost::ref(nextTile), boost::ref(nextPoly));
		}
		catch (const error_already_set&)
		{
			if (PyErr_Occurred())
				PyErr_Print();

			return FLT_MAX;
		}
	}
	return dtQueryFilter::getCost(pa, pb,
			prevRef, prevTile, prevPoly,
			curRef, curTile, curPoly,
			nextRef, nextTile, nextPoly);
}

float dtQueryFilterWraper::getAreaCost(const int i) const
{
	override func = this->get_override("getAreaCost");
	if (func)
	{
		try
		{
			return func(i);
		}
		catch (const error_already_set&)
		{
			if (PyErr_Occurred())
				PyErr_Print();

			return FLT_MAX;
		}
	}

	dtAssert(i >= 0 && i < DT_MAX_AREAS);
	return dtQueryFilter::getAreaCost(i);
}

void dtQueryFilterWraper::setAreaCost(const int i, const float cost)
{
	override func = this->get_override("setAreaCost");
	if (func)
	{
		try
		{
			func(i, cost);
		}
		catch (const error_already_set&)
		{
			if (PyErr_Occurred())
				PyErr_Print();

			return;
		}
	}

	dtAssert(i >= 0 && i < DT_MAX_AREAS);
	dtQueryFilter::setAreaCost(i, cost);
}


dtNavMeshQueryWraper::dtNavMeshQueryWraper() :
		navq_(dtAllocNavMeshQuery()) {
	if (!navq_)
		MemoryError();
}

dtNavMeshQueryWraper::~dtNavMeshQueryWraper() {
	dtFreeNavMeshQuery(navq_);
}

dtStatus dtNavMeshQueryWraper::init(const dtNavMesh *nav, const int maxNodes) const
{
	return navq_->init(nav, maxNodes);
}

dtStatus dtNavMeshQueryWraper::findPath(dtPolyRef startRef, dtPolyRef endRef,
		dtVec3 startPos, dtVec3 endPos, const dtQueryFilter *filter, dict out,
		const int maxPath) const {
	dtAssert(maxPath > 0);

	int pathCount = 0;
	dtPolyRefList path(maxPath);

	dtStatus status = navq_->findPath(startRef, endRef,
		typecast(startPos), typecast(endPos), filter, &path[0], &pathCount, maxPath);
	path.resize(pathCount);
	out["pathCount"] = pathCount;
	out["path"] = path;
	return status;
}

dtStatus dtNavMeshQueryWraper::findStraightPath(dtVec3 startPos, dtVec3 endPos,
		dtPolyRefList path, dict out, const int maxStraightPath, const int options) const {
	dtAssert(maxStraightPath > 0);
	dtAssert((int)path.size() > 0);

	int straightPathCount = 0;
	dtVec3List straightPath(maxStraightPath);
	ByteList straightPathFlags(maxStraightPath);
	dtPolyRefList straightPathRefs(maxStraightPath);

	dtStatus status = navq_->findStraightPath(typecast(startPos), typecast(endPos),
		&path[0], (int)path.size(),
		typecast(straightPath[0]), &straightPathFlags[0],
		&straightPathRefs[0], &straightPathCount, maxStraightPath, options);
	straightPath.resize(straightPathCount);
	straightPathFlags.resize(straightPathCount);
	straightPathRefs.resize(straightPathCount);
	out["straightPathCount"] = straightPathCount;
	out["straightPath"] = straightPath;
	out["straightPathFlags"] = straightPathFlags;
	out["straightPathRefs"] = straightPathRefs;
	return status;
}

dtStatus dtNavMeshQueryWraper::initSlicedFindPath(dtPolyRef startRef,
		dtPolyRef endRef, dtVec3 startPos, dtVec3 endPos,
		const dtQueryFilter *filter) {
	return navq_->initSlicedFindPath(startRef, endRef,
			typecast(startPos), typecast(endPos), filter);
}

dtStatus dtNavMeshQueryWraper::updateSlicedFindPath(const int maxIter,
		dict out) {
	dtAssert(maxIter > 0);

	int doneIters = 0;

	dtStatus status = navq_->updateSlicedFindPath(maxIter, &doneIters);
	out["doneIters"] = doneIters;
	return status;
}

dtStatus dtNavMeshQueryWraper::finalizeSlicedFindPath(dict out,
		const int maxPath) {
	dtAssert(maxPath > 0);

	int pathCount = 0;
	dtPolyRefList path(maxPath);

	dtStatus status = navq_->finalizeSlicedFindPath(&path[0], &pathCount, maxPath);
	path.resize(pathCount);
	out["pathCount"] = pathCount;
	out["path"] = path;
	return status;
}

dtStatus dtNavMeshQueryWraper::finalizeSlicedFindPathPartial(
		dtPolyRefList existing, dict out, const int maxPath) {
	dtAssert(maxPath > 0);

	int pathCount = 0;
	dtPolyRefList path(maxPath);

	dtStatus status = navq_->finalizeSlicedFindPathPartial(
			&existing[0], (int)existing.size(),
			&path[0], &pathCount, maxPath);
	out["pathCount"] = pathCount;
	out["path"] = path;
	return status;
}

dtStatus dtNavMeshQueryWraper::findPolysAroundCircle(dtPolyRef startRef,
		dtVec3 centerPos, const float radius, const dtQueryFilter *filter,
		dict out, const int maxResult) const {
	dtAssert(maxResult > 0);

	int resultCount = 0;
	dtPolyRefList resultRef(maxResult);
	dtPolyRefList resultParent(maxResult);
	FloatList resultCost(maxResult);

	dtStatus status = navq_->findPolysAroundCircle(startRef,
			typecast(centerPos), radius, filter,
			&resultRef[0], &resultParent[0], &resultCost[0],
			&resultCount, maxResult);
	out["resultCount"] = resultCount;
	out["resultRef"] = resultRef;
	out["resultParent"] = resultParent;
	out["resultCost"] = resultCost;
	return status;
}

dtStatus dtNavMeshQueryWraper::findPolysAroundShape(dtPolyRef startRef,
		dtVec3List verts, const dtQueryFilter *filter, dict out,
		const int maxResult) const {
	dtAssert(maxResult > 0);

	int resultCount = 0;
	dtPolyRefList resultRef(maxResult);
	dtPolyRefList resultParent(maxResult);
	FloatList resultCost(maxResult);

	dtStatus status = navq_->findPolysAroundShape(startRef,
			typecast(verts[0]), (int)verts.size(), filter,
			&resultRef[0], &resultParent[0], &resultCost[0],
			&resultCount, maxResult);
	out["resultCount"] = resultCount;
	out["resultRef"] = resultRef;
	out["resultParent"] = resultParent;
	out["resultCost"] = resultCost;
	return status;
}

dtStatus dtNavMeshQueryWraper::findNearestPoly(dtVec3 center, dtVec3 extents,
		const dtQueryFilter *filter, dict out) const {
	dtVec3 nearestPt;
	dtPolyRef nearestRef = 0;

	dtStatus status = navq_->findNearestPoly(typecast(center), typecast(extents),
			filter, &nearestRef, typecast(nearestPt));
	out["nearestRef"] = nearestRef;
	out["nearestPt"] = nearestPt;
	return status;
}

dtStatus dtNavMeshQueryWraper::queryPolygons(dtVec3 center, dtVec3 extents,
		const dtQueryFilter *filter, dict out, const int maxPolys) const {
	dtAssert(maxPolys > 0);

	int polyCount = 0;
	dtPolyRefList polys(maxPolys);

	dtStatus status = navq_->queryPolygons(typecast(center), typecast(extents),
			filter, &polys[0], &polyCount, maxPolys);
	out["polyCount"] = polyCount;
	out["polys"] = polys;
	return status;
}

dtStatus dtNavMeshQueryWraper::findLocalNeighbourhood(dtPolyRef startRef,
		dtVec3 centerPos, const float radius, const dtQueryFilter *filter,
		dict out, const int maxResult) const {
	dtAssert(maxResult > 0);

	int resultCount = 0;
	dtPolyRefList resultRef(maxResult);
	dtPolyRefList resultParent(maxResult);

	dtStatus status = navq_->findLocalNeighbourhood(startRef,
			typecast(centerPos), radius, filter,
			&resultRef[0], &resultParent[0], &resultCount, maxResult);
	out["resultCount"] = resultCount;
	out["resultRef"] = resultRef;
	out["resultParent"] = resultParent;
	return status;
}

dtStatus dtNavMeshQueryWraper::moveAlongSurface(dtPolyRef startRef,
		dtVec3 startPos, dtVec3 endPos, const dtQueryFilter *filter, dict out,
		const int maxVisitedSize) const {
	dtAssert(maxVisitedSize > 0);

	int visitedCount = 0;
	dtVec3 resultPos;
	dtPolyRefList visited(maxVisitedSize);

	dtStatus status = navq_->moveAlongSurface(startRef,
			typecast(startPos), typecast(endPos), filter,
			typecast(resultPos), &visited[0], &visitedCount, maxVisitedSize);
	out["resultPos"] = resultPos;
	out["visitedCount"] = visitedCount;
	out["visited"] = visited;
	return status;
}

dtStatus dtNavMeshQueryWraper::raycast(dtPolyRef startRef, dtVec3 startPos,
		dtVec3 endPos, const dtQueryFilter *filter, dict out,
		const int maxPath) const {
	dtAssert(maxPath > 0);

	int pathCount = 0;
	float t = FLT_MAX;
	dtVec3 hitNormal;
	dtPolyRefList path(maxPath);

	dtStatus status = navq_->raycast(startRef,
			typecast(startPos), typecast(endPos), filter,
			&t, typecast(hitNormal), &path[0], &pathCount, maxPath);
	out["t"] = t;
	out["hitNormal"] = hitNormal;
	out["pathCount"] = pathCount;
	out["path"] = path;
	return status;
}

dtStatus dtNavMeshQueryWraper::findDistanceToWall(dtPolyRef startRef,
		dtVec3 centerPos, const float maxRadius, const dtQueryFilter *filter,
		dict out) const {

	float hitDist = maxRadius;
	dtVec3 hitPos, hitNormal;

	dtStatus status = navq_->findDistanceToWall(startRef,
			typecast(centerPos), maxRadius, filter,
			&hitDist, typecast(hitPos), typecast(hitNormal));
	out["hitDist"] = hitDist;
	out["hitPos"] = hitPos;
	out["hitNormal"] = hitNormal;
	return status;
}

dtStatus dtNavMeshQueryWraper::getPolyWallSegments(dtPolyRef ref,
		const dtQueryFilter *filter, dict out, const int maxSegments) const {
	dtAssert(maxSegments > 0);

	int segmentCount = 0;
	dtVec3List segmentVerts(maxSegments * 2);
	dtPolyRefList segmentRefs(maxSegments);

	dtStatus status = navq_->getPolyWallSegments(ref, filter,
			typecast(segmentVerts[0]), &segmentRefs[0], &segmentCount, maxSegments);
	out["segmentCount"] = segmentCount;
	out["segmentVerts"] = segmentVerts;
	out["segmentRefs"] = segmentRefs;
	return status;
}

dtStatus dtNavMeshQueryWraper::findRandomPoint(const dtQueryFilter *filter,
		dict out) const {
	dtVec3 randomPt;
	dtPolyRef randomRef = 0;

	dtStatus status = navq_->findRandomPoint(filter,
			&frand_01, &randomRef, typecast(randomPt));
	out["randomRef"] = randomRef;
	out["randomPt"] = randomPt;
	return status;
}

dtStatus dtNavMeshQueryWraper::findRandomPointAroundCircle(dtPolyRef startRef,
		dtVec3 centerPos, const float maxRadius, const dtQueryFilter *filter,
		dict out) const {
	dtVec3 randomPt;
	dtPolyRef randomRef = 0;

	dtStatus status = navq_->findRandomPointAroundCircle(startRef,
			typecast(centerPos), maxRadius, filter,
			&frand_01, &randomRef, typecast(randomPt));
	out["randomRef"] = randomRef;
	out["randomPt"] = randomPt;
	return status;
}

dtStatus dtNavMeshQueryWraper::closestPointOnPoly(dtPolyRef ref, dtVec3 pos,
		dict out) const {
	dtVec3 closest;

	dtStatus status = navq_->closestPointOnPoly(ref,
			typecast(pos), typecast(closest));
	out["closest"] = closest;
	return status;
}

dtStatus dtNavMeshQueryWraper::closestPointOnPolyBoundary(dtPolyRef ref,
		dtVec3 pos, dict out) const {
	dtVec3 closest;

	dtStatus status = navq_->closestPointOnPolyBoundary(ref,
				typecast(pos), typecast(closest));
	out["closest"] = closest;
	return status;
}

dtStatus dtNavMeshQueryWraper::getPolyHeight(dtPolyRef ref, dtVec3 pos,
		dict out) const {
	float height = 0.0f;

	dtStatus status = navq_->getPolyHeight(ref, typecast(pos), &height);
	out["height"] = height;
	return status;
}

bool dtNavMeshQueryWraper::isValidPolyRef(dtPolyRef ref,
		const dtQueryFilter *filter) const {
	return navq_->isValidPolyRef(ref, filter);
}

bool dtNavMeshQueryWraper::isInClosedList(dtPolyRef ref) const {
	return navq_->isInClosedList(ref);
}

const dtNavMesh* dtNavMeshQueryWraper::getAttachedNavMesh() const {
	return navq_->getAttachedNavMesh();
}

dtNodePool* dtNavMeshQueryWraper::getNodePool() const {
	return navq_->getNodePool();
}
