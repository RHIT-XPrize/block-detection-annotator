//-----------------------------------------------------------------------------
// <copyright file="MatlabHelper.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------------

#pragma once

#include "resource.h"
#include "windows.h"
#include <new>
#include <NuiApi.h>
#include <vector>

// MATLAB includes
#include "matrix.h"
#include "engine.h"

#include "MatlabFrameHelper.h"

class MatlabHelper
{
public:
    // Constants
    static const int NO_FILTER = -1;
    static const int RGB_DIMENSIONS = 3;
    static const int PIXEL_BYTE_SIZE = 4;
    static const int COLOR_GAUSS_KERNEL_SIZE = 10;
    static const int DEPTH_GAUSS_KERNEL_SIZE = 7;

    const enum StreamType { ColorStream = 1, DepthStream = 2 };

    /// <summary>
    /// Constructor
    /// </summary>
    MatlabHelper();

    /// <summary>
    /// Destructor
    /// </summary>
    ~MatlabHelper();

    /// <summary>
    /// Starts a MATLAB engine session
    /// </summary>
    /// <param name="engineUIVisible">whether to show the MATLAB engine GUI</param>
    /// <returns>S_OK if successful, an error code otherwise
    HRESULT InitMatlabEngine(bool engineUIVisible = false);

    /// <summary>
    /// Sets the color image filter to the one corresponding to the given resource ID
    /// </summary>
    /// <param name="filterID">resource ID of filter to use</param>
    void SetColorFilter(int filterID);

    /// <summary>
    /// Sets the depth image filter to the one corresponding to the given resource ID
    /// </summary>
    /// <param name="filterID">resource ID of filter to use</param>
    void SetDepthFilter(int filterID);

    /// <summary>
    /// Applies the color image filter to the given image
    /// </summary>
    /// <param name="pImg">pointer to mxArray holding image to filter</param>
    /// <returns>S_OK if successful, an error code otherwise
    HRESULT ApplyColorFilter(mxArray* pImg);

    /// <summary>
    /// Converts an RGB MATLAB mxArray into a Windows GDI bitmap
    /// </summary>
    /// <param name="pImg">pointer to mxArray holding image to convert</param>
    /// <param name="ppBitmapBits">pointer to pointer that will point to converted bitmap data</param>
    /// <param name="pBitmapInfo">header info for converted bitmap data</param>
    /// <returns>S_OK if successful, an error code otherwise</returns>
    HRESULT ConvertRgbMxArrayToBitmap(const mxArray* pImg, void** ppBitmapBits, BITMAPINFO* pBitmapInfo);

    /// <summary>
    /// Puts a MATLAB matrix (mxArray) into the MATLAB engine environment
    /// </summary>
    /// <param name="name">name of the matrix</param>
    /// <param name="pVariable">pointer to matrix to put into environment</param>
    /// <returns>S_OK if variable placed in MATLAB, an error code otherwise</returns>
    HRESULT MatlabPutVariable(const char* name, const mxArray* pVariable);

    /// <summary>
    /// Gets a MATLAB matrix (mxArray) from the MATLAB engine environment
    /// </summary>
    /// <param name="name">name of the matrix</param>
    /// <param name="ppVariable">pointer to update with location of fetched matrix</param>
    /// <returns>S_OK if variable fetched from MATLAB, an error code otherwise</returns>
    HRESULT MatlabGetVariable(const char* name, mxArray** ppVariable);

    /// <summary>
    /// Sends an expression to MATLAB for evaluation
    /// </summary>
    /// <param name="expr">expression string to evaluate</param>
    /// <returns>S_OK if expression sent to MATLAB, an error code otherwise</returns>
    HRESULT MatlabEvalExpr(const char* expr);

    void ShutDownEngine();

private:
    // Functions
    /// <summary>
    /// Validates an RGB MATLAB mxArray
    /// </summary>
    /// <param name="pImg">pointer to mxArray holding image to validate</param>
    /// <returns>S_OK if valid, an error code otherwise</returns>
    HRESULT ValidateMxArrayRgbImage(const mxArray* pImg);

    /// <summary>
    /// Converts a MATLAB return code into an HRESULT
    /// </summary>
    /// <param name="retCode">MATLAB return code</param>
    /// <returns>S_OK if success, E_FAIL if an error occurred</returns>
    HRESULT ConvertMatlabRetCodeToHResult(int retCode);

	//// <summary>
	/// Performs greyscale threshold
	/// </summary>
	/// <param name="pImg">pointer to the image that will have a filter applied to it</param>
	/// <returns>S_OK if success, E_FAIL if an error occurred</returns>
	HRESULT MatlabHelper::ApplyGrayscaleThreshold(mxArray* pImg);

	//// <summary>
	/// Finds centroids of objects on largest portion of background
	/// </summary>
	/// <param name="pImg">pointer to the image that will have a filter applied to it</param>
	/// <param name="centroids">2D vector array that contains the centroids of each object</param>
	/// <returns>S_OK if success, E_FAIL if an error occurred</returns>
	HRESULT MatlabHelper::ComputeCentroids(mxArray* pImg, std::vector<std::vector<double> > centroids);

    /// <summary>
    /// Moves RGB data from one mxArray to another
    /// </summary>
    /// <param name="pSourceImg">pointer to the source image</param>
    /// <param name="pDestImg">pointer to the destination image</param>
    /// <returns>S_OK if move is successful, E_FAIL if an error occurred</returns>
    HRESULT MoveRgbMxArrayData(mxArray* pSourceImg, mxArray* pDestImg);

    // Variables:
    // Resource IDs of the active filters
    int m_colorFilterID;
    int m_depthFilterID;

    // MATLAB Engine
    Engine* m_matlabEngine;
};