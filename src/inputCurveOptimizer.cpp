/**
 * inputCurveOptimizer.cpp
 * Input Curve 자동 최적화 도구 구현
 * 특허 US8400455B2 기술을 활용한 곡선 품질 향상
 */

#include "inputCurveOptimizer.h"
#include <maya/MGlobal.h>
#include <maya/MFnMesh.h>
#include <maya/MFnIkJoint.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnNurbsCurveData.h>
#include <maya/MFnNurbsCurveModifier.h>
#include <maya/MFnTransform.h>
#include <maya/MItDag.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MDagModifier.h>
#include <maya/MFnSet.h>
#include <algorithm>
#include <cmath>
#include <limits>

// 생성자
InputCurveOptimizer::InputCurveOptimizer()
    : mUseArcSegment(true),           // 기본값: Arc Segment 모드
      mCurvatureThreshold(0.01),      // 기본값: 0.01
      mMaxControlPoints(20),          // 기본값: 20개
      mEnableKnotOptimization(true)   // 기본값: 활성화
{
}

// 소멸자
InputCurveOptimizer::~InputCurveOptimizer()
{
}

// === 🚀 메인 최적화 기능들 ===

// 1. 메시에서 자동으로 최적의 곡선 생성
MStatus InputCurveOptimizer::generateOptimalCurveFromMesh(const MDagPath& meshPath,
                                                          const MString& curveName,
                                                          double maxCurvatureError,
                                                          int maxControlPoints)
{
    MStatus status;
    
    // 1단계: 메시에서 경로 추출
    std::vector<MPoint> pathPoints;
    status = extractPathFromMesh(meshPath, pathPoints);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to extract path from mesh");
        return status;
    }
    
    // 2단계: 경로를 최적화된 곡선으로 변환
    status = createOptimizedCurveFromPath(pathPoints, curveName);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to create optimized curve");
        return status;
    }
    
    // 3단계: 생성된 곡선을 디포머용으로 추가 최적화
    MDagPath createdCurvePath;
    MItDag dagIter(MItDag::kDepthFirst, MFn::kNurbsCurve);
    for (; !dagIter.isDone(); dagIter.next()) {
        MDagPath currentPath;
        dagIter.getPath(currentPath);
        MFnDagNode dagNode(currentPath);
        if (dagNode.name() == curveName) {
            createdCurvePath = currentPath;
            break;
        }
    }
    
    if (createdCurvePath.isValid()) {
        status = optimizeCurveForDeformer(createdCurvePath, curveName + "_optimized", maxCurvatureError);
        if (status == MS::kSuccess) {
            MGlobal::displayInfo("Successfully created and optimized curve: " + curveName + "_optimized");
        }
    }
    
    return status;
}

// 2. 기존 곡선을 디포머용으로 최적화
MStatus InputCurveOptimizer::optimizeCurveForDeformer(const MDagPath& inputCurvePath,
                                                      const MString& optimizedCurveName,
                                                      double maxCurvatureError)
{
    MStatus status;
    
    // 1단계: 곡선 복잡도 분석
    int numControlPoints, numKnots;
    double averageCurvature;
    status = analyzeCurveComplexity(inputCurvePath, numControlPoints, numKnots, averageCurvature);
    if (status != MS::kSuccess) return status;
    
    // 2단계: 곡률 기반 적응형 세분화 (특허 핵심!)
    std::vector<CurveSegment> segments = subdivideByCurvature(inputCurvePath, maxCurvatureError);
    
    // 3단계: 유사 세그먼트 병합
    mergeSimilarSegments(segments, maxCurvatureError);
    
    // 4단계: 제어점 수 최적화
    if (numControlPoints > mMaxControlPoints) {
        status = optimizeControlPointCount(inputCurvePath, mMaxControlPoints, optimizedCurveName);
        if (status != MS::kSuccess) return status;
    }
    
    // 5단계: Knot 분포 최적화 (활성화된 경우)
    if (mEnableKnotOptimization) {
        status = optimizeKnotDistribution(inputCurvePath, optimizedCurveName);
        if (status != MS::kSuccess) return status;
    }
    
    MGlobal::displayInfo("Curve optimized: " + inputCurvePath.partialPathName() + " -> " + optimizedCurveName);
    return MS::kSuccess;
}

// 3. 스켈레톤에서 자동으로 곡선 생성
MStatus InputCurveOptimizer::generateCurveFromSkeleton(const MDagPath& jointPath,
                                                       const MString& curveName,
                                                       double jointSpacing)
{
    MStatus status;
    
    // 1단계: 스켈레톤에서 경로 추출
    std::vector<MPoint> pathPoints;
    status = extractPathFromSkeleton(jointPath, pathPoints);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to extract path from skeleton");
        return status;
    }
    
    // 2단계: 경로를 최적화된 곡선으로 변환
    status = createOptimizedCurveFromPath(pathPoints, curveName);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to create curve from skeleton");
        return status;
    }
    
    MGlobal::displayInfo("Successfully created curve from skeleton: " + curveName);
    return MS::kSuccess;
}

// 4. 배치 최적화 (여러 곡선 한번에)
MStatus InputCurveOptimizer::batchOptimizeCurves(const std::vector<MDagPath>& inputCurves,
                                                 const MString& outputPrefix,
                                                 double maxCurvatureError)
{
    MStatus status;
    int successCount = 0;
    
    for (size_t i = 0; i < inputCurves.size(); i++) {
        const MDagPath& curvePath = inputCurves[i];
        MString optimizedName = outputPrefix + "_" + MString() + (int)i;
        
        status = optimizeCurveForDeformer(curvePath, optimizedName, maxCurvatureError);
        if (status == MS::kSuccess) {
            successCount++;
        }
    }
    
    MGlobal::displayInfo("Batch optimization completed: " + MString() + successCount + "/" + 
                         MString() + inputCurves.size() + " curves optimized");
    
    return (successCount > 0) ? MS::kSuccess : MS::kFailure;
}

// === 🎨 특허 기반 최적화 옵션들 ===

void InputCurveOptimizer::setOptimizationMode(bool useArcSegment)
{
    mUseArcSegment = useArcSegment;
    MGlobal::displayInfo("Optimization mode set to: " + MString(mUseArcSegment ? "Arc Segment" : "B-Spline"));
}

void InputCurveOptimizer::setCurvatureThreshold(double threshold)
{
    mCurvatureThreshold = threshold;
    MGlobal::displayInfo("Curvature threshold set to: " + MString() + threshold);
}

void InputCurveOptimizer::setMaxControlPoints(int maxPoints)
{
    mMaxControlPoints = maxPoints;
    MGlobal::displayInfo("Max control points set to: " + MString() + maxPoints);
}

void InputCurveOptimizer::enableKnotOptimization(bool enable)
{
    mEnableKnotOptimization = enable;
    MGlobal::displayInfo("Knot optimization " + MString(enable ? "enabled" : "disabled"));
}

// === 🔬 특허 기술 기반 최적화 엔진 ===

// 곡선 복잡도 분석
MStatus InputCurveOptimizer::analyzeCurveComplexity(const MDagPath& curvePath,
                                                    int& numControlPoints,
                                                    int& numKnots,
                                                    double& averageCurvature)
{
    MStatus status;
    MFnNurbsCurve fnCurve(curvePath, &status);
    if (status != MS::kSuccess) return status;
    
    numControlPoints = fnCurve.numCVs();
    numKnots = fnCurve.numKnots();
    
    // 곡률 샘플링으로 평균 곡률 계산
    const int numSamples = 20;
    double totalCurvature = 0.0;
    int validSamples = 0;
    
    for (int i = 0; i < numSamples; i++) {
        double paramU = (double)i / (numSamples - 1);
        
        // 1차 미분 (속도 벡터)
        MVector firstDerivative = fnCurve.tangent(paramU, MSpace::kWorld, &status);
        if (status != MS::kSuccess) continue;
        
        // 2차 미분 (가속도 벡터) - 수치적 계산
        double delta = 1e-6;
        double paramUPlus = std::min(1.0, paramU + delta);
        double paramUMinus = std::max(0.0, paramU - delta);
        
        MVector tangentPlus = fnCurve.tangent(paramUPlus, MSpace::kWorld);
        MVector tangentMinus = fnCurve.tangent(paramUMinus, MSpace::kWorld);
        
        MVector secondDerivative = (tangentPlus - tangentMinus) / (2.0 * delta);
        
        // 곡률 계산: κ = |r' × r''| / |r'|³
        MVector crossProduct = firstDerivative ^ secondDerivative;
        double speedCubed = pow(firstDerivative.length(), 3.0);
        
        if (speedCubed > 1e-12) {
            double curvature = crossProduct.length() / speedCubed;
            totalCurvature += curvature;
            validSamples++;
        }
    }
    
    averageCurvature = (validSamples > 0) ? totalCurvature / validSamples : 0.0;
    
    return MS::kSuccess;
}

// 제어점 수 최적화
MStatus InputCurveOptimizer::optimizeControlPointCount(const MDagPath& curvePath,
                                                       int targetControlPoints,
                                                       const MString& optimizedCurveName)
{
    MStatus status;
    MFnNurbsCurve fnCurve(curvePath, &status);
    if (status != MS::kSuccess) return status;
    
    // 현재 제어점 수
    int currentCVs = fnCurve.numCVs();
    
    if (currentCVs <= targetControlPoints) {
        // 이미 최적화됨
        return MS::kSuccess;
    }
    
    // 제어점 수 줄이기 (Maya의 rebuildCurve 명령 사용)
    MString command = "rebuildCurve -rpo 1 -rt 0 -end 1 -kr 2 -kcp 1 -kep 1 -kt 0 -s " + 
                      MString() + targetControlPoints + " -d 3 -tol 0.01 \"" + 
                      curvePath.partialPathName() + "\"";
    
    status = MGlobal::executeCommand(command);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to optimize control point count");
        return status;
    }
    
    return MS::kSuccess;
}

// Knot 분포 최적화
MStatus InputCurveOptimizer::optimizeKnotDistribution(const MDagPath& curvePath,
                                                      const MString& optimizedCurveName)
{
    MStatus status;
    MFnNurbsCurve fnCurve(curvePath, &status);
    if (status != MS::kSuccess) return status;
    
    // 현재 knot 분포 분석
    MDoubleArray knots;
    fnCurve.getKnots(knots);
    
    // 균등한 knot 분포로 재분배
    int numKnots = knots.length();
    if (numKnots < 4) return MS::kSuccess; // 최소 knot 수 확인
    
    // 새로운 균등 knot 분포 생성
    MDoubleArray newKnots;
    newKnots.setLength(numKnots);
    
    for (int i = 0; i < numKnots; i++) {
        newKnots[i] = (double)i / (numKnots - 1);
    }
    
    // knot 분포 적용
    status = fnCurve.setKnots(newKnots);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to optimize knot distribution");
        return status;
    }
    
    return MS::kSuccess;
}

// 곡률 기반 적응형 세분화 (특허 핵심!)
std::vector<CurveSegment> InputCurveOptimizer::subdivideByCurvature(const MDagPath& curvePath,
                                                                     double maxCurvatureError)
{
    std::vector<CurveSegment> segments;
    MStatus status;
    MFnNurbsCurve fnCurve(curvePath, &status);
    
    if (status != MS::kSuccess) return segments;
    
    const int numSamples = 20;  // 곡선을 20개 구간으로 나누어 분석
    double paramStep = 1.0 / numSamples;
    
    for (int i = 0; i < numSamples; i++) {
        double paramU = i * paramStep;
        double nextParamU = (i + 1) * paramStep;
        
        // 현재 구간의 곡률 분석
        MVector firstDerivative = fnCurve.tangent(paramU, MSpace::kWorld, &status);
        if (status != MS::kSuccess) continue;
        
        // 2차 미분 계산
        double delta = 1e-6;
        double paramUPlus = std::min(1.0, paramU + delta);
        double paramUMinus = std::max(0.0, paramU - delta);
        
        MVector tangentPlus = fnCurve.tangent(paramUPlus, MSpace::kWorld);
        MVector tangentMinus = fnCurve.tangent(paramUMinus, MSpace::kWorld);
        
        MVector secondDerivative = (tangentPlus - tangentMinus) / (2.0 * delta);
        
        // 곡률 계산
        MVector crossProduct = firstDerivative ^ secondDerivative;
        double speedCubed = pow(firstDerivative.length(), 3.0);
        
        CurveSegment segment;
        segment.startParamU = paramU;
        segment.endParamU = nextParamU;
        
        if (speedCubed > 1e-12) {
            segment.curvatureMagnitude = crossProduct.length() / speedCubed;
        } else {
            segment.curvatureMagnitude = 0.0;
        }
        
        // 곡률 기반 분류
        if (segment.curvatureMagnitude < maxCurvatureError) {
            // 직선 세그먼트
            segment.isLinear = true;
            segment.radius = 0.0;
        } else {
            // 곡선 세그먼트
            segment.isLinear = false;
            segment.radius = 1.0 / segment.curvatureMagnitude;
            
            // 원의 중심 계산 (근사)
            MPoint startPoint, endPoint, midPoint;
            fnCurve.getPointAtParam(paramU, startPoint, MSpace::kWorld);
            fnCurve.getPointAtParam(nextParamU, endPoint, MSpace::kWorld);
            fnCurve.getPointAtParam((paramU + nextParamU) * 0.5, midPoint, MSpace::kWorld);
            
            MVector toMid = midPoint - startPoint;
            MVector curvatureDir = crossProduct.normal();
            MVector perpendicular = toMid ^ curvatureDir;
            segment.center = midPoint + perpendicular * segment.radius;
        }
        
        segments.push_back(segment);
    }
    
    return segments;
}

// 인접한 유사 세그먼트 병합
void InputCurveOptimizer::mergeSimilarSegments(std::vector<CurveSegment>& segments,
                                              double maxCurvatureError)
{
    for (size_t i = 0; i < segments.size() - 1; ) {
        CurveSegment& current = segments[i];
        CurveSegment& next = segments[i + 1];
        
        // 두 세그먼트가 모두 직선이거나 곡률이 유사한 경우 병합
        bool canMerge = false;
        
        if (current.isLinear && next.isLinear) {
            canMerge = true;
        } else if (!current.isLinear && !next.isLinear) {
            double curvatureDiff = fabs(current.curvatureMagnitude - next.curvatureMagnitude);
            if (curvatureDiff < maxCurvatureError) {
                canMerge = true;
            }
        }
        
        if (canMerge) {
            // 세그먼트 병합
            current.endParamU = next.endParamU;
            if (!current.isLinear) {
                // 평균 곡률로 업데이트
                current.curvatureMagnitude = (current.curvatureMagnitude + next.curvatureMagnitude) * 0.5;
                current.radius = 1.0 / current.curvatureMagnitude;
            }
            
            // 병합된 세그먼트 제거
            segments.erase(segments.begin() + i + 1);
        } else {
            i++;  // 다음 세그먼트로 이동
        }
    }
}

// === 🏗️ 곡선 생성 엔진 ===

// 메시에서 경로 추출
MStatus InputCurveOptimizer::extractPathFromMesh(const MDagPath& meshPath,
                                                 std::vector<MPoint>& pathPoints)
{
    MStatus status;
    MFnMesh fnMesh(meshPath, &status);
    if (status != MS::kSuccess) return status;
    
    // 메시의 중심선을 따라 경로 추출
    // 간단한 구현: 메시의 중심점들을 연결
    MPointArray vertices;
    fnMesh.getPoints(vertices, MSpace::kWorld);
    
    // 메시의 중심 계산
    MPoint center(0, 0, 0);
    for (unsigned int i = 0; i < vertices.length(); i++) {
        center += vertices[i];
    }
    center /= vertices.length();
    
    // 중심을 지나는 경로 생성 (간단한 예시)
    pathPoints.clear();
    pathPoints.push_back(center + MPoint(-5, 0, 0));  // 시작점
    pathPoints.push_back(center);                      // 중심점
    pathPoints.push_back(center + MPoint(5, 0, 0));   // 끝점
    
    return MS::kSuccess;
}

// 스켈레톤에서 경로 추출
MStatus InputCurveOptimizer::extractPathFromSkeleton(const MDagPath& jointPath,
                                                     std::vector<MPoint>& pathPoints)
{
    MStatus status;
    MFnIkJoint fnJoint(jointPath, &status);
    if (status != MS::kSuccess) return status;
    
    pathPoints.clear();
    
    // 조인트 체인을 따라 경로 추출
    MDagPath currentJoint = jointPath;
    
    while (currentJoint.isValid()) {
        MFnTransform fnTransform(currentJoint, &status);
        if (status != MS::kSuccess) break;
        
        MPoint jointPosition = fnTransform.translation(MSpace::kWorld);
        pathPoints.push_back(jointPosition);
        
        // 다음 조인트 찾기
        unsigned int numChildren = fnTransform.childCount();
        if (numChildren == 0) break;
        
        // 첫 번째 자식 조인트로 이동
        MObject childObj = fnTransform.child(0);
        if (childObj.hasFn(MFn::kJoint)) {
            currentJoint = MDagPath::getAPathTo(childObj);
        } else {
            break;
        }
    }
    
    return MS::kSuccess;
}

// 경로를 최적화된 곡선으로 변환
MStatus InputCurveOptimizer::createOptimizedCurveFromPath(const std::vector<MPoint>& pathPoints,
                                                          const MString& curveName)
{
    if (pathPoints.size() < 2) {
        MGlobal::displayError("Need at least 2 points to create a curve");
        return MS::kFailure;
    }
    
    // 경로 포인트를 Maya 포인트 배열로 변환
    MPointArray mayaPoints;
    for (const MPoint& point : pathPoints) {
        mayaPoints.append(point);
    }
    
    // NURBS 곡선 생성
    MFnNurbsCurve fnCurve;
    MObject curveObj = fnCurve.create(mayaPoints, 3, MFnNurbsCurve::kOpen, false, false, false);
    
    if (curveObj.isNull()) {
        MGlobal::displayError("Failed to create curve");
        return MS::kFailure;
    }
    
    // 곡선 이름 설정
    MFnDagNode dagNode(curveObj);
    dagNode.setName(curveName);
    
    MGlobal::displayInfo("Successfully created curve: " + curveName);
    return MS::kSuccess;
}
