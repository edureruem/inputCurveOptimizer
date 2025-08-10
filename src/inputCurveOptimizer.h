/**
 * inputCurveOptimizer.h
 * Input Curve 자동 최적화 도구 헤더
 * 특허 US8400455B2 기술을 활용한 곡선 품질 향상
 */

#ifndef INPUT_CURVE_OPTIMIZER_H
#define INPUT_CURVE_OPTIMIZER_H

#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MDagPath.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <vector>

// 곡선 세그먼트 구조체
struct CurveSegment {
    double startParamU;           // 시작 매개변수
    double endParamU;             // 끝 매개변수
    double curvatureMagnitude;    // 곡률 크기
    bool isLinear;                // 직선 여부
    double radius;                // 곡률 반지름
    MPoint center;                // 곡률 중심점
};

class InputCurveOptimizer {
public:
    // 생성자/소멸자
    InputCurveOptimizer();
    ~InputCurveOptimizer();
    
    // === 🚀 메인 최적화 기능들 ===
    
    // 1. 메시에서 자동으로 최적의 곡선 생성
    MStatus generateOptimalCurveFromMesh(const MDagPath& meshPath,
                                        const MString& curveName,
                                        double maxCurvatureError = 0.01,
                                        int maxControlPoints = 20);
    
    // 2. 기존 곡선을 디포머용으로 최적화
    MStatus optimizeCurveForDeformer(const MDagPath& inputCurvePath,
                                    const MString& optimizedCurveName,
                                    double maxCurvatureError = 0.01);
    
    // 3. 스켈레톤에서 자동으로 곡선 생성
    MStatus generateCurveFromSkeleton(const MDagPath& jointPath,
                                     const MString& curveName,
                                     double jointSpacing = 1.0);
    
    // 4. 배치 최적화 (여러 곡선 한번에)
    MStatus batchOptimizeCurves(const std::vector<MDagPath>& inputCurves,
                                const MString& outputPrefix,
                                double maxCurvatureError = 0.01);
    
    // === 🎨 특허 기반 최적화 옵션들 ===
    
    // 최적화 모드 설정
    void setOptimizationMode(bool useArcSegment);
    
    // 곡률 임계값 설정
    void setCurvatureThreshold(double threshold);
    
    // 최대 제어점 수 설정
    void setMaxControlPoints(int maxPoints);
    
    // Knot 최적화 활성화/비활성화
    void enableKnotOptimization(bool enable);

private:
    // === 🔬 특허 기술 기반 최적화 엔진 ===
    
    // 곡선 복잡도 분석
    MStatus analyzeCurveComplexity(const MDagPath& curvePath,
                                  int& numControlPoints,
                                  int& numKnots,
                                  double& averageCurvature);
    
    // 제어점 수 최적화
    MStatus optimizeControlPointCount(const MDagPath& curvePath,
                                     int targetControlPoints,
                                     const MString& optimizedCurveName);
    
    // Knot 분포 최적화
    MStatus optimizeKnotDistribution(const MDagPath& curvePath,
                                    const MString& optimizedCurveName);
    
    // 곡률 기반 적응형 세분화 (특허 핵심!)
    std::vector<CurveSegment> subdivideByCurvature(const MDagPath& curvePath,
                                                   double maxCurvatureError);
    
    // 인접한 유사 세그먼트 병합
    void mergeSimilarSegments(std::vector<CurveSegment>& segments,
                             double maxCurvatureError);
    
    // === 🏗️ 곡선 생성 엔진 ===
    
    // 메시에서 경로 추출
    MStatus extractPathFromMesh(const MDagPath& meshPath,
                               std::vector<MPoint>& pathPoints);
    
    // 스켈레톤에서 경로 추출
    MStatus extractPathFromSkeleton(const MDagPath& jointPath,
                                   std::vector<MPoint>& pathPoints);
    
    // 경로를 최적화된 곡선으로 변환
    MStatus createOptimizedCurveFromPath(const std::vector<MPoint>& pathPoints,
                                        const MString& curveName);
    
    // === 🎛️ 최적화 설정 ===
    
    bool mUseArcSegment;              // Arc Segment 모드 (빠른 근사)
    double mCurvatureThreshold;       // 곡률 임계값 (0.001 ~ 0.1)
    int mMaxControlPoints;            // 최대 제어점 수 (5 ~ 50)
    bool mEnableKnotOptimization;     // Knot 분포 최적화 활성화
};

#endif // INPUT_CURVE_OPTIMIZER_H
