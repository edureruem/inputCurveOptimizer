/**
 * pluginMain.cpp
 * Maya Input Curve Optimizer 플러그인 등록 및 초기화
 */

#include "inputCurveOptimizer.h"

#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, "Input Curve Optimizer", "1.0", "Any");
    
    // Input Curve Optimizer 도구 등록
    MGlobal::displayInfo("Input Curve Optimizer plugin loaded successfully.");
    MGlobal::displayInfo("This tool optimizes input curves for the Offset Curve Deformer.");
    MGlobal::displayInfo("Use the Input Curve Optimizer UI to access optimization features.");
    
    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);
    
    MGlobal::displayInfo("Input Curve Optimizer plugin unloaded.");
    
    return status;
}
