//-----------------------------------------------------------------------------
// <copyright file="MatlabFrameHelper.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------------

#include "MatlabFrameHelper.h"

using namespace Microsoft::KinectBridge;

/// <summary>
/// Converts from Kinect color frame data into a MATLAB matrix
/// The user needs to preallocate space for the image e.g.:
/// mwSize dimensions[] = {m_colorHeight, m_colorWidth, NUM_RGB_VALUES_PER_PIXEL};
/// pImage = mxCreateNumericArray(MATLAB_RGB_MATRIX_NUM_DIMENSIONS, dimensions, mxUINT8_CLASS, mxREAL);
/// </summary>
/// <param name="pImage">pointer in which to return the MATLAB matrix</param>
/// <returns>S_OK if successful, an error code otherwise</returns>
HRESULT MatlabFrameHelper::GetColorData(mxArray* pImage) const
{
    // Check if image is valid
    if (m_colorBufferPitch == 0)
    {
        return E_NUI_FRAME_NO_DATA;
    }

    DWORD colorHeight, colorWidth;
    NuiImageResolutionToSize(m_colorResolution, colorWidth, colorHeight);

    UINT8* rgbDataBuffer = reinterpret_cast<UINT8*>(mxGetData(pImage));

    // Move data from image buffer into a MATLAB 3-D matrix
    // MATLAB stores data column-wise. I.e., it starts at the first column, goes through all the rows in that column
    // then moves onto the second column, going through all the rows in that 2nd column, and so forth. 
    // However, K4W SDK returns data row-wise. This loop goes row-wise
    // See http://www.mathworks.com/help/matlab/matlab_external/matlab-data.html#f22019
    for (int i = 0 ; i < m_colorBufferSize ; i += 4) 
    {
        int colIndex = (i / 4) % colorWidth;
        int rowIndex = (i / 4) / colorWidth;
        *(rgbDataBuffer + rowIndex + colIndex * colorHeight) = m_pColorBuffer[i + 2];									// Red pixel
        *(rgbDataBuffer + rowIndex + colIndex * colorHeight + colorWidth * colorHeight) = m_pColorBuffer[i + 1];		// Green pixel
        *(rgbDataBuffer + rowIndex + colIndex * colorHeight + 2 * colorWidth * colorHeight) = m_pColorBuffer[i];		// Blue pixel
    }

    return S_OK;
}

/// <summary>
/// Converts from Kinect depth frame data into a MATLAB matrix
/// The user needs to preallocate a 3D matrix e.g.:
/// mwSize dimensions[] = {m_depthHeight, m_depthWidth, NUM_DEPTH_VALUES_PER_PIXEL};
///	pImage = mxCreateNumericArray(MATLAB_RGB_MATRIX_NUM_DIMENSIONS, dimensions, mxUINT16_CLASS, mxREAL);
/// </summary>
/// <param name="pImage">pointer in which to return the MATLAB matrix</param>
/// <returns>S_OK if successful, an error code otherwise</returns>
HRESULT MatlabFrameHelper::GetDepthData(mxArray* pImage) const
{
    // Check if image is valid
    if (m_depthBufferPitch == 0)
    {
        return E_NUI_FRAME_NO_DATA;
    }

    DWORD depthHeight, depthWidth;
    NuiImageResolutionToSize(m_depthResolution, depthWidth, depthHeight);

    // Move data from image buffer into a MATLAB 3-D matrix
    USHORT* depthDataBuffer = reinterpret_cast<USHORT*>(mxGetData(pImage));
    USHORT* pBufferRun = reinterpret_cast<USHORT*>(m_pDepthBuffer);

    for (unsigned int i = 0 ; i < m_depthBufferSize / sizeof(USHORT) ; i += 1) 
    {
        int colIndex = i % depthWidth;
        int rowIndex = i / depthWidth;
        *(depthDataBuffer + rowIndex + colIndex * depthHeight) = pBufferRun[i];
    }

    return S_OK;
}

/// <summary>
/// Converts from Kinect depth frame data into a ARGB MATLAB matrix.
/// The user needs to preallocate the matrix e.g.:
/// mwSize dimensions[] = {m_depthHeight, m_depthWidth, NUM_RGB_VALUES_PER_PIXEL};
///	pImage = mxCreateNumericArray(MATLAB_RGB_MATRIX_NUM_DIMENSIONS, dimensions, mxUINT8_CLASS, mxREAL);
/// </summary>
/// <param name="pImage">pointer in which to return the RGB MATLAB matrix</param>
/// <returns>S_OK if successful, an error code otherwise</returns>
HRESULT MatlabFrameHelper::GetDepthDataAsArgb(mxArray* pImage) const
{
    DWORD depthWidth, depthHeight;
    NuiImageResolutionToSize(m_depthResolution, depthWidth, depthHeight);

    // Allocate space for depth image
    mwSize depthDimensions[] = {depthHeight, depthWidth, NUM_DEPTH_VALUES_PER_PIXEL};
    mxArray* pDepthImage = mxCreateNumericArray(MATLAB_RGB_MATRIX_NUM_DIMENSIONS, depthDimensions, mxUINT16_CLASS, mxREAL);

    // Get the depth image
    HRESULT hr = GetDepthData(pDepthImage);
    if (!SUCCEEDED(hr)) 
    {
        mxDestroyArray(pDepthImage);
        return hr;
    }
    USHORT* depthDataBuffer = reinterpret_cast<USHORT*>(mxGetData(pDepthImage));

    // Move data from image buffer into a MATLAB 3-D matrix
    UINT8* depthRgbDataBuffer = reinterpret_cast<UINT8*>(mxGetData(pImage));

    for (UINT col = 0 ; col < depthWidth ; col += 1) 
    {
        for (UINT row = 0 ; row < depthHeight ; row += 1) 
        {
            UINT8 redPixel, greenPixel, bluePixel;
            USHORT depth = *(depthDataBuffer + row + col * depthHeight);
            DepthShortToRgb(depth, &redPixel, &greenPixel, &bluePixel);

            *(depthRgbDataBuffer + row + col * depthHeight) = bluePixel;									// Blue pixel
            *(depthRgbDataBuffer + row + col * depthHeight + depthWidth * depthHeight) = greenPixel;		// Green pixel
            *(depthRgbDataBuffer + row + col * depthHeight + 2 * depthWidth * depthHeight) = redPixel;		// Red pixel
        }
    }

    mxDestroyArray(pDepthImage);

    return S_OK;
}

/// <summary>
/// Verify image is of the given resolution
/// </summary>
/// <param name="pImage">pointer to image to verify</param>
/// <param name="resolution">resolution of image</param>
/// <returns>S_OK if image matches given width and height, an error code otherwise</returns>
HRESULT MatlabFrameHelper::VerifySize(const mxArray* pImage, NUI_IMAGE_RESOLUTION resolution) const
{
    DWORD width, height;
    NuiImageResolutionToSize(resolution, width, height);

    const mwSize* dimensions = mxGetDimensions(pImage);
    if (dimensions[0] != height || dimensions[1] != width)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

