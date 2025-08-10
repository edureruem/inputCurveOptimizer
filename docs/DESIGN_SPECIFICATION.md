# Input Curve Optimizer - 설계 명세서

## 문서 개요

**문서 버전**: 1.0  
**최종 수정일**: 2024년  
**작성자**: 개발팀  
**검토자**: 기술 검토팀  

---

## 1. 시스템 개요

### 1.1 프로젝트 목적
Input Curve Optimizer는 Maya에서 Offset Curve Deformer를 사용하기 전에 input curve를 자동으로 최적화하는 독립적인 도구입니다. 특허 US8400455B2의 핵심 기술을 활용하여 디포머가 최고의 성능을 발휘할 수 있도록 도와줍니다.

### 1.2 핵심 가치
- **성능 최적화**: 곡선 복잡도 자동 조정으로 디포머 성능 향상
- **품질 보장**: 특허 기반 알고리즘으로 일관된 곡선 품질
- **사용자 편의성**: 직관적인 UI로 복잡한 최적화 과정 자동화
- **확장성**: 모듈화된 설계로 향후 기능 확장 용이

### 1.3 기술 스택
```
Backend: C++14 + Maya API
Frontend: Python + PySide2 + Maya Commands
Build System: CMake
Parallel Processing: OpenMP
Platform: Windows, Linux, macOS
```

---

## 2. 시스템 아키텍처

### 2.1 전체 시스템 구조
```
┌─────────────────────────────────────────────────────────────┐
│                    Maya Host Environment                    │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐    ┌─────────────────────────────────┐ │
│  │   Python UI     │    │      C++ Plugin (.mll)          │ │
│  │                 │    │                                 │ │
│  │ • 사용자 인터페이스  │◄──►│ • 곡선 최적화 엔진            │ │
│  │ • Maya 명령어 실행  │    │ • 특허 알고리즘 구현          │ │
│  │ • 설정 관리        │    │ • 고성능 수학 연산            │ │
│  └─────────────────┘    └─────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 모듈 구성
```
src/
├── inputCurveOptimizer.h          # C++ 클래스 인터페이스
├── inputCurveOptimizer.cpp        # C++ 핵심 알고리즘 구현
├── inputCurveOptimizerUI.py       # Python 사용자 인터페이스
├── pluginMain.cpp                 # Maya 플러그인 진입점
└── CMakeLists.txt                 # 빌드 설정
```

### 2.3 데이터 흐름
```
1. 사용자 입력 (UI) → 2. Python 처리 → 3. C++ 백엔드 호출 → 4. Maya API 조작 → 5. 결과 반환 → 6. UI 업데이트
```

---

## 3. 핵심 컴포넌트 설계

### 3.1 InputCurveOptimizer 클래스

#### 3.1.1 클래스 구조
```cpp
class InputCurveOptimizer {
public:
    // 생성자/소멸자
    InputCurveOptimizer();
    ~InputCurveOptimizer();
    
    // 공개 인터페이스
    MStatus generateOptimalCurveFromMesh(...);
    MStatus optimizeCurveForDeformer(...);
    MStatus generateCurveFromSkeleton(...);
    MStatus batchOptimizeCurves(...);
    
    // 설정 메서드
    void setOptimizationMode(bool useArcSegment);
    void setCurvatureThreshold(double threshold);
    void setMaxControlPoints(int maxPoints);
    void enableKnotOptimization(bool enable);

private:
    // 내부 구현
    MStatus analyzeCurveComplexity(...);
    MStatus optimizeControlPointCount(...);
    MStatus optimizeKnotDistribution(...);
    std::vector<CurveSegment> subdivideByCurvature(...);
    void mergeSimilarSegments(...);
    
    // 멤버 변수
    bool mUseArcSegment;
    double mCurvatureThreshold;
    int mMaxControlPoints;
    bool mEnableKnotOptimization;
};
```

#### 3.1.2 책임 분리
- **생성자**: 기본 설정값 초기화
- **공개 메서드**: 사용자 요청 처리 및 고수준 워크플로우 조정
- **프라이빗 메서드**: 특정 최적화 알고리즘 구현
- **멤버 변수**: 최적화 설정 및 상태 관리

### 3.2 CurveSegment 구조체
```cpp
struct CurveSegment {
    double startParamU;           // 시작 매개변수 (0.0 ~ 1.0)
    double endParamU;             // 끝 매개변수 (0.0 ~ 1.0)
    double curvatureMagnitude;    // 곡률 크기 (≥ 0.0)
    bool isLinear;                // 직선 여부
    double radius;                // 곡률 반지름 (직선일 때 0.0)
    MPoint center;                // 곡률 중심점 (직선일 때 무시)
};
```

### 3.3 Python UI 클래스
```python
class InputCurveOptimizerUI:
    def __init__(self):
        # UI 요소 초기화
        # 설정값 초기화
        
    def create_ui(self):
        # Maya 윈도우 생성
        # UI 레이아웃 구성
        
    # 이벤트 핸들러들
    def generate_from_mesh(self):
    def optimize_existing_curve(self):
    def generate_from_skeleton(self):
    def batch_optimize(self):
```

---

## 4. 핵심 알고리즘 설계

### 4.1 곡률 기반 적응형 세분화 (특허 핵심)

#### 4.1.1 수학적 기반
```
곡률 공식: κ = |r' × r''| / |r'|³

여기서:
- r': 1차 미분 (속도 벡터)
- r'': 2차 미분 (가속도 벡터)
- κ: 곡률 (curvature)
```

#### 4.1.2 알고리즘 흐름
```
1. 곡선을 N개 구간으로 분할
2. 각 구간에서 곡률 계산
3. 곡률 임계값과 비교하여 분류:
   - κ < threshold → 직선 세그먼트
   - κ ≥ threshold → 곡선 세그먼트
4. 유사한 인접 세그먼트 병합
5. 최적화된 세그먼트 구조 반환
```

#### 4.1.3 구현 세부사항
```cpp
std::vector<CurveSegment> subdivideByCurvature(
    const MDagPath& curvePath, 
    double maxCurvatureError
) {
    const int numSamples = 20;  // 샘플링 수
    double paramStep = 1.0 / numSamples;
    
    for (int i = 0; i < numSamples; i++) {
        // 1. 매개변수 계산
        double paramU = i * paramStep;
        double nextParamU = (i + 1) * paramStep;
        
        // 2. 1차, 2차 미분 계산
        MVector firstDerivative = fnCurve.tangent(paramU);
        MVector secondDerivative = calculateSecondDerivative(paramU);
        
        // 3. 곡률 계산
        MVector crossProduct = firstDerivative ^ secondDerivative;
        double speedCubed = pow(firstDerivative.length(), 3.0);
        double curvature = crossProduct.length() / speedCubed;
        
        // 4. 세그먼트 분류 및 생성
        CurveSegment segment;
        segment.startParamU = paramU;
        segment.endParamU = nextParamU;
        segment.curvatureMagnitude = curvature;
        
        if (curvature < maxCurvatureError) {
            segment.isLinear = true;
            segment.radius = 0.0;
        } else {
            segment.isLinear = false;
            segment.radius = 1.0 / curvature;
            segment.center = calculateCurvatureCenter(paramU, curvature);
        }
        
        segments.push_back(segment);
    }
    
    return segments;
}
```

### 4.2 제어점 수 최적화

#### 4.2.1 최적화 전략
```
목표: 곡선의 복잡도와 성능의 균형

1. 현재 제어점 수 분석
2. 목표 제어점 수와 비교
3. 필요시 Maya의 rebuildCurve 명령 사용
4. 최적화된 곡선 생성
```

#### 4.2.2 구현
```cpp
MStatus optimizeControlPointCount(
    const MDagPath& curvePath,
    int targetControlPoints,
    const MString& optimizedCurveName
) {
    MFnNurbsCurve fnCurve(curvePath);
    int currentCVs = fnCurve.numCVs();
    
    if (currentCVs <= targetControlPoints) {
        return MS::kSuccess;  // 이미 최적화됨
    }
    
    // Maya 명령어로 제어점 수 조정
    MString command = "rebuildCurve -rpo 1 -rt 0 -end 1 -kr 2 -kcp 1 -kep 1 -kt 0 -s " + 
                    MString() + targetControlPoints + " -d 3 -tol 0.01 \"" + 
                    curvePath.partialPathName() + "\"";
    
    return MGlobal::executeCommand(command);
}
```

### 4.3 Knot 분포 최적화

#### 4.3.1 목적
```
균등한 Knot 분포로 안정적인 변형 보장

1. 현재 Knot 분포 분석
2. 균등한 간격으로 재분배
3. 곡선의 매개변수화 개선
```

#### 4.3.2 구현
```cpp
MStatus optimizeKnotDistribution(
    const MDagPath& curvePath,
    const MString& optimizedCurveName
) {
    MFnNurbsCurve fnCurve(curvePath);
    MDoubleArray knots;
    fnCurve.getKnots(knots);
    
    int numKnots = knots.length();
    if (numKnots < 4) return MS::kSuccess;
    
    // 균등한 Knot 분포 생성
    MDoubleArray newKnots;
    newKnots.setLength(numKnots);
    
    for (int i = 0; i < numKnots; i++) {
        newKnots[i] = (double)i / (numKnots - 1);
    }
    
    return fnCurve.setKnots(newKnots);
}
```

---

## 5. 사용자 인터페이스 설계

### 5.1 UI 구조
```
┌─────────────────────────────────────────────────────────┐
│                Input Curve Optimizer                    │
├─────────────────────────────────────────────────────────┤
│ 메인 최적화 기능                                        │
│ [1. 메시에서 최적 곡선 생성] [2. 기존 곡선 최적화]      │
│ [3. 스켈레톤에서 곡선 생성] [4. 배치 최적화]            │
├─────────────────────────────────────────────────────────┤
│ 최적화 옵션                                            │
│ 곡률 임계값: [━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━] │
│ 최대 제어점 수: [━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━] │
│ ☑ Arc Segment 모드 사용  ☑ Knot 분포 최적화            │
├─────────────────────────────────────────────────────────┤
│ 도움말                                                 │
│ • 메시에서 곡선 생성: 선택된 메시의 중심을 따라 곡선 생성 │
│ • 곡선 최적화: 선택된 곡선을 디포머용으로 최적화         │
└─────────────────────────────────────────────────────────┘
```

### 5.2 사용자 워크플로우
```
1. Maya에서 작업할 객체 선택 (메시/곡선/조인트)
2. Input Curve Optimizer UI 열기
3. 원하는 최적화 기능 선택
4. 최적화 옵션 조정 (필요시)
5. 실행 버튼 클릭
6. 결과 확인 및 최적화된 곡선 사용
```

### 5.3 UI 컴포넌트 상세

#### 5.3.1 버튼들
- **색상 코딩**: 기능별로 다른 배경색 사용
- **크기**: 높이 30px로 충분한 클릭 영역 확보
- **상태**: 실행 중일 때 비활성화

#### 5.3.2 슬라이더들
- **곡률 임계값**: 0.001 ~ 0.1 범위, 0.001 단위 조정
- **최대 제어점 수**: 5 ~ 50 범위, 1 단위 조정
- **실시간 업데이트**: 값 변경 시 즉시 반영

#### 5.3.3 체크박스들
- **Arc Segment 모드**: 빠른 근사 vs 정확한 계산
- **Knot 최적화**: 활성화/비활성화 선택

---

## 6. 성능 최적화

### 6.1 병렬 처리 (OpenMP)
```cpp
#ifdef OPENMP_ENABLED
    #pragma omp parallel for
    for (int i = 0; i < numSamples; i++) {
        // 곡률 계산 작업
    }
#endif
```

### 6.2 메모리 관리
- **벡터 사용**: 동적 배열로 메모리 효율성 향상
- **참조 전달**: 큰 객체는 const 참조로 전달
- **지역 변수**: 가능한 한 스택 메모리 사용

### 6.3 Maya API 최적화
- **배치 처리**: 여러 곡선을 한번에 처리
- **명령어 최적화**: Maya 명령어 체인으로 효율성 향상
- **캐싱**: 반복 계산 결과 재사용

---

## 7. 오류 처리 및 예외 상황

### 7.1 오류 분류
```
1. 사용자 입력 오류
   - 잘못된 객체 선택
   - 범위를 벗어난 설정값

2. Maya API 오류
   - 객체 접근 실패
   - 명령어 실행 실패

3. 알고리즘 오류
   - 수치 계산 실패
   - 메모리 부족

4. 시스템 오류
   - 파일 I/O 실패
   - 권한 문제
```

### 7.2 오류 처리 전략
```cpp
MStatus InputCurveOptimizer::generateOptimalCurveFromMesh(...) {
    MStatus status;
    
    // 1단계: 메시에서 경로 추출
    std::vector<MPoint> pathPoints;
    status = extractPathFromMesh(meshPath, pathPoints);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to extract path from mesh");
        return status;  // 조기 반환으로 오류 전파
    }
    
    // 2단계: 경로를 최적화된 곡선으로 변환
    status = createOptimizedCurveFromPath(pathPoints, curveName);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to create optimized curve");
        return status;
    }
    
    return MS::kSuccess;
}
```

### 7.3 사용자 피드백
- **성공**: 확인 대화상자로 결과 표시
- **경고**: 주의사항이나 제한사항 안내
- **오류**: 구체적인 오류 메시지와 해결 방안 제시

---

## 8. 테스트 전략

### 8.1 단위 테스트
```cpp
// 곡률 계산 테스트
void testCurvatureCalculation() {
    // 직선 곡선: 곡률 = 0
    // 원형 곡선: 곡률 = 1/반지름
    // 복합 곡선: 다양한 곡률값 검증
}

// 제어점 최적화 테스트
void testControlPointOptimization() {
    // 제어점 수 감소 테스트
    // 품질 유지 확인
    // 성능 향상 검증
}
```

### 8.2 통합 테스트
- **Maya 플러그인 로딩**: 플러그인 정상 동작 확인
- **UI 동작**: 모든 버튼과 옵션 정상 작동 확인
- **데이터 흐름**: Python → C++ → Maya API 전체 흐름 검증

### 8.3 성능 테스트
- **대용량 데이터**: 많은 곡선 동시 처리
- **메모리 사용량**: 메모리 누수 및 효율성 확인
- **응답 시간**: UI 반응성 및 백엔드 처리 속도 측정

---

## 9. 확장성 및 유지보수

### 9.1 모듈화 설계
```
각 기능을 독립적인 모듈로 분리:
- 곡선 분석 모듈
- 최적화 알고리즘 모듈
- UI 컴포넌트 모듈
- Maya 통합 모듈
```

### 9.2 설정 관리
```cpp
// 설정값을 외부 파일로 관리 가능
struct OptimizationSettings {
    double curvatureThreshold;
    int maxControlPoints;
    bool useArcSegment;
    bool enableKnotOptimization;
};
```

### 9.3 플러그인 아키텍처
- **Maya 플러그인 표준**: Maya 플러그인 개발 가이드라인 준수
- **버전 관리**: 플러그인 버전 및 호환성 관리
- **업데이트 메커니즘**: 자동 업데이트 및 백업 기능

---

## 10. 참고 자료

### 10.1 기술 문서
- **Maya API Reference**: Maya C++ API 문서
- **OpenMP Specification**: 병렬 처리 표준
- **CMake Documentation**: 빌드 시스템 문서

### 10.2 특허 정보
- **US8400455B2**: "Method and apparatus for efficient offset curve deformation from skeletal animation"
- **핵심 기술**: 곡률 기반 적응형 세분화, 제어점 최적화, Knot 분포 최적화

### 10.3 관련 연구
- **NURBS 곡선 이론**: 수학적 기반
- **컴퓨터 그래픽스**: 곡선 최적화 기법
- **성능 최적화**: 알고리즘 효율성 개선 방법

---

## 11. 변경 이력

| 버전 | 날짜 | 변경 내용 | 작성자 |
|------|------|-----------|--------|
| 1.0  | 2024 | 초기 설계 명세서 작성 | 개발팀 |

---

**문서 끝**
