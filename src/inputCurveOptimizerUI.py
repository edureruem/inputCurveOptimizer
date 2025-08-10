"""
inputCurveOptimizerUI.py
Input Curve 최적화 도구의 Maya Python UI
C++ 백엔드와 연동하여 곡선 최적화 기능 제공
"""

import maya.cmds as cmds
import maya.mel as mel
import maya.OpenMaya as om
import maya.OpenMayaUI as omui
from PySide2 import QtWidgets, QtCore, QtGui
import shiboken2

class InputCurveOptimizerUI:
    def __init__(self):
        self.window_name = "InputCurveOptimizerWindow"
        self.optimizer = None  # C++ 백엔드 연결
        
        # UI 요소들
        self.curvature_slider = None
        self.max_cp_slider = None
        self.arc_segment_checkbox = None
        self.knot_optimization_checkbox = None
        
        # 설정값들
        self.curvature_threshold = 0.01
        self.max_control_points = 20
        self.use_arc_segment = True
        self.enable_knot_optimization = True
        
        self.create_ui()
    
    def create_ui(self):
        """Maya 윈도우 생성"""
        # 기존 윈도우가 있으면 삭제
        if cmds.window(self.window_name, exists=True):
            cmds.deleteUI(self.window_name)
        
        # 메인 윈도우
        self.window = cmds.window(self.window_name, title="Input Curve Optimizer", width=400, height=500)
        
        # 메인 레이아웃
        main_layout = cmds.columnLayout(adjustableColumn=True, rowSpacing=10, columnOffset=["both", 10])
        
        # 제목
        cmds.text(label="🎯 Input Curve 자동 최적화 도구", font="boldLabelFont", height=30)
        cmds.separator(height=10)
        
        # 설명
        cmds.text(label="특허 US8400455B2 기술을 활용한 곡선 품질 향상", 
                 font="smallPlainLabelFont", height=20)
        cmds.separator(height=10)
        
        # === 🚀 메인 최적화 기능들 ===
        cmds.text(label="🚀 메인 최적화 기능", font="boldLabelFont", height=25)
        
        # 1. 메시에서 곡선 생성
        cmds.button(label="1. 메시에서 최적 곡선 생성", 
                   command=self.generate_from_mesh, 
                   height=30, backgroundColor=[0.2, 0.6, 0.2])
        
        # 2. 기존 곡선 최적화
        cmds.button(label="2. 기존 곡선 최적화", 
                   command=self.optimize_existing_curve, 
                   height=30, backgroundColor=[0.2, 0.4, 0.8])
        
        # 3. 스켈레톤에서 곡선 생성
        cmds.button(label="3. 스켈레톤에서 곡선 생성", 
                   command=self.generate_from_skeleton, 
                   height=30, backgroundColor=[0.8, 0.4, 0.2])
        
        # 4. 배치 최적화
        cmds.button(label="4. 배치 최적화 (여러 곡선)", 
                   command=self.batch_optimize, 
                   height=30, backgroundColor=[0.4, 0.6, 0.4])
        
        cmds.separator(height=15)
        
        # === 🎨 최적화 옵션들 ===
        cmds.text(label="🎨 최적화 옵션", font="boldLabelFont", height=25)
        
        # 곡률 임계값
        cmds.text(label="곡률 임계값 (낮을수록 정밀함):")
        self.curvature_slider = cmds.floatSliderGrp(
            field=True, 
            minValue=0.001, 
            maxValue=0.1, 
            value=self.curvature_threshold,
            step=0.001,
            changeCommand=self.update_curvature_threshold
        )
        
        # 최대 제어점 수
        cmds.text(label="최대 제어점 수:")
        self.max_cp_slider = cmds.intSliderGrp(
            field=True, 
            minValue=5, 
            maxValue=50, 
            value=self.max_control_points,
            step=1,
            changeCommand=self.update_max_control_points
        )
        
        # Arc Segment 모드
        self.arc_segment_checkbox = cmds.checkBox(
            label="Arc Segment 모드 사용 (빠른 근사)", 
            value=self.use_arc_segment,
            changeCommand=self.update_optimization_mode
        )
        
        # Knot 최적화
        self.knot_optimization_checkbox = cmds.checkBox(
            label="Knot 분포 최적화", 
            value=self.enable_knot_optimization,
            changeCommand=self.update_knot_optimization
        )
        
        cmds.separator(height=15)
        
        # === 🆘 도움말 ===
        cmds.text(label="🆘 도움말", font="boldLabelFont", height=25)
        cmds.text(label="• 메시에서 곡선 생성: 선택된 메시의 중심을 따라 곡선 생성", 
                 font="smallPlainLabelFont", height=20)
        cmds.text(label="• 곡선 최적화: 선택된 곡선을 디포머용으로 최적화", 
                 font="smallPlainLabelFont", height=20)
        cmds.text(label="• 스켈레톤에서 곡선: 선택된 조인트 체인을 따라 곡선 생성", 
                 font="smallPlainLabelFont", height=20)
        cmds.text(label="• 배치 최적화: 여러 곡선을 한번에 최적화", 
                 font="smallPlainLabelFont", height=20)
        
        # 윈도우 표시
        cmds.showWindow(self.window)
    
    # === 🎛️ UI 이벤트 핸들러들 ===
    
    def update_curvature_threshold(self, value):
        """곡률 임계값 업데이트"""
        self.curvature_threshold = float(value)
        cmds.floatSliderGrp(self.curvature_slider, edit=True, value=self.curvature_threshold)
    
    def update_max_control_points(self, value):
        """최대 제어점 수 업데이트"""
        self.max_control_points = int(value)
        cmds.intSliderGrp(self.max_cp_slider, edit=True, value=self.max_control_points)
    
    def update_optimization_mode(self, value):
        """최적화 모드 업데이트"""
        self.use_arc_segment = bool(value)
        cmds.checkBox(self.arc_segment_checkbox, edit=True, value=self.use_arc_segment)
    
    def update_knot_optimization(self, value):
        """Knot 최적화 활성화/비활성화"""
        self.enable_knot_optimization = bool(value)
        cmds.checkBox(self.knot_optimization_checkbox, edit=True, value=self.enable_knot_optimization)
    
    # === 🚀 메인 기능 구현들 ===
    
    def generate_from_mesh(self, *args):
        """메시에서 최적 곡선 생성"""
        try:
            # 선택된 메시 확인
            selection = cmds.ls(selection=True, type="mesh")
            if not selection:
                cmds.warning("메시를 선택해주세요!")
                return
            
            mesh_name = selection[0]
            curve_name = mesh_name + "_optimalCurve"
            
            # C++ 백엔드 호출 (임시로 Python으로 구현)
            self._generate_curve_from_mesh_python(mesh_name, curve_name)
            
            cmds.select(curve_name)
            cmds.confirmDialog(title="완료", 
                             message=f"메시 '{mesh_name}'에서 최적 곡선 '{curve_name}'을 생성했습니다.",
                             button="확인")
            
        except Exception as e:
            cmds.error(f"메시에서 곡선 생성 실패: {str(e)}")
    
    def optimize_existing_curve(self, *args):
        """기존 곡선 최적화"""
        try:
            # 선택된 곡선 확인
            selection = cmds.ls(selection=True, type="nurbsCurve")
            if not selection:
                cmds.warning("최적화할 곡선을 선택해주세요!")
                return
            
            curve_name = selection[0]
            optimized_name = curve_name + "_optimized"
            
            # C++ 백엔드 호출 (임시로 Python으로 구현)
            self._optimize_curve_python(curve_name, optimized_name)
            
            cmds.select(optimized_name)
            cmds.confirmDialog(title="완료", 
                             message=f"곡선 '{curve_name}'을 '{optimized_name}'으로 최적화했습니다.",
                             button="확인")
            
        except Exception as e:
            cmds.error(f"곡선 최적화 실패: {str(e)}")
    
    def generate_from_skeleton(self, *args):
        """스켈레톤에서 곡선 생성"""
        try:
            # 선택된 조인트 확인
            selection = cmds.ls(selection=True, type="joint")
            if not selection:
                cmds.warning("시작 조인트를 선택해주세요!")
                return
            
            joint_name = selection[0]
            curve_name = joint_name + "_skeletonCurve"
            
            # C++ 백엔드 호출 (임시로 Python으로 구현)
            self._generate_curve_from_skeleton_python(joint_name, curve_name)
            
            cmds.select(curve_name)
            cmds.confirmDialog(title="완료", 
                             message=f"스켈레톤 '{joint_name}'에서 곡선 '{curve_name}'을 생성했습니다.",
                             button="확인")
            
        except Exception as e:
            cmds.error(f"스켈레톤에서 곡선 생성 실패: {str(e)}")
    
    def batch_optimize(self, *args):
        """배치 최적화"""
        try:
            # 모든 NURBS 곡선 찾기
            all_curves = cmds.ls(type="nurbsCurve")
            if not all_curves:
                cmds.warning("최적화할 곡선이 없습니다!")
                return
            
            # 배치 최적화 실행
            optimized_count = 0
            for curve_name in all_curves:
                try:
                    optimized_name = curve_name + "_batchOptimized"
                    self._optimize_curve_python(curve_name, optimized_name)
                    optimized_count += 1
                except:
                    continue
            
            cmds.confirmDialog(title="완료", 
                             message=f"총 {len(all_curves)}개 곡선 중 {optimized_count}개를 최적화했습니다.",
                             button="확인")
            
        except Exception as e:
            cmds.error(f"배치 최적화 실패: {str(e)}")
    
    # === 🔧 임시 Python 구현 (C++ 백엔드 연결 전까지) ===
    
    def _generate_curve_from_mesh_python(self, mesh_name, curve_name):
        """메시에서 곡선 생성 (Python 임시 구현)"""
        # 메시의 바운딩 박스 중심을 따라 곡선 생성
        bbox = cmds.exactWorldBoundingBox(mesh_name)
        center_x = (bbox[0] + bbox[3]) / 2
        center_y = (bbox[1] + bbox[4]) / 2
        center_z = (bbox[2] + bbox[5]) / 2
        
        # 간단한 직선 곡선 생성
        start_point = [center_x - 5, center_y, center_z]
        end_point = [center_x + 5, center_y, center_z]
        
        cmds.curve(degree=1, point=[start_point, end_point], name=curve_name)
        
        # 곡선을 NURBS로 변환
        cmds.rebuildCurve(curve_name, 
                         rebuildType=0,  # Uniform
                         spans=3,        # 3 spans
                         degree=3,       # 3차
                         keepRange=0,    # 0 to 1
                         keepEndPoints=True,
                         keepTangents=True,
                         keepCVs=False,
                         tolerance=0.01)
    
    def _optimize_curve_python(self, curve_name, optimized_name):
        """곡선 최적화 (Python 임시 구현)"""
        # 곡선 복사
        optimized_name = cmds.duplicate(curve_name, name=optimized_name)[0]
        
        # 제어점 수 최적화
        if self.max_control_points < 20:
            cmds.rebuildCurve(optimized_name,
                             rebuildType=0,
                             spans=self.max_control_points - 1,
                             degree=3,
                             keepRange=0,
                             keepEndPoints=True,
                             keepTangents=True,
                             keepCVs=False,
                             tolerance=0.01)
        
        # 곡선 부드럽게 만들기
        cmds.smoothCurve(optimized_name, smoothness=0.5)
    
    def _generate_curve_from_skeleton_python(self, joint_name, curve_name):
        """스켈레톤에서 곡선 생성 (Python 임시 구현)"""
        # 조인트 체인을 따라 경로 생성
        joint_chain = [joint_name]
        current_joint = joint_name
        
        # 자식 조인트들을 찾아서 체인 구성
        while True:
            children = cmds.listRelatives(current_joint, children=True, type="joint")
            if not children:
                break
            joint_chain.append(children[0])
            current_joint = children[0]
        
        if len(joint_chain) < 2:
            cmds.warning("조인트 체인이 너무 짧습니다!")
            return
        
        # 조인트 위치들을 가져와서 곡선 생성
        points = []
        for joint in joint_chain:
            pos = cmds.xform(joint, query=True, worldSpace=True, translation=True)
            points.append(pos)
        
        # 곡선 생성
        cmds.curve(degree=3, point=points, name=curve_name)

# === 🚀 실행 함수 ===

def show_ui():
    """UI 표시"""
    ui = InputCurveOptimizerUI()
    return ui

# Maya에서 실행할 때
if __name__ == "__main__":
    show_ui()
