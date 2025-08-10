# 🎯 Input Curve Optimizer

**특허 US8400455B2 기술을 활용한 Input Curve 자동 최적화 도구**

## 📋 개요

Input Curve Optimizer는 Maya에서 Offset Curve Deformer를 사용하기 전에 **input curve를 자동으로 최적화**해주는 독립적인 도구입니다. 

특허 US8400455B2의 핵심 기술인 **곡률 기반 적응형 세분화**, **제어점 분포 최적화**, **Knot 분포 최적화**를 활용하여 디포머가 최고의 성능을 발휘할 수 있도록 도와줍니다.

## 🚀 주요 기능

### 1. **메시에서 최적 곡선 생성**
- 선택된 메시를 분석하여 중심선을 따라 최적의 곡선을 자동 생성
- 곡선의 복잡도에 따라 적절한 제어점 수 자동 조정
- 특허 기술 기반 곡률 분석으로 부드러운 곡선 생성

### 2. **기존 곡선 최적화**
- 현재 곡선의 품질을 분석하고 디포머용으로 최적화
- 제어점 수 자동 조정 (기본값: 최대 20개)
- Knot 분포 균등화로 안정적인 변형 보장

### 3. **스켈레톤에서 곡선 생성**
- 조인트 체인을 따라 자동으로 곡선 경로 생성
- 본의 길이와 개수에 따른 최적화된 곡선 생성
- 리그 파트별 맞춤형 곡선 설계

### 4. **배치 최적화**
- 여러 곡선을 한번에 최적화
- 프로덕션 환경에서 효율적인 워크플로우 지원

## 🎨 최적화 옵션

### **곡률 임계값 (Curvature Threshold)**
- **기본값**: 0.01
- **범위**: 0.001 ~ 0.1
- **설명**: 곡률이 이 값보다 작으면 직선으로, 크면 곡선으로 분류

### **최대 제어점 수 (Max Control Points)**
- **기본값**: 20개
- **범위**: 5 ~ 50개
- **설명**: 곡선의 복잡도를 제한하여 성능 최적화

### **최적화 모드 (Optimization Mode)**
- **Arc Segment**: 빠른 근사 계산 (게임/실시간용)
- **B-Spline**: 정확한 계산 (영화/고품질용)

### **Knot 분포 최적화**
- **기본값**: 활성화
- **설명**: Knot 간격을 균등하게 조정하여 안정적인 변형 보장

## 🔧 설치 및 사용법

### 1. **파일 구조**
```
src/
├── inputCurveOptimizer.h          # C++ 헤더 파일
├── inputCurveOptimizer.cpp        # C++ 구현 파일
├── inputCurveOptimizerUI.py       # Maya Python UI
└── InputCurveOptimizer_README.md  # 이 파일
```

### 2. **Maya에서 실행**
```python
# Python 스크립트 에디터에서 실행
import inputCurveOptimizerUI
optimizer_ui = inputCurveOptimizerUI.show_input_curve_optimizer()
```

### 3. **기본 사용법**

#### **메시에서 곡선 생성**
1. 메시 선택
2. "1. 메시에서 최적 곡선 생성" 버튼 클릭
3. 자동으로 최적화된 곡선 생성

#### **기존 곡선 최적화**
1. 최적화할 곡선 선택
2. "2. 기존 곡선 최적화" 버튼 클릭
3. 최적화된 곡선이 `_optimized` 접미사로 생성

#### **스켈레톤에서 곡선 생성**
1. 시작 조인트 선택
2. "3. 스켈레톤에서 곡선 생성" 버튼 클릭
3. 조인트 체인을 따라 곡선 자동 생성

#### **배치 최적화**
1. "4. 배치 최적화" 버튼 클릭
2. 씬의 모든 NURBS 곡선을 자동으로 최적화

## 🎯 특허 기술 활용

### **곡률 기반 적응형 세분화**
```cpp
// 특허 핵심 기술: κ = |r' × r''| / |r'|³
MVector crossProduct = firstDerivative ^ secondDerivative;
double curvature = crossProduct.length() / pow(firstDerivative.length(), 3.0);

// 곡률에 따른 세그먼트 분류
if (curvature < maxCurvatureError) {
    segment.isLinear = true;      // 직선 세그먼트
} else {
    segment.isLinear = false;     // 곡선 세그먼트
    segment.radius = 1.0 / curvature;
}
```

### **제어점 수 최적화**
- 곡선의 복잡도에 따라 적절한 제어점 수 자동 조정
- 과도한 제어점으로 인한 성능 저하 방지
- 특허의 "메모리 효율성" 목표 달성

### **Knot 분포 최적화**
- 균등한 Knot 분포로 안정적인 변형 보장
- 특허의 "정확한 변형" 목표 달성
- 아티스트 제어의 일관성 향상

## 🔍 고급 사용법

### **배치 최적화**
```python
# 여러 곡선을 한번에 최적화
curves = cmds.ls(type="nurbsCurve")
optimizer.batchOptimizeCurves(curves, "batch_optimized", 0.01)
```

### **품질 임계값 조정**
```python
# 고품질 프로덕션용 설정
optimizer.setCurvatureThreshold(0.005)      # 더 엄격한 곡률 기준
optimizer.setMaxControlPoints(30)            # 더 많은 제어점 허용
optimizer.setOptimizationMode(False)         # B-Spline 모드 사용
```

### **자동화 스크립트**
```python
# 프로젝트 전체 곡선 자동 최적화
def optimize_project_curves():
    all_curves = cmds.ls(type="nurbsCurve")
    optimizer = InputCurveOptimizer()
    optimizer.setCurvatureThreshold(0.01)
    optimizer.setMaxControlPoints(20)
    
    for curve in all_curves:
        if "deformer" in curve.lower():
            optimizer.optimizeCurveForDeformer(curve, curve + "_optimized")
```

## 🚨 주의사항

### **성능 고려사항**
- **Arc Segment 모드**: 빠르지만 근사 계산
- **B-Spline 모드**: 정확하지만 느림
- **제어점 수**: 많을수록 정확하지만 성능 저하

### **메모리 사용량**
- 곡선 복잡도에 따라 메모리 사용량 증가
- 배치 처리 시 메모리 모니터링 필요

### **호환성**
- Maya 2020 이상 권장
- Python 2.7 / 3.7+ 지원

## 🔧 문제 해결

### **일반적인 오류**

#### **"메시를 선택해주세요"**
- 메시가 선택되었는지 확인
- 메시 타입이 올바른지 확인 (mesh, nurbsSurface 등)

#### **"최적화할 곡선을 선택해주세요"**
- 곡선이 선택되었는지 확인
- 곡선 타입이 nurbsCurve인지 확인

#### **"조인트 체인이 충분하지 않습니다"**
- 최소 2개 이상의 조인트가 연결되어 있는지 확인
- 조인트 계층 구조가 올바른지 확인

### **성능 최적화 팁**
1. **Arc Segment 모드** 사용 (실시간/게임용)
2. **제어점 수 제한** (20개 이하 권장)
3. **배치 처리** 시 메모리 모니터링
4. **품질 임계값**을 프로젝트 요구사항에 맞게 조정

## 📚 추가 자료

### **참고 자료**
- **특허 US8400455B2**: "Method and apparatus for efficient offset curve deformation from skeletal animation"
- **Maya API 문서**: NURBS 곡선 조작 및 최적화
- **컴퓨터 그래픽스**: 곡선 이론 및 최적화 기법

## 🤝 기여 및 지원

### **버그 리포트**
- GitHub Issues를 통해 버그 리포트
- 재현 가능한 단계와 에러 메시지 포함

### **기능 제안**
- 새로운 최적화 알고리즘 제안
- UI/UX 개선 아이디어
- 성능 최적화 방안

### **개발 참여**
- C++ 최적화 엔진 개선
- Python UI 확장
- 테스트 케이스 작성

## 📄 라이선스

이 도구는 특허 US8400455B2의 기술을 교육 및 연구 목적으로 구현한 것입니다. 상업적 사용 시 해당 특허의 라이선스 상태를 확인하시기 바랍니다.

---

**🎯 Input Curve Optimizer로 더 나은 디포머 경험을 만들어보세요!**
