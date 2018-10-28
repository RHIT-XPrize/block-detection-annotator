//-----------------------------------------------------------------------------
// <copyright file="MatlabFrameHelper.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------------

#pragma once
#include "KinectHelper.h"
#include "matrix.h"

namespace Microsoft {
    namespace KinectBridge {
        class MatlabFrameHelper : public KinectHelper<mxArray> {
        public:
            // Constants
            static const int NUM_RGB_VALUES_PER_PIXEL = 3;
            static const int NUM_DEPTH_VALUES_PER_PIXEL = 1;
            static const mwSize MATLAB_RGB_MATRIX_NUM_DIMENSIONS = 3;
            static const mwSize MATLAB_DEPTH_MATRIX_NUM_DIMENSIONS = 2;

            // Functions:
            /// <summary>
            /// Constructor
            /// </summary>
            MatlabFrameHelper() : KinectHelper<mxArray>() {}

            /// <summary>
            /// Destructor
            /// </summary>
            ~MatlabFrameHelper() {}

        protected:
            // Functions:
            /// <summary>
            /// Converts from Kinect color frame data into a MATLAB matrix
            /// The user needs to preallocate space for the image e.g.:
            /// mwSize dimensions[] = {m_colorHeight, m_colorWidth, NUM_RGB_VALUES_PER_PIXEL};
            ///	pImage = mxCreateNumericArray(MATLAB_RGB_MATRIX_NUM_DIMENSIONS, dimensions, mxUINT8_CLASS, mxREAL);
            /// </summary>
            /// <param name="pImage">pointer in which to return the MATLAB matrix</param>
            /// <returns>S_OK if successful, an error code otherwise</returns>
            HRESULT GetColorData(mxArray* pImage) const override;

            /// <summary>
            /// Converts from Kinect depth frame data into a MATLAB matrix
            /// The user needs to preallocate a 3D matrix e.g.:
            /// mwSize dimensions[] = {m_depthHeight, m_depthWidth, NUM_DEPTH_VALUES_PER_PIXEL};
            ///	pImage = mxCreateNumericArray(MATLAB_RGB_MATRIX_NUM_DIMENSIONS, dimensions, mxUINT16_CLASS, mxREAL);
            /// </summary>
            /// <param name="pImage">pointer in which to return the MATLAB matrix</param>
            /// <returns>S_OK if successful, an error code otherwise</returns>
            HRESULT GetDepthData(mxArray* pImage) const override;

            /// <summary>
            /// Converts from Kinect depth frame data into a ARGB MATLAB matrix.
            /// The user needs to preallocate the matrix e.g.:
            /// mwSize dimensions[] = {m_depthHeight, m_depthWidth, NUM_RGB_VALUES_PER_PIXEL};
            ///	pImage = mxCreateNumericArray(MATLAB_RGB_MATRIX_NUM_DIMENSIONS, dimensions, mxUINT8_CLASS, mxREAL);
            /// </summary>
            /// <param name="pImage">pointer in which to return the RGB MATLAB matrix</param>
            /// <returns>S_OK if successful, an error code otherwise</returns>
            HRESULT GetDepthDataAsArgb(mxArray* pImage) const override;

            /// <summary>
            /// Verify image is of the given resolution
            /// </summary>
            /// <param name="pImage">pointer to image to verify</param>
            /// <param name="resolution">resolution of image</param>
            /// <returns>S_OK if image matches given width and height, an error code otherwise</returns>
            HRESULT VerifySize(const mxArray* pImage, NUI_IMAGE_RESOLUTION resolution) const override;
        };
    }
}