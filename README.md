# Input Curve Optimizer

Maya용 Input Curve 최적화 도구입니다. Offset Curve Deformer에 사용할 입력 커브를 자동으로 최적화합니다.

## 프로젝트 구조

```
inputCurveOptimizer/
├── CMakeLists.txt              # 메인 CMake 설정
├── README.md                   # 이 파일
├── modules/
│   └── findMaya.cmake         # Maya 찾기 모듈
└── src/
    ├── CMakeLists.txt          # 소스 빌드 설정
    ├── pluginMain.cpp          # Maya 플러그인 등록
    ├── inputCurveOptimizer.h   # 헤더 파일
    ├── inputCurveOptimizer.cpp # 구현 파일
    ├── inputCurveOptimizerUI.py # Python UI
    └── InputCurveOptimizer_README.md # 상세 문서
```

## 빌드 방법

### Windows (Visual Studio)

```bash
# 빌드 디렉토리 생성
mkdir build
cd build

# CMake 설정
cmake .. -G "Visual Studio 17 2022" -A x64

# 빌드
cmake --build . --config Release
```

### Linux/macOS

```bash
# 빌드 디렉토리 생성
mkdir build
cd build

# CMake 설정
cmake ..

# 빌드
make -j$(nproc)
```

## 설치

빌드된 `.mll` 파일을 Maya의 `plug-ins` 디렉토리에 복사하거나, CMake의 `install` 타겟을 사용하세요.

## 사용법

1. Maya에서 플러그인을 로드합니다
2. Python UI를 실행합니다: `python inputCurveOptimizerUI.py`
3. 메시, 스켈레톤, 또는 기존 커브에서 최적화된 커브를 생성합니다

## 의존성

- Maya API
- OpenMP (병렬 처리)
- C++14 지원 컴파일러
